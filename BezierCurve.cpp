#include "BezierCurve.h"
#include <cmath>
#include <GL/glut.h>

BezierCurve::BezierCurve(const mat& points) : controlPoints(points) {
    // matrice de Bézier cubique
    M = { {1.0, 0.0, 0.0, 0.0},
          {-3.0, 3.0, 0.0, 0.0},
          {3.0, -6.0, 3.0, 0.0},
          {-1.0, 3.0, -3.0, 1.0} };
}

vec BezierCurve::evalPoint(double u) const {
    mat Tu = {1.0, u, u*u, u*u*u};  // 1x4
    mat Bu = Tu * M * controlPoints; // 1x2
    return Bu.row(0).t(); 
}

vec BezierCurve::evalDerivative(double u) const {
    mat dTu = {0.0, 1.0, 2.0*u, 3.0*u*u}; // Dérivée de Tu
    mat dBu = dTu * M * controlPoints; 
    return dBu.row(0).t();
}

void BezierCurve::draw(float t) const {

    vec B = evalPoint(t);
    glPointSize(5.0);
    glBegin(GL_POINTS);
        glColor3f(1.0,0.0,0.0);
        glVertex2f(B(0), B(1));
    glEnd();

    // --- courbe complète ---
    glPointSize(3.0);
    glBegin(GL_LINE_STRIP);
        glColor3f(1.0,1.0,1.0);
        for(float u=0; u<=1.0f; u+=0.01f){
            vec Bu = evalPoint(u);
            glVertex2f(Bu(0), Bu(1));
        }
    glEnd();

    // --- points de contrôle ---
    glPointSize(5.0);
    glBegin(GL_POINTS);
        glColor3f(0.0,1.0,0.0);
        for(int i=0; i<4; i++)
            glVertex2f(controlPoints(i,0), controlPoints(i,1));
    glEnd();

    // --- lignes de contrôle ---
    glBegin(GL_LINE_STRIP);
        glColor3f(0.5,0.5,0.5);
        for(int i=0; i<4; i++)
            glVertex2f(controlPoints(i,0), controlPoints(i,1));
    glEnd();
}
