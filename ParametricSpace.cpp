#include "ParametricSpace.h"

ParametricSpace::ParametricSpace(int rU, int rV) : resU(rU), resV(rV) {}

void ParametricSpace::draw() const {
    // grille
    glColor3f(0.5f,0.5f,0.5f);
    glBegin(GL_LINES);
    for(int i=0;i<=resU;i++){
        float u = float(i)/resU;
        glVertex3f(u,0,0); glVertex3f(u,1,0);
    }
    for(int j=0;j<=resV;j++){
        float v = float(j)/resV;
        glVertex3f(0,v,0); glVertex3f(1,v,0);
    }
    glEnd();

    // triangles
    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLES);
    for(auto& tri : triangles){
        glVertex3f(std::get<0>(tri), std::get<1>(tri),0);
        glVertex3f(std::get<2>(tri), std::get<3>(tri),0);
        glVertex3f(std::get<4>(tri), std::get<5>(tri),0);
    }
    glEnd();
}

void ParametricSpace::addTriangle(float u1,float v1, float u2,float v2, float u3,float v3){
    triangles.push_back({u1,v1,u2,v2,u3,v3});
}
