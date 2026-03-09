#pragma once
#include "RevolutionSurface.h" 
#include "BezierCurve.h"
#include <vector>
#include <cmath>
#include <GL/glut.h>

class BezierSurface : public RevolutionSurface { 
public:
    BezierSurface(BezierCurve* curve, int slices = 8); 

    void draw(float t = 0.0f) const override;
    
    void setT(float value) override { 
        if (curve) curve->setT(value);
    }
    float getT() const override { return curve ? curve->getT() : 0.0f; }

    // --- Implémentation obligatoire de RevolutionSurface ---
    BezierCurve* getCurve() override { return curve; } 
    
    const BezierCurve* getCurve() const override { return curve; }

private:
    BezierCurve* curve; // courbe verticale
    int slices;               // nombre de segments autour de l’axe Y
};
