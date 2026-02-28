#include "OpenGLManager.h"
#include <GL/glu.h>

OpenGLManager::OpenGLManager() {}

void OpenGLManager::init() 
{

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    

    glEnable(GL_NORMALIZE); 
    glShadeModel(GL_SMOOTH); 

    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
    
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);


    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f }; 
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void OpenGLManager::setupProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)1.0f, 0.1f, 10.0f); 
    glMatrixMode(GL_MODELVIEW);
}

void OpenGLManager::setupCamera()
{
    glLoadIdentity();
    gluLookAt(0., 0., 4., 
              0., 0., 0.,  
              0., 1., 0.); 
}
