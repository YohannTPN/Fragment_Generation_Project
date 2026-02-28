#pragma once
#include "IFSTransform.h"
#include <vector>
#include <utility>

struct Vec2f {
    float u, v;
    Vec2f(float u_ = 0.0f, float v_ = 0.0f) : u(u_), v(v_) {}
};

// Classe pour une arête IFS avec 4 points de contrôle
class IFSEdge {
public:
    // Constructeur: crée une arête entre start et end
    // Les points P1 et P2 sont initialement placés à 1/3 et 2/3
    IFSEdge(const Vec2f& start, const Vec2f& end);
    
    // Définir les 3 transformations
    void setTransforms(
        const IFSTransform& Ti,   // Pour segment [P0, P1]
        const IFSTransform& Tij,  // Pour segment [P1, P2]
        const IFSTransform& Tj    // Pour segment [P2, P3]
    );
    
    // Définir manuellement les points de contrôle intermédiaires
    void setControlPoints(const Vec2f& p1, const Vec2f& p2) {
        P1 = p1;
        P2 = p2;
    }
    
    // Génère les points de l'arête après n itérations IFS
    // Retourne une liste de points 2D à relier en segments
    std::vector<Vec2f> generate(int depth);
    
    // Accesseurs pour les points de contrôle
    const Vec2f& getP0() const { return P0; }
    const Vec2f& getP1() const { return P1; }
    const Vec2f& getP2() const { return P2; }
    const Vec2f& getP3() const { return P3; }
    
    void print() const;
    
private:
    Vec2f P0, P1, P2, P3;  // 4 points de contrôle (en coordonnées UV)
    
    IFSTransform T_start;   // Ti  - transformation pour segment 1
    IFSTransform T_middle;  // Tij - transformation pour segment 2
    IFSTransform T_end;     // Tj  - transformation pour segment 3
    
    // Structure interne pour la récursion
    struct ControlPoints {
        Vec2f p[4];  // Les 4 points de contrôle
    };
    
    // Fonction récursive de subdivision
    void subdivide(
        const ControlPoints& ctrl,
        const IFSTransform& transform,
        int depth,
        std::vector<Vec2f>& result
    );
    
    // Convertit des coordonnées barycentriques en point 2D
    Vec2f barycentricToPoint(const float bary[4], const ControlPoints& ctrl) const;
};