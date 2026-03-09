#pragma once
#include "IFSTransform.h"
#include <vector>
#include <utility>

struct Vec2f {
    float u, v;
    Vec2f(float u_ = 0.0f, float v_ = 0.0f) : u(u_), v(v_) {}
};

// Classe pour une arête IFS avec 5 points de contrôle
class IFSEdge {
public:
    // Constructeur: crée une arête entre start et end
    // Les points P1, P2, P3 sont placés à 1/4, 1/2, 3/4
    IFSEdge(const Vec2f& start, const Vec2f& end);
    
    // Définir les 2 transformations
    void setTransforms(
        const IFSTransform& T0,   // Pour segment gauche [P0 → P2]
        const IFSTransform& T1    // Pour segment droit [P2 → P4]
    );
    
    // Définir manuellement les points de contrôle intermédiaires
    void setControlPoints(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3) {
        P1 = p1;
        P2 = p2;
        P3 = p3;
    }
    
    // Génère les points de l'arête après n itérations IFS
    // Retourne une liste de points 2D à relier en segments
    std::vector<Vec2f> generate(int depth);
    
    // Accesseurs pour les points de contrôle
    const Vec2f& getP0() const { return P0; }
    const Vec2f& getP1() const { return P1; }
    const Vec2f& getP2() const { return P2; }
    const Vec2f& getP3() const { return P3; }
    const Vec2f& getP4() const { return P4; }
    
    void print() const;
    
private:
    Vec2f P0, P1, P2, P3, P4;  // 5 points de contrôle (en coordonnées UV)
    
    IFSTransform T_left;   // T0 - transformation pour segment gauche [P0 → P2]
    IFSTransform T_right;  // T1 - transformation pour segment droit [P2 → P4]
    
    // Structure interne pour la récursion
    struct ControlPoints {
        Vec2f p[5];  // Les 5 points de contrôle
    };
    
    // Fonction récursive de subdivision
    void subdivide(
        const ControlPoints& ctrl,
        bool useLeftTransform,  // true = T_left, false = T_right
        int depth,
        std::vector<Vec2f>& result
    );
    
    // Convertit des coordonnées barycentriques en point 2D
    Vec2f barycentricToPoint(const float bary[5], const ControlPoints& ctrl) const;
};