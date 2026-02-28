#pragma once
#include <GL/glut.h>
#include "Renderable.h"

class Scene {
public:
    Scene();

    void setRenderable(Renderable* obj) { object = obj; }
    void display(float t) const;

    void drawAxes() const;

    float cameraDistance;
    float cameraAngleX;
    float cameraAngleY;

private:
    Renderable* object; 
};
