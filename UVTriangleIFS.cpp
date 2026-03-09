#include "UVTriangleIFS.h"
#include <iostream>
#include <GL/glut.h>
#include <cmath>

UVTriangleIFS::UVTriangleIFS(UVTriangleManager* triManager) 
    : triangleManager(triManager), currentDepth(0), generatedMesh(new HalfEdgeMesh()) 
{
}

void UVTriangleIFS::setTransforms(
    const IFSTransform& T0,
    const IFSTransform& T1
) {
    ifsManager.setGlobalTransforms(T0, T1);
}

std::vector<Vec2f> UVTriangleIFS::getOrCreateEdgeIFS(float u1, float v1, float u2, float v2) {
    const long precision = 1000;  // Même précision que dans EdgeKey
    
    EdgeKey key(u1, v1, u2, v2);
    
    // Détecter si on doit inverser
    long qu1 = (long)std::round(u1 * precision);
    long qv1 = (long)std::round(v1 * precision);
    bool needsReversal = (qu1 != key.u1 || qv1 != key.v1);
    
    auto it = edgeIFSCache.find(key);
    if (it != edgeIFSCache.end()) {
        if (needsReversal) {
            std::vector<Vec2f> reversed = it->second;
            std::reverse(reversed.begin(), reversed.end());
            return reversed;
        }
        return it->second;
    }
    
    // Détecter les bords
    const long epsilon = (long)(0.001f * precision);
    const long quantMax = (long)(1.0f * precision);
    
    bool onLeftBorder = (std::abs(key.u1) < epsilon && std::abs(key.u2) < epsilon);
    bool onRightBorder = (std::abs(key.u1 - quantMax) < epsilon && std::abs(key.u2 - quantMax) < epsilon);
    bool onBottomBorder = (std::abs(key.v1) < epsilon && std::abs(key.v2) < epsilon);
    bool onTopBorder = (std::abs(key.v1 - quantMax) < epsilon && std::abs(key.v2 - quantMax) < epsilon);
    
    bool isBorderEdge = onLeftBorder || onRightBorder || onBottomBorder || onTopBorder;
    
    if (isBorderEdge) {
        float fu1 = key.u1 / (float)precision;
        float fv1 = key.v1 / (float)precision;
        float fu2 = key.u2 / (float)precision;
        float fv2 = key.v2 / (float)precision;
        
        std::vector<Vec2f> straightLine;
        straightLine.push_back(Vec2f(fu1, fv1));
        straightLine.push_back(Vec2f(fu2, fv2));
        edgeIFSCache[key] = straightLine;
        
        if (needsReversal) {
            std::reverse(straightLine.begin(), straightLine.end());
        }
        return straightLine;
    }
    
    // Générer les points IFS avec 5 points de contrôle
    float fu1 = key.u1 / (float)precision;
    float fv1 = key.v1 / (float)precision;
    float fu2 = key.u2 / (float)precision;
    float fv2 = key.v2 / (float)precision;
    
    auto edge = std::make_unique<IFSEdge>(Vec2f(fu1, fv1), Vec2f(fu2, fv2));
    
    // Calculer les points de contrôle pour former un "chapeau"
    float dx = fu2 - fu1;
    float dy = fv2 - fv1;
    float length = std::sqrt(dx * dx + dy * dy);
    
    float perpX = -dy / length;
    float perpY = dx / length;
    
    // Amplitude plus grande pour un chapeau visible
    float amplitude = length * 0.15f;  // 15% au lieu de 2%
    
    // P1 à 1/4 déplacé VERS LE HAUT
    Vec2f p1;
    p1.u = fu1 + dx / 4.0f + perpX * amplitude;
    p1.v = fv1 + dy / 4.0f + perpY * amplitude;
    
    // P2 à 1/2 reste SUR LA LIGNE (pas de déplacement perpendiculaire)
    Vec2f p2;
    p2.u = fu1 + dx / 2.0f;
    p2.v = fv1 + dy / 2.0f;
    
    // P3 à 3/4 déplacé VERS LE BAS (négatif)
    Vec2f p3;
    p3.u = fu1 + 3.0f * dx / 4.0f - perpX * amplitude;
    p3.v = fv1 + 3.0f * dy / 4.0f - perpY * amplitude;
    
    edge->setControlPoints(p1, p2, p3);
    edge->setTransforms(
        ifsManager.globalT0,
        ifsManager.globalT1
    );
    
    std::vector<Vec2f> points = edge->generate(currentDepth);
    
    // Quantifier tous les points SAUF les extrémités (déjà forcées)
    const float quantPrecision = (float)precision;
    for (size_t i = 1; i < points.size() - 1; ++i) {
        points[i].u = std::round(points[i].u * quantPrecision) / quantPrecision;
        points[i].v = std::round(points[i].v * quantPrecision) / quantPrecision;
    }
    
    // Les extrémités sont déjà forcées à fu1,fv1 et fu2,fv2 dans IFSEdge::generate()
    
    edgeIFSCache[key] = points;
    
    if (needsReversal) {
        std::reverse(points.begin(), points.end());
    }
    
    return points;
}

