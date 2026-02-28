#pragma once
#include <GL/glut.h>

class OpenGLManager {
public:
    OpenGLManager();
    void init();               // Initialise les paramètres OpenGL (lumière, profondeur, etc.)
    void setupProjection();    // Configure la matrice de projection
    void setupCamera();        // Positionne la caméra
};
