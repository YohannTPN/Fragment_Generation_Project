#include "IFSEdge.h"
#include <iostream>
#include <iomanip>

IFSEdge::IFSEdge(const Vec2f& start, const Vec2f& end) 
    : P0(start), P3(end) 
{
    // Initialiser P1 et P2 à 1/3 et 2/3 de l'arête
    P1.u = P0.u + (P3.u - P0.u) / 3.0f;
    P1.v = P0.v + (P3.v - P0.v) / 3.0f;
    
    P2.u = P0.u + 2.0f * (P3.u - P0.u) / 3.0f;
    P2.v = P0.v + 2.0f * (P3.v - P0.v) / 3.0f;
}

void IFSEdge::setTransforms(
    const IFSTransform& Ti,
    const IFSTransform& Tij,
    const IFSTransform& Tj
) {
    T_start = Ti;
    T_middle = Tij;
    T_end = Tj;
}

Vec2f IFSEdge::barycentricToPoint(const float bary[4], const ControlPoints& ctrl) const {
    Vec2f result;
    result.u = bary[0] * ctrl.p[0].u + bary[1] * ctrl.p[1].u + 
               bary[2] * ctrl.p[2].u + bary[3] * ctrl.p[3].u;
    result.v = bary[0] * ctrl.p[0].v + bary[1] * ctrl.p[1].v + 
               bary[2] * ctrl.p[2].v + bary[3] * ctrl.p[3].v;
    return result;
}

void IFSEdge::subdivide(
    const ControlPoints& ctrl,
    const IFSTransform& transform,
    int depth,
    std::vector<Vec2f>& result
) {
    if (depth <= 0) {
        // Cas de base: dessiner la ligne brisée entre les 4 points de contrôle
        result.push_back(ctrl.p[0]);
        result.push_back(ctrl.p[1]);
        result.push_back(ctrl.p[2]);
        result.push_back(ctrl.p[3]);
        return;
    }
    
    // Appliquer la transformation pour obtenir les nouveaux points de contrôle
    ControlPoints newCtrl;
    
    for (int i = 0; i < 4; ++i) {
        // Pour chaque point de contrôle Pi, calculer Pi'
        // Pi est représenté par le vecteur barycentrique ei = (0,0,...,1,...,0)
        float bary[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        bary[i] = 1.0f;
        
        float transformedBary[4];
        transform.apply(bary, transformedBary);
        
        // Convertir les coordonnées barycentriques en point 2D
        newCtrl.p[i] = barycentricToPoint(transformedBary, ctrl);
    }
    
    // Récursivement subdiviser avec la MÊME transformation
    subdivide(newCtrl, transform, depth - 1, result);
}

std::vector<Vec2f> IFSEdge::generate(int depth) {
    std::vector<Vec2f> result;
    
    if (depth <= 0) {
        result.push_back(P0);
        result.push_back(P1);
        result.push_back(P2);
        result.push_back(P3);
        return result;
    }
    
    ControlPoints ctrl;
    ctrl.p[0] = P0;
    ctrl.p[1] = P1;
    ctrl.p[2] = P2;
    ctrl.p[3] = P3;
    
    // Segment 1 : de P0 à P1
    ControlPoints seg1Ctrl;
    seg1Ctrl.p[0] = P0;
    seg1Ctrl.p[1] = Vec2f(P0.u + (P1.u - P0.u) / 3.0f, P0.v + (P1.v - P0.v) / 3.0f);
    seg1Ctrl.p[2] = Vec2f(P0.u + 2.0f * (P1.u - P0.u) / 3.0f, P0.v + 2.0f * (P1.v - P0.v) / 3.0f);
    seg1Ctrl.p[3] = P1;
    
    // Segment 2 : de P1 à P2
    ControlPoints seg2Ctrl;
    seg2Ctrl.p[0] = P1;
    seg2Ctrl.p[1] = Vec2f(P1.u + (P2.u - P1.u) / 3.0f, P1.v + (P2.v - P1.v) / 3.0f);
    seg2Ctrl.p[2] = Vec2f(P1.u + 2.0f * (P2.u - P1.u) / 3.0f, P1.v + 2.0f * (P2.v - P1.v) / 3.0f);
    seg2Ctrl.p[3] = P2;
    
    // Segment 3 : de P2 à P3
    ControlPoints seg3Ctrl;
    seg3Ctrl.p[0] = P2;
    seg3Ctrl.p[1] = Vec2f(P2.u + (P3.u - P2.u) / 3.0f, P2.v + (P3.v - P2.v) / 3.0f);
    seg3Ctrl.p[2] = Vec2f(P2.u + 2.0f * (P3.u - P2.u) / 3.0f, P2.v + 2.0f * (P3.v - P2.v) / 3.0f);
    seg3Ctrl.p[3] = P3;
    
    // Appliquer les transformations
    std::vector<Vec2f> seg1, seg2, seg3;
    subdivide(seg1Ctrl, T_start, depth, seg1);
    subdivide(seg2Ctrl, T_middle, depth, seg2);
    subdivide(seg3Ctrl, T_end, depth, seg3);
    
    if (!seg1.empty() && !seg2.empty()) {
        seg1.back() = P1;
        seg2.front() = P1;
    }
    
    if (!seg2.empty() && !seg3.empty()) {
        seg2.back() = P2;
        seg3.front() = P2;
    }
    
    if (!seg1.empty()) {
        seg1.front() = P0;
    }
    if (!seg3.empty()) {
        seg3.back() = P3;
    }
    
    result.insert(result.end(), seg1.begin(), seg1.end());
    
    if (!seg2.empty() && !seg1.empty()) {
        result.insert(result.end(), seg2.begin() + 1, seg2.end());
    }
    
    if (!seg3.empty() && !result.empty()) {
        result.insert(result.end(), seg3.begin() + 1, seg3.end());
    }
    
    return result;
}

void IFSEdge::print() const {
    std::cout << "IFSEdge:" << std::endl;
    std::cout << "  P0: (" << P0.u << ", " << P0.v << ")" << std::endl;
    std::cout << "  P1: (" << P1.u << ", " << P1.v << ")" << std::endl;
    std::cout << "  P2: (" << P2.u << ", " << P2.v << ")" << std::endl;
    std::cout << "  P3: (" << P3.u << ", " << P3.v << ")" << std::endl;
}