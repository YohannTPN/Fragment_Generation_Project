#include "HalfEdge.h"
#include <GL/glut.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <set>
#include <fstream>
#include <map>

HalfEdgeMesh::HalfEdgeMesh() : vertexIdCounter(0), faceIdCounter(0) {}

HalfEdgeMesh::~HalfEdgeMesh() { clear(); }

void HalfEdgeMesh::clear() {
    for (auto v : vertices) delete v;
    for (auto f : faces) delete f;
    for (auto he : halfEdges) delete he;
    
    vertices.clear();
    faces.clear();
    halfEdges.clear();
    vertexMap.clear();
    edgeMap.clear();
    
    vertexIdCounter = 0;
    faceIdCounter = 0;
}

std::string HalfEdgeMesh::makePosKey(float x, float y, float z) const {
    long ix = static_cast<long>(x * 1000.0f);
    long iy = static_cast<long>(y * 1000.0f);
    long iz = static_cast<long>(z * 1000.0f);
    
    std::ostringstream oss;
    oss << ix << "_" << iy << "_" << iz;
    return oss.str();
}

std::string HalfEdgeMesh::makeEdgeKey(int v1Id, int v2Id) const {
    std::ostringstream oss;
    oss << v1Id << "_" << v2Id;
    return oss.str();
}

Vertex* HalfEdgeMesh::addVertex(const Vec3f& pos, float u, float v) {
    Vertex* vertex = new Vertex(pos, u, v, vertexIdCounter++);
    vertices.push_back(vertex);
    return vertex;
}

Vertex* HalfEdgeMesh::getOrCreateVertex(float u, float v, ParametricMapping* mapping) {

    Vec3f pos = mapping->map(u, v);
    
    std::string key = makePosKey(pos.x, pos.y, pos.z);
    
    auto it = vertexMap.find(key);
    if (it != vertexMap.end()) {
        return it->second; 
    }
    
    Vertex* vertex = addVertex(pos, u, v);
    vertexMap[key] = vertex;
    return vertex;
}

Face* HalfEdgeMesh::addFace(Vertex* v1, Vertex* v2, Vertex* v3, float r, float g, float b) {
    return addFace(v1, v2, v3, r, g, b, -1);  
}

Face* HalfEdgeMesh::addFace(Vertex* v1, Vertex* v2, Vertex* v3, float r, float g, float b, int fragmentId) {
    
    if (v1 == v2 || v2 == v3 || v3 == v1) return nullptr;

    Face* face = new Face(faceIdCounter++);
    face->setColor(r, g, b);
    face->fragmentId = fragmentId;  
    faces.push_back(face);
    
    HalfEdge* he1 = new HalfEdge();
    HalfEdge* he2 = new HalfEdge();
    HalfEdge* he3 = new HalfEdge();
    
    halfEdges.push_back(he1);
    halfEdges.push_back(he2);
    halfEdges.push_back(he3);
    
    he1->vertex = v2; he1->face = face; he1->next = he2;
    he2->vertex = v3; he2->face = face; he2->next = he3;
    he3->vertex = v1; he3->face = face; he3->next = he1;
    
    face->halfEdge = he1;
    
    if (v1->halfEdge == nullptr) v1->halfEdge = he1;
    if (v2->halfEdge == nullptr) v2->halfEdge = he2;
    if (v3->halfEdge == nullptr) v3->halfEdge = he3;
    
    edgeMap[makeEdgeKey(v1->id, v2->id)] = he1;
    edgeMap[makeEdgeKey(v2->id, v3->id)] = he2;
    edgeMap[makeEdgeKey(v3->id, v1->id)] = he3;
    
    return face;
}

void HalfEdgeMesh::connectTwins() {
    for (auto& entry : edgeMap) {
        HalfEdge* he = entry.second;
        if (he->twin != nullptr) continue;
        
        Vertex* v1 = he->next->next->vertex;
        Vertex* v2 = he->vertex;
        
        std::string twinKey = makeEdgeKey(v2->id, v1->id);
        auto it = edgeMap.find(twinKey);
        
        if (it != edgeMap.end()) {
            HalfEdge* twin = it->second;
            he->twin = twin;
            twin->twin = he;
        }
    }
}

