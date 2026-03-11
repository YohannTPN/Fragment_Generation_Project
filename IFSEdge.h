#pragma once
#include "IFSTransform.h"
#include <vector>
#include <utility>

struct Vec2f {
    float u, v;
    Vec2f(float u_ = 0.0f, float v_ = 0.0f) : u(u_), v(v_) {}
};

class IFSEdge {
public:
    IFSEdge(const Vec2f& start, const Vec2f& end);
    
    void setTransforms(
        const IFSTransform& T0,
        const IFSTransform& T1
    );
    
    void setControlPoints(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3) {
        P1 = p1; P2 = p2; P3 = p3;
    }
    
    std::vector<Vec2f> generate(int depth);
    
    const Vec2f& getP0() const { return P0; }
    const Vec2f& getP1() const { return P1; }
    const Vec2f& getP2() const { return P2; }
    const Vec2f& getP3() const { return P3; }
    const Vec2f& getP4() const { return P4; }
    
    void print() const;
    
private:
    Vec2f P0, P1, P2, P3, P4;
    
    IFSTransform T_left;
    IFSTransform T_right;
    
    struct ControlPoints {
        Vec2f p[5];
    };
    
    // Conversion coordonnées barycentriques → point 2D
    Vec2f barycentricToPoint(const float bary[5], const ControlPoints& ctrl) const;
    
    // Applique une transformation IFS aux 5 points de contrôle
    ControlPoints applyTransform(const IFSTransform& T, const ControlPoints& ctrl) const;

    // Génération récursive CORRECTE :
    // à chaque niveau, T_left → sous-segment gauche, T_right → sous-segment droit
    void generateRecursive(const ControlPoints& ctrl, int depth, std::vector<Vec2f>& result) const;
};