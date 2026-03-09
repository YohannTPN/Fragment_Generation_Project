#pragma once
#include "Curve.h" 
#include <armadillo>
#include <GL/glut.h>
using namespace arma;

class BSplineCurve : public Curve { 
public:
    BSplineCurve(const mat& points, int degree = 3); // points de contrôle nx2
    
    void draw(float t = 0.0f) const override;
    void setT(float value) override { t = value; }
    float getT() const override { return t; }

    // Méthode héritée de Curve
    vec evalPoint(double u) const override; 
    vec evalDerivative(double u) const override ;

    const mat& getControlPoints() const { return controlPoints; }
    int getDegree() const { return k; }

private:
    double basis(int i, int k, double u) const; // fonction de base Cox-de Boor

private:
    mat controlPoints;   // nx2
    int k;               // degré de la B-spline
    float t = 0.5f;      // paramètre d'animation
};
