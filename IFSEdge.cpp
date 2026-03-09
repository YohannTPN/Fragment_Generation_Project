#include "IFSEdge.h"
#include <iostream>
#include <iomanip>

IFSEdge::IFSEdge(const Vec2f& start, const Vec2f& end) 
    : P0(start), P4(end) 
{
    // Initialiser P1, P2, P3 à 1/4, 1/2, 3/4 de l'arête
    P1.u = P0.u + (P4.u - P0.u) / 4.0f;
    P1.v = P0.v + (P4.v - P0.v) / 4.0f;
    
    P2.u = P0.u + (P4.u - P0.u) / 2.0f;
    P2.v = P0.v + (P4.v - P0.v) / 2.0f;
    
    P3.u = P0.u + 3.0f * (P4.u - P0.u) / 4.0f;
    P3.v = P0.v + 3.0f * (P4.v - P0.v) / 4.0f;
}

void IFSEdge::setTransforms(
    const IFSTransform& T0,
    const IFSTransform& T1
) {
    T_left = T0;
    T_right = T1;
    
    // === DIAGNOSTIC : Afficher les matrices ===
    static bool firstTime = true;
    if (firstTime) {
        std::cout << "\n=== DIAGNOSTIC IFSEdge ===\n";
        std::cout << "Matrice T_left (T0) :\n";
        T_left.print();
        std::cout << "\nMatrice T_right (T1) :\n";
        T_right.print();
        std::cout << "=== FIN DIAGNOSTIC ===\n\n";
        firstTime = false;
    }
}

Vec2f IFSEdge::barycentricToPoint(const float bary[5], const ControlPoints& ctrl) const {
    Vec2f result;
    result.u = bary[0] * ctrl.p[0].u + bary[1] * ctrl.p[1].u + 
               bary[2] * ctrl.p[2].u + bary[3] * ctrl.p[3].u + bary[4] * ctrl.p[4].u;
    result.v = bary[0] * ctrl.p[0].v + bary[1] * ctrl.p[1].v + 
               bary[2] * ctrl.p[2].v + bary[3] * ctrl.p[3].v + bary[4] * ctrl.p[4].v;
    return result;
}

void IFSEdge::subdivide(
    const ControlPoints& ctrl,
    bool useLeftTransform,
    int depth,
    std::vector<Vec2f>& result
) {
    if (depth <= 0) {
        // Cas de base: retourner les 5 points tels quels
        result.push_back(ctrl.p[0]);
        result.push_back(ctrl.p[1]);
        result.push_back(ctrl.p[2]);
        result.push_back(ctrl.p[3]);
        result.push_back(ctrl.p[4]);
        return;
    }
    
    // Choisir la transformation selon le segment
    const IFSTransform& transform = useLeftTransform ? T_left : T_right;
    
    // === DIAGNOSTIC : Afficher les points avant transformation ===
    static int callCount = 0;
    if (callCount == 0) {
        std::cout << "\n=== DIAGNOSTIC subdivide (premier appel) ===\n";
        std::cout << "Utilise " << (useLeftTransform ? "T_left" : "T_right") << "\n";
        std::cout << "Points de contrôle avant transformation:\n";
        for (int i = 0; i < 5; ++i) {
            std::cout << "  P" << i << ": (" << ctrl.p[i].u << ", " << ctrl.p[i].v << ")\n";
        }
    }
    
    // Appliquer la transformation pour obtenir les nouveaux points de contrôle
    ControlPoints newCtrl;
    
    for (int i = 0; i < 5; ++i) {
        float bary[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        bary[i] = 1.0f;
        
        float transformedBary[5];
        transform.apply(bary, transformedBary);
        
        newCtrl.p[i] = barycentricToPoint(transformedBary, ctrl);
    }
    
    // === DIAGNOSTIC : Afficher les points après transformation ===
    if (callCount == 0) {
        std::cout << "Points de contrôle après transformation:\n";
        for (int i = 0; i < 5; ++i) {
            std::cout << "  P" << i << "': (" << newCtrl.p[i].u << ", " << newCtrl.p[i].v << ")\n";
        }
        std::cout << "=== FIN DIAGNOSTIC ===\n\n";
        callCount++;
    }
    
    // === RÉCURSION DOUBLE ===
    std::vector<Vec2f> leftPart;
    subdivide(newCtrl, true, depth - 1, leftPart);
    
    std::vector<Vec2f> rightPart;
    subdivide(newCtrl, false, depth - 1, rightPart);
    
    // Combiner gauche et droite
    result.insert(result.end(), leftPart.begin(), leftPart.end());
    if (!rightPart.empty() && !leftPart.empty()) {
        result.insert(result.end(), rightPart.begin() + 1, rightPart.end());
    }
}

std::vector<Vec2f> IFSEdge::generate(int depth) {
    std::vector<Vec2f> result;
    
    if (depth <= 0) {
        result.push_back(P0);
        result.push_back(P1);
        result.push_back(P2);
        result.push_back(P3);
        result.push_back(P4);
        return result;
    }
    
    ControlPoints ctrl;
    ctrl.p[0] = P0;
    ctrl.p[1] = P1;
    ctrl.p[2] = P2;
    ctrl.p[3] = P3;
    ctrl.p[4] = P4;
    
    std::vector<Vec2f> segLeft;
    subdivide(ctrl, true, depth, segLeft);
    
    std::vector<Vec2f> segRight;
    subdivide(ctrl, false, depth, segRight);
    
    if (!segLeft.empty()) {
        segLeft.front() = P0;
        segLeft.back() = P2;
    }
    
    if (!segRight.empty()) {
        segRight.front() = P2;
        segRight.back() = P4;
    }
    
    result.insert(result.end(), segLeft.begin(), segLeft.end());
    
    if (!segRight.empty() && !segLeft.empty()) {
        result.insert(result.end(), segRight.begin() + 1, segRight.end());
    }
    
    return result;
}

void IFSEdge::print() const {
    std::cout << "IFSEdge (5 points):" << std::endl;
    std::cout << "  P0: (" << P0.u << ", " << P0.v << ")" << std::endl;
    std::cout << "  P1: (" << P1.u << ", " << P1.v << ")" << std::endl;
    std::cout << "  P2: (" << P2.u << ", " << P2.v << ")" << std::endl;
    std::cout << "  P3: (" << P3.u << ", " << P3.v << ")" << std::endl;
    std::cout << "  P4: (" << P4.u << ", " << P4.v << ")" << std::endl;
}