void HalfEdgeMesh::computeVertexNormals() {
    // Réinitialiser les normales de tous les sommets
    for (Vertex* v : vertices) {
        v->nx = 0.0f;
        v->ny = 0.0f;
        v->nz = 0.0f;
    }

    // Accumuler les normales des faces
    for (const Face* f : faces) {
        if (!f->halfEdge) continue;

        // Récupérer les sommets de la face
        Vertex* v1 = f->halfEdge->next->next->vertex;
        Vertex* v2 = f->halfEdge->vertex;
        Vertex* v3 = f->halfEdge->next->vertex;

        // Calculer la normale géométrique de la face
        Vec3f d1(v2->position.x - v1->position.x, v2->position.y - v1->position.y, v2->position.z - v1->position.z);
        Vec3f d2(v3->position.x - v1->position.x, v3->position.y - v1->position.y, v3->position.z - v1->position.z);

        // Produit vectoriel
        float fnx = d1.y*d2.z - d1.z*d2.y;
        float fny = d1.z*d2.x - d1.x*d2.z;
        float fnz = d1.x*d2.y - d1.y*d2.x;

        // Ajouter cette normale aux 3 sommets du triangle
        v1->nx += fnx; v1->ny += fny; v1->nz += fnz;
        v2->nx += fnx; v2->ny += fny; v2->nz += fnz;
        v3->nx += fnx; v3->ny += fny; v3->nz += fnz;
    }

    // Normaliser les résultats pour chaque sommet
    for (Vertex* v : vertices) {
        float len = sqrt(v->nx*v->nx + v->ny*v->ny + v->nz*v->nz);
        if (len > 1e-6) {
            v->nx /= len;
            v->ny /= len;
            v->nz /= len;
        } else {
            // Sécurité si normale nulle
            v->nx = 0.0f; v->ny = 1.0f; v->nz = 0.0f;
        }
    }
}

void HalfEdgeMesh::buildFromDelaunayTriangulation(
    const std::vector<DelaunayTriangle>& triangles,
    ParametricMapping* mapping,
    int subdivisions)
{
    clear();
    
    int autoFragmentIdCounter = 0;  // Pour auto-génération
    
    for (const auto& tri : triangles) {
        // Utiliser le fragmentId du triangle, OU auto-générer si -1
        int currentFragmentId = (tri.fragmentId >= 0) 
                                ? tri.fragmentId 
                                : autoFragmentIdCounter++;
        
        float step = 1.0f / subdivisions;
        
        for (int i = 0; i < subdivisions; ++i) {
            for (int j = 0; j < subdivisions - i; ++j) {
                float uA, vA, uB, vB, uC, vC;
                
                auto bary = [&](int _i, int _j, float& _u, float& _v) {
                    float alpha = (subdivisions - _i - _j) * step;
                    float beta = _j * step;
                    float gamma = _i * step;
                    _u = alpha * tri.u1 + beta * tri.u2 + gamma * tri.u3;
                    _v = alpha * tri.v1 + beta * tri.v2 + gamma * tri.v3;
                };
                
                bary(i, j, uA, vA);
                bary(i, j+1, uB, vB);
                bary(i+1, j, uC, vC);
                
                Vertex* vA_ptr = getOrCreateVertex(uA, vA, mapping);
                Vertex* vB_ptr = getOrCreateVertex(uB, vB, mapping);
                Vertex* vC_ptr = getOrCreateVertex(uC, vC, mapping);
                
                addFace(vA_ptr, vB_ptr, vC_ptr, tri.r, tri.g, tri.b, currentFragmentId);
                
                if (i + j < subdivisions - 1) {
                    float uD, vD;
                    bary(i+1, j+1, uD, vD);
                    Vertex* vD_ptr = getOrCreateVertex(uD, vD, mapping);
                    addFace(vB_ptr, vD_ptr, vC_ptr, tri.r, tri.g, tri.b, currentFragmentId);
                }
            }
        }
    }
    
    connectTwins();
    computeVertexNormals();
    
    // Compter les fragments uniques
    std::set<int> uniqueFragments;
    for (const auto& face : faces) {
        if (face->fragmentId >= 0) {
            uniqueFragments.insert(face->fragmentId);
        }
    }
    
    std::cout << "Mesh Construit : " << vertices.size() << " sommets, " 
              << uniqueFragments.size() << " fragments." << std::endl;
}

