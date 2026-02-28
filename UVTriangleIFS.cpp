#include "UVTriangleIFS.h"
#include <iostream>
#include <GL/glut.h>
#include <cmath>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

struct VertexInfo {
    float r, g, b;  // Couleur du triangle d'origine
};

typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> Delaunay;
typedef K::Point_2 Point_2;

UVTriangleIFS::UVTriangleIFS(UVTriangleManager* triManager) 
    : triangleManager(triManager), currentDepth(0), generatedMesh(new HalfEdgeMesh()) 
{
}



void UVTriangleIFS::setTransforms(
    const IFSTransform& Ti,
    const IFSTransform& Tij,
    const IFSTransform& Tj
) {
    ifsManager.setGlobalTransforms(Ti, Tij, Tj);
}

std::vector<Vec2f> UVTriangleIFS::getOrCreateEdgeIFS(float u1, float v1, float u2, float v2) {
    EdgeKey key(u1, v1, u2, v2);
    
    // Détecter si on doit inverser
    long qu1 = (long)std::round(u1 * 100000.0f);
    long qv1 = (long)std::round(v1 * 100000.0f);
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
    const long epsilon = (long)(0.001f * 100000.0f);
    const long quantMax = (long)(1.0f * 100000.0f);
    
    bool onLeftBorder = (std::abs(key.u1) < epsilon && std::abs(key.u2) < epsilon);
    bool onRightBorder = (std::abs(key.u1 - quantMax) < epsilon && std::abs(key.u2 - quantMax) < epsilon);
    bool onBottomBorder = (std::abs(key.v1) < epsilon && std::abs(key.v2) < epsilon);
    bool onTopBorder = (std::abs(key.v1 - quantMax) < epsilon && std::abs(key.v2 - quantMax) < epsilon);
    
    bool isBorderEdge = onLeftBorder || onRightBorder || onBottomBorder || onTopBorder;
    
    if (isBorderEdge) {
        float fu1 = key.u1 / 100000.0f;
        float fv1 = key.v1 / 100000.0f;
        float fu2 = key.u2 / 100000.0f;
        float fv2 = key.v2 / 100000.0f;
        
        std::vector<Vec2f> straightLine;
        straightLine.push_back(Vec2f(fu1, fv1));
        straightLine.push_back(Vec2f(fu2, fv2));
        edgeIFSCache[key] = straightLine;
        
        if (needsReversal) {
            std::reverse(straightLine.begin(), straightLine.end());
        }
        return straightLine;
    }
    
    // Générer les points IFS
    float fu1 = key.u1 / 100000.0f;
    float fv1 = key.v1 / 100000.0f;
    float fu2 = key.u2 / 100000.0f;
    float fv2 = key.v2 / 100000.0f;
    
    auto edge = std::make_unique<IFSEdge>(Vec2f(fu1, fv1), Vec2f(fu2, fv2));
    
    float dx = fu2 - fu1;
    float dy = fv2 - fv1;
    float length = std::sqrt(dx * dx + dy * dy);
    
    float perpX = -dy / length;
    float perpY = dx / length;
    
    float amplitude = length * 0.02f;
    
    Vec2f p1;
    p1.u = fu1 + dx / 3.0f + perpX * amplitude;
    p1.v = fv1 + dy / 3.0f + perpY * amplitude;
    
    Vec2f p2;
    p2.u = fu1 + 2.0f * dx / 3.0f - perpX * amplitude;
    p2.v = fv1 + 2.0f * dy / 3.0f - perpY * amplitude;
    
    edge->setControlPoints(p1, p2);
    edge->setTransforms(
        ifsManager.globalTi, 
        ifsManager.globalTij, 
        ifsManager.globalTj
    );
    
    std::vector<Vec2f> points = edge->generate(currentDepth);
    
    // === QUANTIFIER TOUS LES POINTS AVANT MISE EN CACHE ===
    const float quantPrecision = 100000.0f;
    for (auto& pt : points) {
        pt.u = std::round(pt.u * quantPrecision) / quantPrecision;
        pt.v = std::round(pt.v * quantPrecision) / quantPrecision;
    }
    
    // S'assurer que le premier et dernier point sont EXACTEMENT
    // les sommets quantifiés de l'arête
    if (!points.empty()) {
        points[0].u = fu1;
        points[0].v = fv1;
        points[points.size() - 1].u = fu2;
        points[points.size() - 1].v = fv2;
    }
    

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
    
    for (const auto& tri : triangles) {
        std::vector<Vec2f> edge1 = getOrCreateEdgeIFS(tri.u1, tri.v1, tri.u2, tri.v2);
        std::vector<Vec2f> edge2 = getOrCreateEdgeIFS(tri.u2, tri.v2, tri.u3, tri.v3);
        std::vector<Vec2f> edge3 = getOrCreateEdgeIFS(tri.u3, tri.v3, tri.u1, tri.v1);


        static bool firstTime = true;
        if (firstTime) {
            std::cout << "=== DIAGNOSTIC ARÊTES IFS ===" << std::endl;
            std::cout << "Triangle original: (" << tri.u1 << "," << tri.v1 << ") -> (" 
                      << tri.u2 << "," << tri.v2 << ") -> (" << tri.u3 << "," << tri.v3 << ")" << std::endl;
            std::cout << "Edge1 premier point: (" << edge1[0].u << "," << edge1[0].v << ")" << std::endl;
            std::cout << "Edge1 dernier point: (" << edge1[edge1.size()-1].u << "," << edge1[edge1.size()-1].v << ")" << std::endl;
            std::cout << "Edge2 premier point: (" << edge2[0].u << "," << edge2[0].v << ")" << std::endl;
            std::cout << "Différence edge1.last vs edge2.first (u): " 
                      << std::abs(edge1[edge1.size()-1].u - edge2[0].u) << std::endl;
            std::cout << "Différence edge1.last vs edge2.first (v): " 
                      << std::abs(edge1[edge1.size()-1].v - edge2[0].v) << std::endl;
            firstTime = false;
        }
        
        IFSTriangle ifsTri;
        ifsTri.r = tri.r;
        ifsTri.g = tri.g;
        ifsTri.b = tri.b;
        
        ifsTri.boundary.clear();
        
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
    
    std::cout << "Construction du maillage 3D IFS (Smart Triangulation)..." << std::endl;
    
    std::vector<DelaunayTriangle> simpleTriangles;
    int fragmentId = 0;
    int skippedSlivers = 0;
    
    for (const auto& ifsTri : ifsTriangles) {
        size_t n = ifsTri.boundary.size();
        if (n < 3) continue;

        // Lambda pour ajouter un triangle proprement avec vérification d'aire et d'orientation
        auto addTri = [&](const Vec2f& p1, const Vec2f& p2, const Vec2f& p3) {
            float u1 = p1.u, v1 = p1.v;
            float u2 = p2.u, v2 = p2.v;
            float u3 = p3.u, v3 = p3.v;

            // 1. Calcul de l'aire signée (Cross Product 2D)
            float cross_product = (u2 - u1) * (v3 - v1) - (v2 - v1) * (u3 - u1);
            
            // 2. FILTRE ANTI-PICS : Si le triangle est trop plat (aire quasi nulle), on le jette.
            if (std::abs(cross_product) < 0.000001f) {
                skippedSlivers++;
                return;
            }

            DelaunayTriangle tri;
            tri.r = ifsTri.r;
            tri.g = ifsTri.g;
            tri.b = ifsTri.b;
            tri.fragmentId = fragmentId;

            // 3. Correction du Winding (Toujours Anti-Horaire)
            if (cross_product < 0) {
                // Sens Horaire -> On inverse pour avoir une normale sortante
                tri.u1 = u1; tri.v1 = v1;
                tri.u2 = u3; tri.v2 = v3; // Swap 2 et 3
                tri.u3 = u2; tri.v3 = v2;
            } else {
                tri.u1 = u1; tri.v1 = v1;
                tri.u2 = u2; tri.v2 = v2;
                tri.u3 = u3; tri.v3 = v3;
            }
            simpleTriangles.push_back(tri);
        };

        // --- STRATÉGIE DE TRIANGULATION ---
        
        if (n == 3) {
            // Cas simple : Triangle
            addTri(ifsTri.boundary[0], ifsTri.boundary[1], ifsTri.boundary[2]);
        }
        else if (n == 4) {
            // Cas Quadrilatère : "Shortest Diagonal"
            // On coupe selon la diagonale la plus courte pour mieux respecter la concavité
            const auto& p0 = ifsTri.boundary[0];
            const auto& p1 = ifsTri.boundary[1];
            const auto& p2 = ifsTri.boundary[2];
            const auto& p3 = ifsTri.boundary[3];

            float dist02 = std::pow(p0.u - p2.u, 2) + std::pow(p0.v - p2.v, 2);
            float dist13 = std::pow(p1.u - p3.u, 2) + std::pow(p1.v - p3.v, 2);

            if (dist02 < dist13) {
                // Coupure 0-2
                addTri(p0, p1, p2);
                addTri(p0, p2, p3);
            } else {
                // Coupure 1-3
                addTri(p0, p1, p3);
                addTri(p1, p2, p3);
            }
        }
        else {
            // Cas N > 4 : Méthode Centroïde (Repli si nécessaire)
            float centerU = 0.0f;
            float centerV = 0.0f;
            for (const auto& p : ifsTri.boundary) {
                centerU += p.u;
                centerV += p.v;
            }
            centerU /= n;
            centerV /= n;
            Vec2f center = {centerU, centerV}; // Supposant que Vec2f a un constructeur ou {u, v}

            for (size_t i = 0; i < n; ++i) {
                addTri(center, ifsTri.boundary[i], ifsTri.boundary[(i + 1) % n]);
            }
        }
        
        fragmentId++;
    }
    
    std::cout << "✓ Triangles valides: " << simpleTriangles.size() 
              << " (Rejetés car trop plats: " << skippedSlivers << ")" << std::endl;
    
    // Construction via Delaunay/Subdivision
    int subdivisions = 3; 
    mesh->buildFromDelaunayTriangulation(simpleTriangles, mapping, subdivisions);
    
    // Finalisation
    mesh->connectTwins();
    mesh->computeVertexNormals();
    
    // EXTRUSION
    if (triangleManager && triangleManager->isExtrusionEnabled()) {
        float depth = triangleManager->getExtrusionDepth();
        mesh->extrudeMesh(depth);
    }
}