void UVTriangleIFS::generateIFSTriangles(int ifsDepth) {
    if (!triangleManager) {
        std::cerr << "ERREUR: Pas de triangle manager!" << std::endl;
        return;
    }
    
    currentDepth = ifsDepth;
    ifsTriangles.clear();
    edgeIFSCache.clear();
    
    const auto& triangles = triangleManager->getTriangles();
    
    std::cout << "Génération IFS pour " << triangles.size() 
              << " triangles (profondeur " << ifsDepth << ")..." << std::endl;
    
    // Compter le nombre total d'arêtes (avant dédoublonnage)
    int totalEdgeRequests = 0;
    
    for (const auto& tri : triangles) {
        totalEdgeRequests += 3;  // 3 arêtes par triangle
        
        std::vector<Vec2f> edge1 = getOrCreateEdgeIFS(tri.u1, tri.v1, tri.u2, tri.v2);
        std::vector<Vec2f> edge2 = getOrCreateEdgeIFS(tri.u2, tri.v2, tri.u3, tri.v3);
        std::vector<Vec2f> edge3 = getOrCreateEdgeIFS(tri.u3, tri.v3, tri.u1, tri.v1);

        IFSTriangle ifsTri;
        ifsTri.r = tri.r;
        ifsTri.g = tri.g;
        ifsTri.b = tri.b;
        
        ifsTri.boundary.clear();
        
        // Assembler sans les derniers points (pour éviter doublons)
        for (size_t i = 0; i < edge1.size() - 1; ++i) {
            ifsTri.boundary.push_back(edge1[i]);
        }
        
        for (size_t i = 0; i < edge2.size() - 1; ++i) {
            ifsTri.boundary.push_back(edge2[i]);
        }
        
        for (size_t i = 0; i < edge3.size() - 1; ++i) {
            ifsTri.boundary.push_back(edge3[i]);
        }
        
        ifsTriangles.push_back(ifsTri);
    }
    
    std::cout << "✓ " << ifsTriangles.size() << " triangles IFS générés" << std::endl;
    std::cout << "=== DIAGNOSTIC ARÊTES ===" << std::endl;
    std::cout << "Arêtes demandées: " << totalEdgeRequests << std::endl;
    std::cout << "Arêtes uniques créées: " << edgeIFSCache.size() << std::endl;
    std::cout << "Taux de dédoublonnage: " 
              << (100.0f * edgeIFSCache.size() / totalEdgeRequests) << "%" << std::endl;
    std::cout << "Attendu (maillage parfait): ~" << (totalEdgeRequests / 2) << " arêtes" << std::endl;
    std::cout << "=========================" << std::endl;
}

void UVTriangleIFS::drawUVWireframe() const {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    
    for (const auto& ifsTri : ifsTriangles) {
        glColor3f(ifsTri.r, ifsTri.g, ifsTri.b);
        
        glBegin(GL_LINE_LOOP);
        for (const auto& pt : ifsTri.boundary) {
            glVertex2f(pt.u, pt.v);
        }
        glEnd();
    }
}

void UVTriangleIFS::drawUVFilled() const {
    glDisable(GL_LIGHTING);
    
    // 1. Dessiner les polygones remplis
    for (const auto& ifsTri : ifsTriangles) {
        if (ifsTri.boundary.size() < 3) continue;
        
        glColor3f(ifsTri.r, ifsTri.g, ifsTri.b);
        
        // Triangle fan depuis le premier point
        glBegin(GL_TRIANGLE_FAN);
        for (const auto& pt : ifsTri.boundary) {
            glVertex2f(pt.u, pt.v);
        }
        glEnd();
    }
    
    // 2. Dessiner les arêtes IFS en noir
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    
    for (const auto& edgeEntry : edgeIFSCache) {
        const std::vector<Vec2f>& edgePoints = edgeEntry.second;
        
        glBegin(GL_LINE_STRIP);
        for (const auto& pt : edgePoints) {
            glVertex2f(pt.u, pt.v);
        }
        glEnd();
    }
}