void HalfEdgeMesh::extrudeMesh(float depth) {
    if (depth == 0.0f) return;
    std::cout << "Extrusion (depth=" << depth << ")..." << std::endl;
    
    int originalFaceCount = faces.size();
    int originalVertexCount = vertices.size();
    
    std::unordered_map<int, Vertex*> extrudedVertices;
    
    // 1. Créer les sommets extrudés
    for (int i = 0; i < originalVertexCount; ++i) {
        Vertex* v = vertices[i];
        
        Vec3f normal(0, 0, 0);
        
        if (v->halfEdge) {
            HalfEdge* start = v->halfEdge;
            HalfEdge* curr = start;
            do {
                if (!curr->face) break;
                
                Vertex* p1 = curr->next->next->vertex;
                Vertex* p2 = curr->vertex;
                Vertex* p3 = curr->next->vertex;
                
                Vec3f d1(p2->position.x - p1->position.x, 
                        p2->position.y - p1->position.y, 
                        p2->position.z - p1->position.z);
                Vec3f d2(p3->position.x - p1->position.x, 
                        p3->position.y - p1->position.y, 
                        p3->position.z - p1->position.z);
                
                Vec3f n(d1.y*d2.z - d1.z*d2.y, 
                       d1.z*d2.x - d1.x*d2.z, 
                       d1.x*d2.y - d1.y*d2.x);
                normal.x += n.x; normal.y += n.y; normal.z += n.z;
                
                if (curr->twin) {
                    curr = curr->twin->next;
                } else {
                    break;
                }
            } while (curr != start);
        }
        
        float len = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
        if (len > 1e-6) { 
            normal.x /= len; normal.y /= len; normal.z /= len; 
        } else { 
            normal = Vec3f(0, 0, 1); 
        }
        
        Vec3f newPos(
            v->position.x + normal.x * depth,
            v->position.y + normal.y * depth,
            v->position.z + normal.z * depth
        );
        
        Vertex* extrudedV = addVertex(newPos, v->u, v->v);
        extrudedVertices[v->id] = extrudedV;
    }
    
    // 2. Sauvegarder les faces originales
    std::vector<Face*> originalFaces;
    for (int i = 0; i < originalFaceCount; ++i) {
        originalFaces.push_back(faces[i]);
    }
    
    // 3. Créer la coque inférieure avec MÊME COULEUR et MÊME fragmentId
    for (Face* f : originalFaces) {
        if (!f->halfEdge) continue;
        
        Vertex* v1 = f->halfEdge->next->next->vertex;
        Vertex* v2 = f->halfEdge->vertex;
        Vertex* v3 = f->halfEdge->next->vertex;
        
        
        addFace(extrudedVertices[v1->id], 
                extrudedVertices[v3->id], 
                extrudedVertices[v2->id], 
                f->r, f->g, f->b,           
                f->fragmentId);              
    }
    
    // 4. Créer les murs latéraux
std::vector<HalfEdge*> borderEdges;
std::set<std::pair<int, int>> processedExteriorEdges;  // Seulement pour bordures extérieures

// Compteurs pour debug
int exteriorBorders = 0;
int interiorBorders = 0;

for (HalfEdge* he : halfEdges) {
    if (!he->face) continue;  // Ignorer les arêtes orphelines
    
    bool isExteriorBorder = false;
    bool isInteriorBorder = false;
    
    // CAS 1 : Arête extérieure du domaine UV (bordure absolue)
    if (he->twin == nullptr) {
        isExteriorBorder = true;
    }
    // CAS 2 : Arête entre deux fragments différents (frontière interne)
    else if (he->twin->face && 
             he->face->fragmentId != he->twin->face->fragmentId) {
        isInteriorBorder = true;
    }
    
    // Traitement des bordures extérieures (avec déduplication)
    if (isExteriorBorder) {
        Vertex* topV1 = he->next->next->vertex;
        Vertex* topV2 = he->vertex;
        
        // Vérifier que ces sommets appartiennent à la couche TOP
        if (extrudedVertices.find(topV1->id) == extrudedVertices.end() ||
            extrudedVertices.find(topV2->id) == extrudedVertices.end()) {
            continue;
        }
        
        // Déduplication pour bordures extérieures uniquement
        int minId = std::min(topV1->id, topV2->id);
        int maxId = std::max(topV1->id, topV2->id);
        auto edgeKey = std::make_pair(minId, maxId);
        
        if (processedExteriorEdges.find(edgeKey) != processedExteriorEdges.end()) {
            continue;  // Déjà traité
        }
        
        processedExteriorEdges.insert(edgeKey);
        borderEdges.push_back(he);
        exteriorBorders++;
    }
    
    // Traitement des frontières internes (SANS déduplication)
    if (isInteriorBorder) {
        Vertex* topV1 = he->next->next->vertex;
        Vertex* topV2 = he->vertex;
        
        // Vérifier que ces sommets appartiennent à la couche TOP
        if (extrudedVertices.find(topV1->id) == extrudedVertices.end() ||
            extrudedVertices.find(topV2->id) == extrudedVertices.end()) {
            continue;
        }
        
        // PAS de déduplication ici → on garde les deux côtés
        borderEdges.push_back(he);
        interiorBorders++;
    }
}

std::cout << "=== Détection des frontières ===" << std::endl;
std::cout << "  Bordures extérieures: " << exteriorBorders << std::endl;
std::cout << "  Frontières internes (avec doublons): " << interiorBorders << std::endl;
std::cout << "  Total de murs à créer: " << borderEdges.size() << std::endl;

// Créer les quads (2 triangles) pour chaque mur
int wallsCreated = 0;
for (HalfEdge* he : borderEdges) {
    Vertex* topV1 = he->next->next->vertex;  // Origine TOP
    Vertex* topV2 = he->vertex;              // Destination TOP
    
    Vertex* botV1 = extrudedVertices[topV1->id];  // Origine BOTTOM
    Vertex* botV2 = extrudedVertices[topV2->id];  // Destination BOTTOM
    
    // Hériter de la couleur et du fragmentId de la face parent
    Face* parentFace = he->face;
    float r = parentFace->r;
    float g = parentFace->g;
    float b = parentFace->b;
    int fragId = parentFace->fragmentId;
    
    // Créer le quad comme 2 triangles
    // Triangle 1: topV1 -> botV1 -> botV2
    addFace(topV1, botV1, botV2, r, g, b, fragId);
    
    // Triangle 2: topV1 -> botV2 -> topV2
    addFace(topV1, botV2, topV2, r, g, b, fragId);
    
    wallsCreated += 2;
}

std::cout << "  Murs créés: " << wallsCreated << " faces triangulaires" << std::endl;
}


