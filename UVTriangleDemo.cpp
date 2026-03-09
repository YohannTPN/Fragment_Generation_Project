#include "UVTriangleManager.h"
#include <GL/glut.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdlib>

// --- Distance cylindrique ---
// Pour la coordonnée v (angle), on prend en compte la périodicité [0, 1]
inline float distSqCylindrical(float u, float v, const UVPoint& p) {
    float du = u - p.u;
    
    // Distance angulaire minimale (périodique)
    float dv = std::abs(v - p.v);
    if (dv > 0.5f) dv = 1.0f - dv;  // wraparound
    
    return du * du + dv * dv;
}

// Trouve le site de Voronoi le plus proche en tenant compte de la topologie cylindrique
UVPoint UVTriangleManager::findClosestSite(float u, float v) const {
    if (voronoiSites.empty()) {
        return {0.0f, 0.0f, 0.5f, 0.5f, 0.5f}; 
    }

    float minDistSq = std::numeric_limits<float>::max();
    const UVPoint* closestSite = &voronoiSites[0];

    for (const auto& site : voronoiSites) {
        float dSq = distSqCylindrical(u, v, site);
        if (dSq < minDistSq) {
            minDistSq = dSq;
            closestSite = &site;
        }
    }
    return *closestSite;
}

void UVTriangleManager::generateVoronoiSites(int numSites, unsigned int seed) {
    if (numSites <= 0) return;
    
    std::srand(seed); 
    voronoiSites.clear();
    voronoiSites.reserve(numSites);

    for (int i = 0; i < numSites; ++i) {
        UVPoint site;
        site.u = static_cast<float>(std::rand()) / RAND_MAX;
        site.v = static_cast<float>(std::rand()) / RAND_MAX;

        // Couleurs aléatoires variées
        site.r = static_cast<float>(std::rand()) / RAND_MAX * 0.8f + 0.2f;
        site.g = static_cast<float>(std::rand()) / RAND_MAX * 0.8f + 0.2f;
        site.b = static_cast<float>(std::rand()) / RAND_MAX * 0.8f + 0.2f;

        voronoiSites.push_back(site);
    }
}

UVTriangleManager::UVTriangleManager(ParametricMapping* m) : mapping(m) {
    generateVoronoiSites(25);
}

void UVTriangleManager::addTriangle(const UVTriangle& tri) {
    triangles.push_back(tri);
}

#define N_GRID_SUBDIVISIONS 40  // Augmenté pour un meilleur rendu

void UVTriangleManager::draw() const {
    if (!mapping) return;
    
    // Désactiver l'éclairage pour un rendu plat du patchwork
    glDisable(GL_LIGHTING);
    
    if (!voronoiSites.empty()) {
        int N = N_GRID_SUBDIVISIONS;
        float du = 1.0f / N;
        float dv = 1.0f / N;

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                float u0 = i * du;
                float v0 = j * dv;
                float u1 = (i + 1) * du;
                float v1 = (j + 1) * dv;

                float u_center = u0 + du / 2.0f;
                float v_center = v0 + dv / 2.0f;
                UVPoint centerSite = findClosestSite(u_center, v_center);
                
                glColor3f(centerSite.r, centerSite.g, centerSite.b);

                glBegin(GL_QUADS);
                    Vec3f pA = mapping->map(u0, v0);
                    glVertex3f(pA.x, pA.y, pA.z);

                    Vec3f pB = mapping->map(u1, v0);
                    glVertex3f(pB.x, pB.y, pB.z);

                    Vec3f pC = mapping->map(u1, v1);
                    glVertex3f(pC.x, pC.y, pC.z);

                    Vec3f pD = mapping->map(u0, v1);
                    glVertex3f(pD.x, pD.y, pD.z);
                glEnd();
            }
        }
        
        // Lignes de séparation (bordures du patchwork)
        glColor3f(0.1f, 0.1f, 0.1f);
        glLineWidth(2.0f);
        
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                float u0 = i * du;
                float v0 = j * dv;
                float u1 = (i + 1) * du;
                float v1 = (j + 1) * dv;
                
                float u_center = u0 + du / 2.0f;
                float v_center = v0 + dv / 2.0f;
                UVPoint centerSite = findClosestSite(u_center, v_center);
                
                // Vérifier si les voisins ont des sites différents
                if (i < N - 1) {
                    UVPoint rightSite = findClosestSite(u1 + du / 2.0f, v_center);
                    if (rightSite.u != centerSite.u || rightSite.v != centerSite.v) {
                        glBegin(GL_LINES);
                            Vec3f pA = mapping->map(u1, v0);
                            Vec3f pB = mapping->map(u1, v1);
                            glVertex3f(pA.x, pA.y, pA.z);
                            glVertex3f(pB.x, pB.y, pB.z);
                        glEnd();
                    }
                }
                
                if (j < N - 1) {
                    UVPoint topSite = findClosestSite(u_center, v1 + dv / 2.0f);
                    if (topSite.u != centerSite.u || topSite.v != centerSite.v) {
                        glBegin(GL_LINES);
                            Vec3f pA = mapping->map(u0, v1);
                            Vec3f pB = mapping->map(u1, v1);
                            glVertex3f(pA.x, pA.y, pA.z);
                            glVertex3f(pB.x, pB.y, pB.z);
                        glEnd();
                    }
                }
            }
        }
    }
    
    // Réactiver l'éclairage pour les autres objets
    glEnable(GL_LIGHTING);
}

void UVTriangleManager::drawUVPatches() const {
    if (voronoiSites.empty()) return;
    
    int N = N_GRID_SUBDIVISIONS;
    float du = 1.0f / N;
    float dv = 1.0f / N;

    // Patchs colorés
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float u0 = i * du;
            float v0 = j * dv;
            float u1 = (i + 1) * du;
            float v1 = (j + 1) * dv;
            
            float u_center = u0 + du / 2.0f;
            float v_center = v0 + dv / 2.0f;
            UVPoint centerSite = findClosestSite(u_center, v_center);
            
            glColor3f(centerSite.r, centerSite.g, centerSite.b);

            glBegin(GL_QUADS);
                glVertex2f(u0, v0);
                glVertex2f(u1, v0);
                glVertex2f(u1, v1);
                glVertex2f(u0, v1);
            glEnd();
        }
    }
    
    // Lignes de séparation
    glColor3f(0.1f, 0.1f, 0.1f); 
    glLineWidth(2.0f);
    
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float u0 = i * du;
            float v0 = j * dv;
            float u1 = (i + 1) * du;
            float v1 = (j + 1) * dv;
            
            float u_center = u0 + du / 2.0f;
            float v_center = v0 + dv / 2.0f;
            UVPoint centerSite = findClosestSite(u_center, v_center);
            
            bool drawRight = false, drawTop = false;
            
            if (i < N - 1) {
                UVPoint rightSite = findClosestSite(u1 + du / 2.0f, v_center);
                drawRight = (rightSite.u != centerSite.u || rightSite.v != centerSite.v);
            }
            
            if (j < N - 1) {
                UVPoint topSite = findClosestSite(u_center, v1 + dv / 2.0f);
                drawTop = (topSite.u != centerSite.u || topSite.v != centerSite.v);
            }
            
            glBegin(GL_LINES);
            if (drawRight) {
                glVertex2f(u1, v0);
                glVertex2f(u1, v1);
            }
            if (drawTop) {
                glVertex2f(u0, v1);
                glVertex2f(u1, v1);
            }
            glEnd();
        }
    }
}