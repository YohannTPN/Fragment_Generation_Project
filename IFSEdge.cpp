#include "IFSEdge.h"
#include <iostream>
#include <iomanip>

IFSEdge::IFSEdge(const Vec2f& start, const Vec2f& end)
    : P0(start), P4(end)
{
    P1.u = P0.u + (P4.u - P0.u) / 4.0f;
    P1.v = P0.v + (P4.v - P0.v) / 4.0f;

    P2.u = P0.u + (P4.u - P0.u) / 2.0f;
    P2.v = P0.v + (P4.v - P0.v) / 2.0f;

    P3.u = P0.u + 3.0f * (P4.u - P0.u) / 4.0f;
    P3.v = P0.v + 3.0f * (P4.v - P0.v) / 4.0f;
}

void IFSEdge::setTransforms(const IFSTransform& T0, const IFSTransform& T1) {
    T_left  = T0;
    T_right = T1;
}

Vec2f IFSEdge::barycentricToPoint(const float bary[5], const ControlPoints& ctrl) const {
    Vec2f result;
    result.u = bary[0]*ctrl.p[0].u + bary[1]*ctrl.p[1].u +
               bary[2]*ctrl.p[2].u + bary[3]*ctrl.p[3].u + bary[4]*ctrl.p[4].u;
    result.v = bary[0]*ctrl.p[0].v + bary[1]*ctrl.p[1].v +
               bary[2]*ctrl.p[2].v + bary[3]*ctrl.p[3].v + bary[4]*ctrl.p[4].v;
    return result;
}

IFSEdge::ControlPoints IFSEdge::applyTransform(const IFSTransform& T,
                                                const ControlPoints& ctrl) const {
    ControlPoints result;
    for (int i = 0; i < 5; ++i) {
        float bary[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
        bary[i] = 1.0f;
        float newBary[5];
        T.apply(bary, newBary);
        result.p[i] = barycentricToPoint(newBary, ctrl);
    }
    return result;
}

// ============================================================
// Algorithme IFS correct :
//
// À chaque niveau de récursion, on subdivise le segment courant
// en DEUX sous-segments indépendants :
//   - le sous-segment gauche  = T_left  appliqué aux 5 pts courants
//   - le sous-segment droit   = T_right appliqué aux 5 pts courants
//
// Chaque sous-segment est ensuite traité récursivement.
//
// Le bug de l'ancienne implémentation (subdivide) était :
// elle appliquait UNE transformation, puis récursait dans LES DEUX,
// ce qui causait un re-tracé complet du même sous-segment → courbe
// auto-intersectante → triangulation impossible.
// ============================================================
void IFSEdge::generateRecursive(const ControlPoints& ctrl, int depth,
                                 std::vector<Vec2f>& result) const {
    if (depth <= 0) {
        // Cas de base : retourner les 5 points de contrôle dans l'ordre
        for (int i = 0; i < 5; ++i)
            result.push_back(ctrl.p[i]);
        return;
    }

    // Sous-segment gauche : T_left appliqué
    // T_left garantit : P0' = P0 original, P4' = P2 original (mi-segment)
    ControlPoints leftCtrl  = applyTransform(T_left,  ctrl);

    // Sous-segment droit : T_right appliqué
    // T_right garantit : P0' = P2 original (mi-segment), P4' = P4 original
    ControlPoints rightCtrl = applyTransform(T_right, ctrl);

    std::vector<Vec2f> leftPts, rightPts;
    generateRecursive(leftCtrl,  depth - 1, leftPts);
    generateRecursive(rightCtrl, depth - 1, rightPts);

    // Concaténer : le dernier point de leftPts == le premier de rightPts
    // (tous deux = P2 dans le référentiel courant), donc on saute le premier
    // point de rightPts pour éviter le doublon.
    result.insert(result.end(), leftPts.begin(), leftPts.end());
    if (!rightPts.empty())
        result.insert(result.end(), rightPts.begin() + 1, rightPts.end());
}

std::vector<Vec2f> IFSEdge::generate(int depth) {
    ControlPoints ctrl;
    ctrl.p[0] = P0; ctrl.p[1] = P1; ctrl.p[2] = P2;
    ctrl.p[3] = P3; ctrl.p[4] = P4;

    std::vector<Vec2f> result;
    generateRecursive(ctrl, depth, result);

    // Forcer les extrémités exactes (éliminer la dérive en virgule flottante)
    if (!result.empty()) {
        result.front() = P0;
        result.back()  = P4;
    }

    return result;
}

void IFSEdge::print() const {
    std::cout << "IFSEdge (5 points):\n";
    std::cout << "  P0: (" << P0.u << ", " << P0.v << ")\n";
    std::cout << "  P1: (" << P1.u << ", " << P1.v << ")\n";
    std::cout << "  P2: (" << P2.u << ", " << P2.v << ")\n";
    std::cout << "  P3: (" << P3.u << ", " << P3.v << ")\n";
    std::cout << "  P4: (" << P4.u << ", " << P4.v << ")\n";
}