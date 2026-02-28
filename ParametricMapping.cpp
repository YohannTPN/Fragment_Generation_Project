#include "ParametricMapping.h" 
#include "RevolutionSurface.h" 
#include <armadillo>
#include <cmath>

using namespace arma;
using namespace std;

Vec3f ParametricMapping::map(float u, float v) const {
    if (!surface) return Vec3f();


    const Curve* curve = surface->getCurve();
    if (!curve) return Vec3f();

    arma::vec Bu = curve->evalPoint(u); // 2D → [r, y]

    float r = Bu(0); // rayon
    float y = Bu(1); // hauteur

    // v ∈ [0,1] correspond à l'angle θ ∈ [0,2π] pour la révolution
    float angle = 2.0f * M_PI * v;
    float x = r * cos(angle);
    float z = r * sin(angle);

    return Vec3f(x, y, z);
}
