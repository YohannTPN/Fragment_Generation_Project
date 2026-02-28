#include "UVTriangleManager.h"
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <cstdlib>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<ColoredUVPoint, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> Delaunay;
typedef K::Point_2 Point_2;

UVTriangleManager::UVTriangleManager(ParametricMapping* m) 
    : mapping(m), extrusionDepth(0.1f), extrusionEnabled(true), 
      wireframeEnabled(false), explosionEnabled(false), explosionFactor(0.5f), mesh(nullptr) {
    mesh = new HalfEdgeMesh();
    generateDelaunayTriangulation(30, std::time(nullptr));
}

UVTriangleManager::~UVTriangleManager() {
    delete mesh;
}

void UVTriangleManager::setExtrusionDepth(float depth) {
    extrusionDepth = depth;
    rebuildMesh();
}

void UVTriangleManager::setExtrusionEnabled(bool enabled) {
    extrusionEnabled = enabled;
    rebuildMesh();
}

void UVTriangleManager::rebuildMesh() {
    if (!mesh) return;
    
    // Reconstruire le maillage de base avec MOINS de subdivisions
    mesh->buildFromDelaunayTriangulation(triangles, mapping, 4);  // Réduit de 8 à 4
    
    // Appliquer l'extrusion si activée
    if (extrusionEnabled) {
        mesh->extrudeMesh(extrusionDepth);
    }

   
}

void UVTriangleManager::generateDelaunayTriangulation(int numPoints, unsigned int seed) {
    std::srand(seed);
    
    points.clear();
    triangles.clear();
    
    // Étape 1 : Créer une grille régulière avec perturbation contrôlée
    // Calculer le nombre de points par dimension pour avoir environ numPoints au total
    int gridSize = static_cast<int>(std::sqrt(static_cast<float>(numPoints))) + 1;
    float cellSize = 1.0f / gridSize;
    float perturbationAmount = cellSize * 0.35f; // 35% de la taille de cellule max
    
    std::cout << "Génération d'une grille " << gridSize << "x" << gridSize 
              << " avec perturbation..." << std::endl;
    
    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            ColoredUVPoint pt;
            
            // Position de base sur la grille
            float baseU = static_cast<float>(i) / gridSize;
            float baseV = static_cast<float>(j) / gridSize;
            
            // Perturbation aléatoire, mais pas pour les bords
            float perturbU = 0.0f;
            float perturbV = 0.0f;
            
            bool isOnBorder = (i == 0 || i == gridSize || j == 0 || j == gridSize);
            
            if (!isOnBorder) {
                // Perturbation uniforme dans [-perturbationAmount, +perturbationAmount]
                perturbU = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f) * perturbationAmount;
                perturbV = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f) * perturbationAmount;
            }
            
            // Position finale avec clamping pour rester dans [0,1]
            pt.u = std::max(0.0f, std::min(1.0f, baseU + perturbU));
            pt.v = std::max(0.0f, std::min(1.0f, baseV + perturbV));
            
            // Couleur aléatoire
            pt.r = 0.3f + static_cast<float>(std::rand()) / RAND_MAX * 0.6f;
            pt.g = 0.3f + static_cast<float>(std::rand()) / RAND_MAX * 0.6f;
            pt.b = 0.3f + static_cast<float>(std::rand()) / RAND_MAX * 0.6f;
            
            // Couleur grise pour les points de bord (pour visualisation)
            if (isOnBorder) {
                pt.r = 0.6f;
                pt.g = 0.6f;
                pt.b = 0.6f;
            }
            
            points.push_back(pt);
        }
    }
    
    std::cout << "Total : " << points.size() << " points générés." << std::endl;
    
    // Étape 3 : Construire la triangulation de Delaunay
    Delaunay dt;
    
    for (const auto& pt : points) {
        Point_2 p(pt.u, pt.v);
        auto vh = dt.insert(p);
        vh->info() = pt;
    }
    
    std::cout << "Triangulation construite, extraction des triangles..." << std::endl;
    
    // Étape 4 : Extraire les triangles
    for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit) {
        auto v0 = fit->vertex(0);
        auto v1 = fit->vertex(1);
        auto v2 = fit->vertex(2);
        
        Point_2 p0 = v0->point();
        Point_2 p1 = v1->point();
        Point_2 p2 = v2->point();
        
        ColoredUVPoint c0 = v0->info();
        ColoredUVPoint c1 = v1->info();
        ColoredUVPoint c2 = v2->info();
        
        DelaunayTriangle tri;
        tri.u1 = p0.x(); tri.v1 = p0.y();
        tri.u2 = p1.x(); tri.v2 = p1.y();
        tri.u3 = p2.x(); tri.v3 = p2.y();
        
        // Couleur moyenne
        tri.r = (c0.r + c1.r + c2.r) / 3.0f;
        tri.g = (c0.g + c1.g + c2.g) / 3.0f;
        tri.b = (c0.b + c1.b + c2.b) / 3.0f;
        
        triangles.push_back(tri);
    }
    
    std::cout << "✓ " << triangles.size() << " triangles générés." << std::endl;
    
    // Construire le maillage half-edge
    rebuildMesh();
}

void UVTriangleManager::draw() const {
    if (!mesh) return;
    
    if (explosionEnabled) {
        // Vue éclatée avec facteur ajustable
        mesh->drawExploded(explosionFactor); 
    } else {
        // Vue normale
        mesh->draw();
    }

    // Le wireframe est dessiné par-dessus dans tous les cas si activé
    if (wireframeEnabled) {
        mesh->drawWireframe();
    }
}

void UVTriangleManager::drawUVPatches() const {
    // Dessiner les triangles remplis
    for (const auto& tri : triangles) {
        glColor3f(tri.r, tri.g, tri.b);
        glBegin(GL_TRIANGLES);
            glVertex2f(tri.u1, tri.v1);
            glVertex2f(tri.u2, tri.v2);
            glVertex2f(tri.u3, tri.v3);
        glEnd();
    }
    
    // Dessiner les arêtes
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);
    
    for (const auto& tri : triangles) {
        glBegin(GL_LINE_LOOP);
            glVertex2f(tri.u1, tri.v1);
            glVertex2f(tri.u2, tri.v2);
            glVertex2f(tri.u3, tri.v3);
        glEnd();
    }
    
    // Dessiner les points générateurs (optionnel)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (const auto& pt : points) {
        // Ne dessiner que les points non-bordure
        if (pt.r != 0.6f || pt.g != 0.6f || pt.b != 0.6f) {
            glVertex2f(pt.u, pt.v);
        }
    }
    glEnd();
}