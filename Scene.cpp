#include "Scene.h"
#include "InputManager.h"
#include <GL/glut.h>
#include <GL/glu.h>

Scene::Scene() : object(nullptr), cameraDistance(-3.0f), cameraAngleX(20.0f), cameraAngleY(30.0f) {}

void Scene::drawAxes() const
{
    glBegin(GL_LINES);
    // Axe X rouge
    glColor3f(1.0,0.0,0.0);
    glVertex3f(0.,0.,0.);
    glVertex3f(1.,0.,0.);
    // Axe Y vert
    glColor3f(0.0,1.0,0.0);
    glVertex3f(0.,0.,0.);
    glVertex3f(0.,1.,0.);
    // Axe Z bleu
    glColor3f(0.0,0.0,1.0);
    glVertex3f(0.,0.,0.);
    glVertex3f(0.,0.,1.);
    glEnd();
}

void Scene::display(float t) const{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
        glTranslatef(0,0,cameraDistance);
        glRotatef(cameraAngleX,1,0,0);
        glRotatef(cameraAngleY,0,1,0);

        drawAxes();

        
    

    
}
