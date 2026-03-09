#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "ParametricMapping.h"

// Forward declarations
struct HalfEdge;
struct Vertex;
struct Face;

// Définition de DelaunayTriangle 
struct DelaunayTriangle {
    float u1, v1;
    float u2, v2;
    float u3, v3;
    float r, g, b;
    int fragmentId = -1; 
};

// ============================================
// VERTEX - Sommet du maillage
// ============================================
struct Vertex {
    Vec3f position;           // Position 3D
    float u, v;               // Coordonnées paramétriques
    HalfEdge* halfEdge;       // Une demi-arête sortante
    int id;                   // Identifiant unique
    float nx, ny, nz;         // Composantes de la normale au sommet
    
    Vertex(const Vec3f& pos, float u_param, float v_param, int id_val)
        : position(pos), u(u_param), v(v_param), halfEdge(nullptr), id(id_val), nx(0.0f), ny(0.0f), nz(0.0f){}
};

// ============================================
// FACE - Face triangulaire
// ============================================
struct Face {
    HalfEdge* halfEdge;       // Une demi-arête de la face
    float r, g, b;            // Couleur de la face
    int id;                   // Identifiant unique de la face
    int fragmentId;           // NOUVEAU : Identifiant du fragment parent
    
    Face(int id_val) : halfEdge(nullptr), r(0.7f), g(0.7f), b(0.7f), 
                       id(id_val), fragmentId(-1) {}  // -1 = non assigné
    
    void setColor(float red, float green, float blue) {
        r = red; g = green; b = blue;
    }
};

// ============================================
// HALF-EDGE - Demi-arête orientée
// ============================================
struct HalfEdge {
    Vertex* vertex;           // Sommet destination
    HalfEdge* twin;           // Demi-arête opposée
    HalfEdge* next;           // Demi-arête suivante dans la face
    Face* face;               // Face à gauche
    
    HalfEdge() : vertex(nullptr), twin(nullptr), next(nullptr), face(nullptr) {}
};

// ============================================
// HALF-EDGE MESH - Maillage complet
// ============================================
class HalfEdgeMesh {
    friend class UVTriangleIFS;
public:
    HalfEdgeMesh();
    ~HalfEdgeMesh();
    
    // Construction du maillage
    Vertex* addVertex(const Vec3f& pos, float u, float v);
    Face* addFace(Vertex* v1, Vertex* v2, Vertex* v3, float r, float g, float b);
    Face* addFace(Vertex* v1, Vertex* v2, Vertex* v3, float r, float g, float b, int fragmentId);
    
    // Construction depuis une triangulation de Delaunay
    void buildFromDelaunayTriangulation(
        const std::vector<struct DelaunayTriangle>& triangles,
        ParametricMapping* mapping,
        int subdivisions
    );
    
    // Extrusion du maillage
    void extrudeMesh(float depth);
    
    // Dessin
    void draw() const;
    void drawWireframe() const;
    void computeVertexNormals();
    void drawExploded(float factor) const;
    
    // Accesseurs
    const std::vector<Vertex*>& getVertices() const { return vertices; }
    const std::vector<Face*>& getFaces() const { return faces; }
    const std::vector<HalfEdge*>& getHalfEdges() const { return halfEdges; }
    
    // Utilitaires
    void clear();
    void computeNormals(); 
    
private:
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;
    std::vector<HalfEdge*> halfEdges;
    
    std::unordered_map<std::string, Vertex*> vertexMap;  
    std::unordered_map<std::string, HalfEdge*> edgeMap; 
    
    int vertexIdCounter;
    int faceIdCounter;
    
    std::string makePosKey(float x, float y, float z) const;
    
    // Génère une clé unique pour une arête (v1->v2)
    std::string makeEdgeKey(int v1Id, int v2Id) const;
    
    // Trouve ou crée un sommet
    Vertex* getOrCreateVertex(float u, float v, ParametricMapping* mapping);
    
    // Connecte les twins des half-edges
    void connectTwins();
};