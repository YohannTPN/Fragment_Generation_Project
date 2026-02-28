#pragma once
#include <GL/glut.h>
#include <vector>
#include <utility>

class ParametricSpace {
public:
    ParametricSpace(int resU = 20, int resV = 20);

    void draw() const;                         
    void addTriangle(float u1,float v1, float u2,float v2, float u3,float v3);

private:
    int resU, resV;
    std::vector<std::tuple<float,float,float,float,float,float>> triangles; 
};
