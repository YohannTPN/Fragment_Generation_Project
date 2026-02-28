#include "BSplineSurface.h"
#include <GL/glut.h>
#include <armadillo>
#include <cmath>
using namespace arma;

BSplineSurface::BSplineSurface( BSplineCurve* c, int s)
    : curve(c), slices(s) {}

void BSplineSurface::draw(float t) const {
    if (!curve) return;
    curve->setT(t);

    int subdivisions = 40; 
    float du = 1.0f / subdivisions;

    glColor3f(0.7f, 0.7f, 1.0f);

    for (int i = 0; i < slices; ++i) {
        float angle1 = 2.0f * M_PI * i / slices;
        float angle2 = 2.0f * M_PI * (i + 1) / slices;
        float c1 = cos(angle1), s1 = sin(angle1);
        float c2 = cos(angle2), s2 = sin(angle2);

        glBegin(GL_TRIANGLE_STRIP);
        for (float u = 0.0f; u <= 1.0f + du; u += du) {
            arma::vec Bu = curve->evalPoint(u);
            float r = Bu(0);
            float y = Bu(1);

            glVertex3f(r * c1, y, r * s1);

            glVertex3f(r * c2, y, r * s2);
        }
        glEnd();
    }

    // --- disque à la base ---
    float u = 0.0f;
    arma::vec Bu = curve->evalPoint(u);
    float r = Bu(0);
    float y = Bu(1);

    glColor3f(0.6f, 0.6f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, y, 0.0f); 
        int n = 40;
        for (int i = 0; i <= n; ++i) {
            float angle = 2.0f * M_PI * i / n;
            float cA = cos(angle);
            float sA = sin(angle);
            glVertex3f(r * cA, y, r * sA);
        }
    glEnd();

    curve->draw(t);
}
