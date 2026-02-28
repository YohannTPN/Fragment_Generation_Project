#pragma once
#include "Renderable.h" 

#include <armadillo>

using namespace arma;

class Curve : public Renderable {
public:
    virtual ~Curve() = default;
    

    virtual vec evalPoint(double u) const = 0;

    virtual vec evalDerivative(double u) const = 0;


};