void HalfEdgeMesh::draw() const {
    glEnable(GL_LIGHTING);
    glEnable(GL_RESCALE_NORMAL); 

    glBegin(GL_TRIANGLES);
    for (const Face* face : faces) {
        if (!face->halfEdge) continue;

        glColor3f(face->r, face->g, face->b);
        
        Vertex* v1 = face->halfEdge->next->next->vertex;
        Vertex* v2 = face->halfEdge->vertex;
        Vertex* v3 = face->halfEdge->next->vertex;
        
        // SOMMET 1 : Normale puis Position
        glNormal3f(v1->nx, v1->ny, v1->nz);
        glVertex3f(v1->position.x, v1->position.y, v1->position.z);
        
        // SOMMET 2
        glNormal3f(v2->nx, v2->ny, v2->nz);
        glVertex3f(v2->position.x, v2->position.y, v2->position.z);
        
        // SOMMET 3
        glNormal3f(v3->nx, v3->ny, v3->nz);
        glVertex3f(v3->position.x, v3->position.y, v3->position.z);
    }
    glEnd();
}

void HalfEdgeMesh::drawWireframe() const {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D); // Juste au cas où
    
    glColor3f(0.0f, 0.0f, 0.0f); // Noir
    glLineWidth(1.0f);
    
    glBegin(GL_LINES);
    
   
    
    
    
    std::set<std::string> drawnEdges; // Pour éviter les doublons si vous préférez

    for (const auto& he : halfEdges) {
        if (!he->face) continue; // Ignorer les arêtes "orphelines" si elles existent

        Vertex* v1 = he->next->next->vertex; // Origine
        Vertex* v2 = he->vertex;             // Destination
        
        
        if (he->twin == nullptr || v1->id < v2->id) {
            glVertex3f(v1->position.x, v1->position.y, v1->position.z);
            glVertex3f(v2->position.x, v2->position.y, v2->position.z);
        }
    }
    
    glEnd();
    glEnable(GL_LIGHTING);
}

