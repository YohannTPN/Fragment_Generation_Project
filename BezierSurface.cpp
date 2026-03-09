#include "BezierSurface.h"
#include <armadillo>
#include <cmath>
#include <GL/glut.h>
using namespace arma;

BezierSurface::BezierSurface( BezierCurve* c, int s)
    : curve(c), slices(s) {}

void BezierSurface::draw(float t) const {
    if (!curve) return;
    
    // Transmet le temps à la courbe
    curve->setT(t);

    int subdivisions = 10; // nombre de segments le long de la courbe
    float du = 1.0f / subdivisions;

    glColor3f(0.7f, 0.7f, 1.0f);

    for (int i = 0; i < slices; ++i) {
        float angle1 = 2.0f * M_PI * i / slices;
        float angle2 = 2.0f * M_PI * (i + 1) / slices;
        float c1 = cos(angle1), s1 = sin(angle1);
        float c2 = cos(angle2), s2 = sin(angle2);

        glBegin(GL_TRIANGLE_STRIP);
        for (float u = 0.0f; u <= 1.0f + du; u += du) {
            vec Bu = curve->evalPoint(u); 
            float r = Bu(0); // rayon
            float y = Bu(1); // hauteur

            // 1er point (courbe à angle1)
            glVertex3f(r * c1, y, r * s1);

            // 2e point (courbe à angle2)
            glVertex3f(r * c2, y, r * s2);
        }
        glEnd();
    }

    // --- disque à la base ---
    float u = 0.0f;
    vec Bu = curve->evalPoint(u); 
    float r = Bu(0);
    float y = Bu(1);

    glColor3f(0.6f, 0.6f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
        // centre du disque
        glVertex3f(0.0f, y, 0.0f);
        // contour du disque
        int n = 40; // nombre de segments pour arrondir
        for (int i = 0; i <= n; ++i) {
            float angle = 2.0f * M_PI * i / n;
            float cA = cos(angle);
            float sA = sin(angle);
            glVertex3f(r * cA, y, r * sA);
        }
        glEnd();

    curve->draw(t);
}
