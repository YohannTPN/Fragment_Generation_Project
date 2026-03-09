#pragma once
#include "RevolutionSurface.h" 
#include <cmath>

// Struct pour représenter un point 3D
struct Vec3f {
    float x, y, z;
    Vec3f(float X=0.0f, float Y=0.0f, float Z=0.0f) : x(X), y(Y), z(Z) {}

    Vec3f operator-(const Vec3f& other) const {
    return {x - other.x, y - other.y, z - other.z};
}
Vec3f operator+(const Vec3f& other) const {
    return {x + other.x, y + other.y, z + other.z};
}
};



// Classe pour faire le mapping paramétrique (u,v) -> espace 3D
class ParametricMapping {
public:
    ParametricMapping(const RevolutionSurface* s) : surface(s) {}

    Vec3f map(float u, float v) const;

private:
    const RevolutionSurface* surface; 
};
