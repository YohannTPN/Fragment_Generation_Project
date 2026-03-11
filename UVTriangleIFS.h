#pragma once
#include "IFSManager.h"
#include "UVTriangleManager.h"
#include "HalfEdge.h"
#include <set>
#include <map>
#include <vector>

// Structure pour représenter un triangle avec arêtes IFS
struct IFSTriangle {
    std::vector<Vec2f> boundary;  // Points du périmètre (arêtes IFS)
    float r, g, b;                // Couleur du triangle

    // Centroïde des 3 sommets Delaunay ORIGINAUX.
    // Toujours strictement à l'intérieur du triangle → jamais biaisé
    // par la forme de la courbe IFS.
    Vec2f originalCentroid;
};

// Classe pour appliquer l'IFS sur les arêtes et retrianguler
class UVTriangleIFS {
public:
    UVTriangleIFS(UVTriangleManager* triManager);
    
    // Définir les 2 transformations IFS globales
    void setTransforms(
        const IFSTransform& T0,  // Segment gauche
        const IFSTransform& T1   // Segment droit
    );
    
    // Générer les triangles IFS complets (avec retriangulation)
    void generateIFSTriangles(int ifsDepth);
    
    // Dessiner dans l'espace UV (wireframe des arêtes IFS)
    void drawUVWireframe() const;
    
    // Dessiner dans l'espace UV (triangles remplis + arêtes IFS)
    void drawUVFilled() const;
    
    // Construire un nouveau maillage 3D avec les triangles IFS
    void buildIFSMesh(HalfEdgeMesh* mesh, ParametricMapping* mapping);

    void draw3D(bool wireframe, bool explosion, float explosionFactor) const;
    
    // Accesseurs
    int getIFSDepth() const { return currentDepth; }
    int getTriangleCount() const { return ifsTriangles.size(); }
    
private:
    UVTriangleManager* triangleManager;
    IFSManager ifsManager;
    int currentDepth;
    
    std::vector<IFSTriangle> ifsTriangles;
    HalfEdgeMesh* generatedMesh = nullptr;
    
    // Clé pour identifier une arête unique
    struct EdgeKey {
        long u1, v1, u2, v2;  
        
        EdgeKey(float _u1, float _v1, float _u2, float _v2) {
            const long precision = 1000;
            long qu1 = (long)std::round(_u1 * precision);
            long qv1 = (long)std::round(_v1 * precision);
            long qu2 = (long)std::round(_u2 * precision);
            long qv2 = (long)std::round(_v2 * precision);
            
            if (qu1 < qu2 || (qu1 == qu2 && qv1 < qv2)) {
                u1 = qu1; v1 = qv1;
                u2 = qu2; v2 = qv2;
            } else {
                u1 = qu2; v1 = qv2;
                u2 = qu1; v2 = qv1;
            }
        }
        
        bool operator<(const EdgeKey& other) const {
            if (u1 != other.u1) return u1 < other.u1;
            if (v1 != other.v1) return v1 < other.v1;
            if (u2 != other.u2) return u2 < other.u2;
            return v2 < other.v2;
        }
    };

    std::map<EdgeKey, std::vector<Vec2f>> edgeIFSCache;
    std::vector<Vec2f> getOrCreateEdgeIFS(float u1, float v1, float u2, float v2);
};