void UVTriangleIFS::draw3D(bool wireframe, bool explosion, float explosionFactor) const {
    if (!generatedMesh) return;

    if (explosion) {
        generatedMesh->drawExploded(explosionFactor);
    } else {
        generatedMesh->draw();
    }

    if (wireframe) {
        generatedMesh->drawWireframe();
    }
}

void UVTriangleIFS::buildIFSMesh(HalfEdgeMesh* mesh, ParametricMapping* mapping) {
    if (!mesh || !mapping) {
        std::cerr << "ERREUR: mesh ou mapping null!" << std::endl;
        return;
    }
    
    mesh->clear();
    
    std::cout << "Construction du maillage 3D IFS..." << std::endl;
    
    std::vector<DelaunayTriangle> simpleTriangles;
    int fragmentId = 0;
    int skippedSlivers = 0;
    
    for (const auto& ifsTri : ifsTriangles) {
        size_t n = ifsTri.boundary.size();
        if (n < 3) continue;

        auto addTri = [&](const Vec2f& p1, const Vec2f& p2, const Vec2f& p3) {
            float u1 = p1.u, v1 = p1.v;
            float u2 = p2.u, v2 = p2.v;
            float u3 = p3.u, v3 = p3.v;

            float cross_product = (u2 - u1) * (v3 - v1) - (v2 - v1) * (u3 - u1);
            
            if (std::abs(cross_product) < 0.000001f) {
                skippedSlivers++;
                return;
            }

            DelaunayTriangle tri;
            tri.r = ifsTri.r;
            tri.g = ifsTri.g;
            tri.b = ifsTri.b;
            tri.fragmentId = fragmentId;

            if (cross_product < 0) {
                tri.u1 = u1; tri.v1 = v1;
                tri.u2 = u3; tri.v2 = v3;
                tri.u3 = u2; tri.v3 = v2;
            } else {
                tri.u1 = u1; tri.v1 = v1;
                tri.u2 = u2; tri.v2 = v2;
                tri.u3 = u3; tri.v3 = v3;
            }
            simpleTriangles.push_back(tri);
        };

        if (n == 3) {
            addTri(ifsTri.boundary[0], ifsTri.boundary[1], ifsTri.boundary[2]);
        }
        else if (n == 4) {
            const auto& p0 = ifsTri.boundary[0];
            const auto& p1 = ifsTri.boundary[1];
            const auto& p2 = ifsTri.boundary[2];
            const auto& p3 = ifsTri.boundary[3];

            float dist02 = std::pow(p0.u - p2.u, 2) + std::pow(p0.v - p2.v, 2);
            float dist13 = std::pow(p1.u - p3.u, 2) + std::pow(p1.v - p3.v, 2);

            if (dist02 < dist13) {
                addTri(p0, p1, p2);
                addTri(p0, p2, p3);
            } else {
                addTri(p0, p1, p3);
                addTri(p1, p2, p3);
            }
        }
        else {
            float centerU = 0.0f;
            float centerV = 0.0f;
            for (const auto& p : ifsTri.boundary) {
                centerU += p.u;
                centerV += p.v;
            }
            centerU /= n;
            centerV /= n;
            Vec2f center = {centerU, centerV};

            for (size_t i = 0; i < n; ++i) {
                addTri(center, ifsTri.boundary[i], ifsTri.boundary[(i + 1) % n]);
            }
        }
        
        fragmentId++;
    }
    
    std::cout << "✓ Triangles créés: " << simpleTriangles.size() 
              << " (Rejetés: " << skippedSlivers << ")" << std::endl;
    
    // Construction via subdivision réduite
    int subdivisions = 2;
    mesh->buildFromDelaunayTriangulation(simpleTriangles, mapping, subdivisions);
    
    mesh->connectTwins();
    mesh->computeVertexNormals();
    
    std::cout << "✓ Maillage IFS terminé" << std::endl;
    
    // EXTRUSION
    if (triangleManager && triangleManager->isExtrusionEnabled()) {
        float depth = triangleManager->getExtrusionDepth();
        std::cout << "Application extrusion (depth=" << depth << ")..." << std::endl;
        mesh->extrudeMesh(depth);
        std::cout << "✓ Extrusion terminée" << std::endl;
    }
}