#pragma once
#include "Curve.h" 
#include <armadillo>
#include <GL/glut.h>
using namespace arma;

class BSplineCurve : public Curve { 
public:
    BSplineCurve(const mat& points, int degree = 3); 
    
    void draw(float t = 0.0f) const override;
    void setT(float value) override { t = value; }
    float getT() const override { return t; }

    
    vec evalPoint(double u) const override; 
    vec evalDerivative(double u) const override ;

    const mat& getControlPoints() const { return controlPoints; }
    int getDegree() const { return k; }

private:
    double basis(int i, int k, double u) const; 

private:
    mat controlPoints;  
    int k;               
    float t = 0.5f;      
};
