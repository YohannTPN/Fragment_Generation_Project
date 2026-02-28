#include "IFSManager.h"
#include <GL/glut.h>
#include <iostream>

IFSManager::IFSManager() 
    : currentDepth(0), transformsSet(false) 
{
}

void IFSManager::setGlobalTransforms(
    const IFSTransform& Ti,
    const IFSTransform& Tij,
    const IFSTransform& Tj
) {
    globalTi = Ti;
    globalTij = Tij;
    globalTj = Tj;
    transformsSet = true;
    
    std::cout << "IFSManager: Transformations globales définies" << std::endl;
}

void IFSManager::addEdge(const Vec2f& start, const Vec2f& end) {
    auto edge = std::make_unique<IFSEdge>(start, end);
    
    if (transformsSet) {
        edge->setTransforms(globalTi, globalTij, globalTj);
    }
    
    edges.push_back(std::move(edge));
}

void IFSManager::addEdgeWithControlPoints(const Vec2f& p0, const Vec2f& p1, 
                                          const Vec2f& p2, const Vec2f& p3) {
    auto edge = std::make_unique<IFSEdge>(p0, p3);
    edge->setControlPoints(p1, p2);
    
    if (transformsSet) {
        edge->setTransforms(globalTi, globalTij, globalTj);
    }
    
    edges.push_back(std::move(edge));
}

void IFSManager::generate(int depth) {
    if (!transformsSet) {
        std::cerr << "ERREUR: Transformations non définies!" << std::endl;
        return;
    }
    
    currentDepth = depth;
    generatedEdges.clear();
    
    std::cout << "Génération IFS avec profondeur " << depth 
              << " pour " << edges.size() << " arêtes..." << std::endl;
    
    for (const auto& edge : edges) {
        std::vector<Vec2f> points = edge->generate(depth);
        generatedEdges.push_back(points);
    }
    
    std::cout << "✓ " << generatedEdges.size() << " arêtes générées" << std::endl;
}

void IFSManager::drawUV() const {
    if (generatedEdges.empty()) {
        return;
    }
    
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f);  // Rouge pour les arêtes IFS
    glLineWidth(2.0f);
    
    for (const auto& edgePoints : generatedEdges) {
        if (edgePoints.size() < 2) continue;
        
        glBegin(GL_LINE_STRIP);
        for (const auto& pt : edgePoints) {
            glVertex2f(pt.u, pt.v);
        }
        glEnd();
    }
    
    glColor3f(0.0f, 0.0f, 1.0f);  
    glPointSize(4.0f);
    
    glBegin(GL_POINTS);
    for (const auto& edge : edges) {
        glVertex2f(edge->getP0().u, edge->getP0().v);
        glVertex2f(edge->getP1().u, edge->getP1().v);
        glVertex2f(edge->getP2().u, edge->getP2().v);
        glVertex2f(edge->getP3().u, edge->getP3().v);
    }
    glEnd();
}

void IFSManager::clear() {
    edges.clear();
    generatedEdges.clear();
    currentDepth = 0;
}