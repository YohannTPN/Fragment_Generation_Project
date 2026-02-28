#pragma once
#include <vector>
#include "ParametricMapping.h"
#include "HalfEdge.h"

// Point coloré dans l'espace UV
struct ColoredUVPoint {
    float u, v;
    float r, g, b;
};

// DelaunayTriangle est maintenant défini dans HalfEdge.h

class UVTriangleManager {
public:
    UVTriangleManager(ParametricMapping* mapping);
    ~UVTriangleManager();
    
    // Génère une triangulation de Delaunay à partir de points aléatoires
    void generateDelaunayTriangulation(int numPoints, unsigned int seed = 42);
    
    // Dessine la triangulation sur la surface 3D
    void draw() const;
    
    // Dessine la triangulation dans l'espace UV (fenêtre 2D)
    void drawUVPatches() const;
    
    // Configure l'extrusion des triangles
    void setExtrusionDepth(float depth);
    void setExtrusionEnabled(bool enabled);
    void setWireframeEnabled(bool enabled) { wireframeEnabled = enabled; }
    void setExplosionEnabled(bool enabled) { explosionEnabled = enabled; }
    void rebuildMesh();  // Reconstruit le maillage half-edge
    
    // Accesseurs
    const std::vector<DelaunayTriangle>& getTriangles() const { return triangles; }
    const std::vector<ColoredUVPoint>& getPoints() const { return points; }
    float getExtrusionDepth() const { return extrusionDepth; }
    bool isExtrusionEnabled() const { return extrusionEnabled; }
    bool isWireframeEnabled() const { return wireframeEnabled; }
    bool isExplosionEnabled() const { return explosionEnabled; }
    HalfEdgeMesh* getMesh() const { return mesh; }
    void setExplosionFactor(float factor) { explosionFactor = factor; }
    float getExplosionFactor() const { return explosionFactor; }

private:
    ParametricMapping* mapping;
    std::vector<ColoredUVPoint> points;
    std::vector<DelaunayTriangle> triangles;
    
    float extrusionDepth;
    bool extrusionEnabled;
    bool wireframeEnabled;  
    bool explosionEnabled;
    float explosionFactor;
    
    HalfEdgeMesh* mesh;  
};