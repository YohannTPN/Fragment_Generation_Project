#pragma once
#include "Renderable.h"
#include "Curve.h" 

// L'interface RevolutionSurface hérite de Renderable.
class RevolutionSurface : public Renderable {
public:
    virtual ~RevolutionSurface() = default;
    

    virtual Curve* getCurve() = 0; 

    virtual const Curve* getCurve() const = 0;

    // draw, setT, getT sont hérités de Renderable.
};
