#pragma once
#include "Renderable.h"
#include "Curve.h" 


class RevolutionSurface : public Renderable {
public:
    virtual ~RevolutionSurface() = default;
    

    virtual Curve* getCurve() = 0; 

    virtual const Curve* getCurve() const = 0;

};
