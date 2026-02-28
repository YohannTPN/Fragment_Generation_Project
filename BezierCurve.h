#pragma once
#include "Curve.h" 
#include <armadillo>
#include <GL/glut.h>

using namespace arma;

class BezierCurve : public Curve { 
public:
    BezierCurve(const mat& points); 

    void draw(float t = 0.0f) const override;
    void setT(float value) override { t = value; }
    float getT() const override { return t; }
    
    vec evalPoint(double u) const override; 
    vec evalDerivative(double u) const override ;

    const mat& getControlPoints() const { return controlPoints; }
    const mat& getM() const { return M; }

private:
    mat controlPoints;  
    mat M;              
    float t = 0.5f; 
};
