#pragma once
#include "Renderable.h" 

#include <armadillo>

using namespace arma;

// L'interface Curve hérite de Renderable.
// Toutes les courbes (Bezier, BSpline) doivent hériter de Curve.
class Curve : public Renderable {
public:
    virtual ~Curve() = default;
    
    // Méthode purement virtuelle pour évaluer un point 2D (r, y) en fonction de u.
    // Cette méthode est utilisée par RevolutionSurface et ParametricMapping.
    virtual vec evalPoint(double u) const = 0;

    virtual vec evalDerivative(double u) const = 0;

    // Les méthodes draw, setT, getT sont héritées de Renderable et rendues concrètes 
    // dans les classes enfants (BezierCurve, BSplineCurve).
};
