#include "OpenGLManager.h"
#include <GL/glu.h>

OpenGLManager::OpenGLManager() {}

void OpenGLManager::init() 
{
    // 1. Fond gris moyen (plus clair qu'avant pour moins de contraste dur)
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // IMPORTANT : Recalcule les normales pour qu'elles aient une taille de 1
    // Sinon l'éclairage devient incohérent si on zoom/scale
    glEnable(GL_NORMALIZE); 
    glShadeModel(GL_SMOOTH); 

    // --- 2. LUMIÈRE AMBIANTE GLOBALE (La solution à "C'est trop sombre") ---
    // Cela ajoute une lumière de base partout, indépendamment des lampes.
    // {0.5, 0.5, 0.5} = Gris moyen partout. Montez à 0.8 pour que ce soit très clair.
    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // --- 3. CONFIGURATION DE LA LAMPE (SOLEIL) ---
    // Position avec w=0.0f (le 4ème paramètre) -> C'est une lumière DIRECTIONNELLE (infini)
    // Elle vient de la diagonale haut-droite-devant.
    GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
    
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f }; // Lumière blanche forte
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Reflet blanc brillant

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // --- 4. MATÉRIAUX ---
    // Permet à glColor3f() de définir la couleur de l'objet
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Ajoute le "brillant" (plastique) pour voir les courbes
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
    gluLookAt(0., 0., 4.,  // position de la caméra
              0., 0., 0.,  // point regardé
              0., 1., 0.); // vecteur "up"
}
