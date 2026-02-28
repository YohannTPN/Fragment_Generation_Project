#pragma once
#include "IFSEdge.h"
#include "IFSTransform.h"
#include <vector>
#include <memory>

class IFSManager {
public:
    IFSManager();
    
    // Définir les transformations globales (mêmes pour toutes les arêtes)
    void setGlobalTransforms(
        const IFSTransform& Ti,
        const IFSTransform& Tij,
        const IFSTransform& Tj
    );
    
    // Ajouter une arête à traiter
    void addEdge(const Vec2f& start, const Vec2f& end);
    
    // Ajouter une arête avec points de contrôle personnalisés
    void addEdgeWithControlPoints(const Vec2f& p0, const Vec2f& p1, 
                                   const Vec2f& p2, const Vec2f& p3);
    
    // Générer toutes les arêtes IFS avec la profondeur donnée
    void generate(int depth);
    
    // Dessiner toutes les arêtes IFS dans l'espace UV
    void drawUV() const;
    
    // Obtenir tous les points générés (pour export ou passage en 3D)
    const std::vector<std::vector<Vec2f>>& getGeneratedEdges() const {
        return generatedEdges;
    }
    
    // Effacer toutes les arêtes
    void clear();
    
    // Accesseurs
    int getEdgeCount() const { return edges.size(); }
    int getDepth() const { return currentDepth; }
    
    // Transformations globales (public pour UVTriangleIFS)
    IFSTransform globalTi;
    IFSTransform globalTij;
    IFSTransform globalTj;

private:
    std::vector<std::unique_ptr<IFSEdge>> edges;
    std::vector<std::vector<Vec2f>> generatedEdges;  // Points générés pour chaque arête
    
    int currentDepth;
    bool transformsSet;
};