void HalfEdgeMesh::drawExploded(float factor) const {
    if (vertices.empty() || faces.empty()) return;

    // 1. Calcul du barycentre global
    float sumX = 0, sumY = 0, sumZ = 0;
    for (const auto& v : vertices) {
        sumX += v->position.x;
        sumY += v->position.y;
        sumZ += v->position.z;
    }
    float count = (float)vertices.size();
    Vec3f G_global = { sumX / count, sumY / count, sumZ / count };

    // 2. Regrouper les faces par fragmentId
    std::map<int, std::vector<const Face*>> fragments;
    
    for (const Face* face : faces) {
        if (face->fragmentId >= 0) {  // Ignorer les faces sans fragment
            fragments[face->fragmentId].push_back(face);
        }
    }
    
    std::cout << "Vue éclatée : " << fragments.size() << " fragments détectés" << std::endl;

    
    glEnable(GL_LIGHTING);
    glEnable(GL_RESCALE_NORMAL);
    
    for (const auto& [fragId, facesInFragment] : fragments) {
        // Calculer le barycentre du fragment complet
        Vec3f fragmentCenter = {0, 0, 0};
        int vertexCount = 0;
        
        for (const Face* face : facesInFragment) {
            if (!face->halfEdge) continue;
            
            Vertex* v1 = face->halfEdge->next->next->vertex;
            Vertex* v2 = face->halfEdge->vertex;
            Vertex* v3 = face->halfEdge->next->vertex;
            
            fragmentCenter.x += v1->position.x + v2->position.x + v3->position.x;
            fragmentCenter.y += v1->position.y + v2->position.y + v3->position.y;
            fragmentCenter.z += v1->position.z + v2->position.z + v3->position.z;
            vertexCount += 3;
        }
        
        if (vertexCount == 0) continue;
        
        fragmentCenter.x /= vertexCount;
        fragmentCenter.y /= vertexCount;
        fragmentCenter.z /= vertexCount;
        
        // Vecteur de déplacement : du centre global vers le centre du fragment
        Vec3f displacement = {
            fragmentCenter.x - G_global.x,
            fragmentCenter.y - G_global.y,
            fragmentCenter.z - G_global.z
        };
        
        // Normalisation
        float len = sqrt(displacement.x*displacement.x + 
                        displacement.y*displacement.y + 
                        displacement.z*displacement.z);
        if (len > 1e-6) {
            displacement.x /= len;
            displacement.y /= len;
            displacement.z /= len;
        }
        
        // Application du facteur d'éclatement
        displacement.x *= factor;
        displacement.y *= factor;
        displacement.z *= factor;
        
        
        for (const Face* face : facesInFragment) {
            if (!face->halfEdge || !face->halfEdge->next || !face->halfEdge->next->next) continue;
            
            Vertex* v1 = face->halfEdge->next->next->vertex;
            Vertex* v2 = face->halfEdge->vertex;
            Vertex* v3 = face->halfEdge->next->vertex;
            
            glBegin(GL_TRIANGLES);
                glColor3f(face->r, face->g, face->b);
                
                glNormal3f(v1->nx, v1->ny, v1->nz);
                glVertex3f(v1->position.x + displacement.x, 
                          v1->position.y + displacement.y, 
                          v1->position.z + displacement.z);
                
                glNormal3f(v2->nx, v2->ny, v2->nz);
                glVertex3f(v2->position.x + displacement.x, 
                          v2->position.y + displacement.y, 
                          v2->position.z + displacement.z);
                
                glNormal3f(v3->nx, v3->ny, v3->nz);
                glVertex3f(v3->position.x + displacement.x, 
                          v3->position.y + displacement.y, 
                          v3->position.z + displacement.z);
            glEnd();
        }
    }
}

void HalfEdgeMesh::exportOBJ(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "# IFS Mesh Export\n";

    // Grouper les faces par fragmentId
    std::map<int, std::vector<Face*>> fragmentFaces;
    for (auto* f : faces) {
        fragmentFaces[f->fragmentId].push_back(f);
    }

    int vertexOffset = 1;  // OBJ est 1-based

    for (const auto& group : fragmentFaces) {
        file << "g fragment_" << group.first << "\n";

        // Collecter les vertices de CE fragment uniquement
        std::map<int, int> localIndex;  // id global → index local dans ce groupe
        int localCount = 0;

        for (const auto* f : group.second) {
            if (!f->halfEdge) continue;
            HalfEdge* he = f->halfEdge;
            Vertex* verts[3] = {
                he->vertex,
                he->next->vertex,
                he->next->next->vertex
            };
            for (auto* v : verts) {
                if (localIndex.find(v->id) == localIndex.end()) {
                    localIndex[v->id] = localCount++;
                    file << "v " << v->position.x << " "
                                 << v->position.y << " "
                                 << v->position.z << "\n";
                }
            }
        }

        // Écrire les faces avec indices locaux
        for (const auto* f : group.second) {
            if (!f->halfEdge) continue;
            HalfEdge* he = f->halfEdge;
            file << "f "
                 << (localIndex[he->vertex->id]             + vertexOffset) << " "
                 << (localIndex[he->next->vertex->id]       + vertexOffset) << " "
                 << (localIndex[he->next->next->vertex->id] + vertexOffset) << "\n";
        }

        vertexOffset += localCount;
    }

    file.close();
    std::cout << "✓ Export OBJ : " << filename
              << " (" << fragmentFaces.size() << " fragments)" << std::endl;
}