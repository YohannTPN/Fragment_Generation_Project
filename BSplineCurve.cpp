#include "BSplineCurve.h"
#include <cmath>
#include <GL/glut.h>

BSplineCurve::BSplineCurve(const mat& points, int degree)
    : controlPoints(points), k(degree) {}


vec BSplineCurve::evalPoint(double u) const {
    vec point = zeros<vec>(2);  
    int n = controlPoints.n_rows;
    

    double u_scaled = u * (n - (k - 1)) + (k - 1);

    for (int i = 0; i < n; ++i) {
        double b = basis(i, k, u_scaled);
        point += b * controlPoints.row(i).t();
    }
    return point;
}

vec BSplineCurve::evalDerivative(double u) const {
    vec derivative = zeros<vec>(2);  
    int n = controlPoints.n_rows;
    
    double u_scaled = u * (n - (k - 1)) + (k - 1);

    for (int i = 0; i < n; ++i) {
        double b_deriv = 0.0;
        // Calcul de la dérivée de la fonction de base B-spline
        if (k > 0) {
            double denom1 = k;
            double denom2 = k;

            double term1 = 0.0;
            if (denom1 != 0)
                term1 = basis(i, k - 1, u_scaled) / denom1;

            double term2 = 0.0;
            if (denom2 != 0)
                term2 = basis(i + 1, k - 1, u_scaled) / denom2;

            b_deriv = term1 - term2;
        }
        derivative += b_deriv * controlPoints.row(i).t();
    }
    // Ajustement pour le changement d'échelle de u
    derivative *= (n - (k - 1));
    return derivative;
}

// Fonction de base B-spline (Cox-de Boor)
double BSplineCurve::basis(int i, int k, double u) const {
    if (k == 0) {
        // Condition de base: B_{i,0}(u) = 1 si u est dans l'intervalle [i, i+1[, 0 sinon.
        return (u >= i && u < i + 1) ? 1.0 : 0.0;
    } else {
        // Premier terme
        // (u - T_i) / (T_i+k - T_i) * B_{i, k-1}(u)
        // Pour les nœuds entiers T_j = j, le dénominateur est k.
        double denom1 = k;
        double term1 = 0.0;
        if (denom1 != 0)
            term1 = (u - i) / denom1 * basis(i, k - 1, u);

        // Deuxième terme
        // (T_i+k+1 - u) / (T_i+k+1 - T_i+1) * B_{i+1, k-1}(u)
        // Pour les nœuds entiers T_j = j, le dénominateur est k.
        double denom2 = k;
        double term2 = 0.0;
        if (denom2 != 0)
            term2 = (i + k + 1 - u) / denom2 * basis(i + 1, k - 1, u);

        return term1 + term2;
    }
}

void BSplineCurve::draw(float t) const {
    if (controlPoints.n_rows == 0) return;

    // --- point courant ---
    vec B = evalPoint(t);
    glPointSize(5.0);
    glBegin(GL_POINTS);
        glColor3f(1.0,0.0,0.0); // Rouge
        glVertex2f(B(0), B(1));
    glEnd();

    // --- courbe complète ---
    glPointSize(3.0);
    glBegin(GL_LINE_STRIP);
        glColor3f(1.0,1.0,1.0); // Blanc
        // u va de 0.0 à 1.0, et evalPoint s'occupe de la mise à l'échelle.
        for(double u = 0.0; u <= 1.0; u += 0.01) {
            vec Bu = evalPoint(u);
            glVertex2f(Bu(0), Bu(1));
        }
    glEnd();

    // --- points de contrôle ---
    glPointSize(5.0);
    glBegin(GL_POINTS);
        glColor3f(0.0,1.0,0.0); // Vert
        for(int i = 0; i < controlPoints.n_rows; ++i)
            glVertex2f(controlPoints(i,0), controlPoints(i,1));
    glEnd();

    // --- lignes de contrôle ---
    glBegin(GL_LINE_STRIP);
        glColor3f(0.5,0.5,0.5); // Gris
        for(int i = 0; i < controlPoints.n_rows; ++i)
            glVertex2f(controlPoints(i,0), controlPoints(i,1));
    glEnd();
}
