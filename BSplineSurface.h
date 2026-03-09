#pragma once
#include "RevolutionSurface.h" 
#include "BSplineCurve.h"
#include <vector>
#include <cmath>
#include <GL/glut.h>

class BSplineSurface : public RevolutionSurface { 
public:
    BSplineSurface(BSplineCurve* curve, int slices = 8); 

    // --- Implémentations de Renderable ---
    void draw(float t = 0.0f) const override;
    
    void setT(float value) override { 
        // Fonctionne maintenant car 'curve' n'est plus const
        if (curve) curve->setT(value);
    }
    float getT() const override { return curve ? curve->getT() : 0.0f; }


    BSplineCurve* getCurve() override { return curve; }
    
    const BSplineCurve* getCurve() const override { return curve; }


private:
    BSplineCurve* curve; // courbe verticale
    int slices;                // nombre de segments autour de l’axe Y
};
