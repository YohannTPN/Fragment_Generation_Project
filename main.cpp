#include <GL/glut.h>
#include "Scene.h"
#include "InputManager.h"

#include "Icosphere.h"
#include "OpenGLManager.h"
#include "BezierCurve.h"
#include "BezierSurface.h"
#include "BSplineCurve.h"
#include "BSplineSurface.h"
#include "ParametricMapping.h"
#include "UVTriangleManager.h"
#include "UVTriangleIFS.h"
#include "IFSTransform.h"

#include <armadillo>
#include <cstdlib> 
#include <ctime>   

Scene* scene;
InputManager* input;

OpenGLManager* ogl;

Icosphere* ico;
BezierCurve* bezier;
BezierSurface* surface;

BSplineCurve* bspline;
BSplineSurface* bsplineSurface;

ParametricMapping* mapping;
UVTriangleManager* triManager;

UVTriangleIFS* triangleIFS = nullptr;
HalfEdgeMesh* ifsMesh3D = nullptr;  // Maillage IFS permanent
bool showIFS = false; 
int ifsDepth = 2; 

int mainWindowId;
int uvwWindowId; 

void uvwDisplayWrapper() {
    glutSetWindow(uvwWindowId);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-0.1, 1.1, -0.1, 1.1);

    // Fond
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();

    if (showIFS && triangleIFS) {
        triangleIFS->drawUVFilled();
    } else {
        if(triManager) {
            triManager->drawUVPatches();
        }
    }
    
    glFlush();
    glutSwapBuffers();
    
    glutSetWindow(mainWindowId);
    glMatrixMode(GL_PROJECTION); 
    ogl->setupProjection(); 
}

void displayWrapper() { 
    glutSetWindow(mainWindowId);
    glPushMatrix(); 
    
    scene->display(input->getT()); 

    if (showIFS && ifsMesh3D) {
    // Détection changement extrusion
    static bool lastExtrusionState = false;
    if (lastExtrusionState != triManager->isExtrusionEnabled()) {
        delete ifsMesh3D;
        ifsMesh3D = new HalfEdgeMesh();
        triangleIFS->buildIFSMesh(ifsMesh3D, mapping);
        lastExtrusionState = triManager->isExtrusionEnabled();
    }
    
    // Dessin
    if (triManager->isExplosionEnabled()) {
        ifsMesh3D->drawExploded(triManager->getExplosionFactor());
    } else {
        ifsMesh3D->draw();
    }
    
    if (triManager->isWireframeEnabled()) {
        ifsMesh3D->drawWireframe();
    }
} else {
    triManager->draw();  // Maillage normal
}

    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}

void regenerateIFSMesh() {
    if (!triangleIFS || !mapping) return;
    
    std::cout << "\n=== Régénération du maillage IFS 3D ===\n";
    
    if (ifsMesh3D != nullptr) {
        delete ifsMesh3D;
    }
    
    ifsMesh3D = new HalfEdgeMesh();
    triangleIFS->buildIFSMesh(ifsMesh3D, mapping);
    
    std::cout << "=== Maillage IFS 3D prêt ===\n\n";
}

void redrawAll() {
    glutPostRedisplay();
    glutSetWindow(uvwWindowId);
    glutPostRedisplay();
    glutSetWindow(mainWindowId); 
}

int main(int argc,char** argv){
    std::srand(std::time(nullptr)); 

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200,200);
    glutInitWindowSize(600,600);
    mainWindowId = glutCreateWindow("Objet 3D (Espace XYZ)");

    ogl = new OpenGLManager();
    ogl->init();
    ogl->setupProjection();

    scene = new Scene();
    
    mat P = {
        {0.2, 0.0},  
        {0.3, 0.5},   
        {0.4, 1.0},   
        {0.5, 1.5}    
    };

    bezier = new BezierCurve(P);
    surface = new BezierSurface(bezier, 20); 

    mat Q = {
        {0.2, 0.0},   
        {0.4, 0.5},   
        {0.3, 1.0},   
        {0.5, 1.5},   
        {0.4, 2.0}    
    };
    bspline = new BSplineCurve(Q, 3); 
    bsplineSurface = new BSplineSurface(bspline, 20); 

    scene->setRenderable(bsplineSurface); 
    
    glEnable(GL_DEPTH_TEST);

    mapping = new ParametricMapping(bsplineSurface);
    triManager = new UVTriangleManager(mapping);

    triManager->setExtrusionDepth(0.15f);  
    triManager->setExtrusionEnabled(true); 
    triManager->generateDelaunayTriangulation(30, std::time(nullptr));

    // === CODE IFS ===
    triangleIFS = new UVTriangleIFS(triManager);
    
// Paramètres selon le fichier courbe.txt
float a_param = -0.1f;   // Paramètre a
float a = 2.0f * a_param;       // a = 2*a_param = -0.2
float a1 = 0.5f - a;            // a1 = 0.5 - a = 0.7 (mais fichier dit 0.6, donc utiliser 0.6)

float c_param = 0.2f;    // Paramètre c  
float b = 2.0f * c_param;       // b = 2*c_param = 0.4
float b1 = 0.5f - b;            // b1 = 0.5 - b = 0.1 (mais fichier dit 0.3, donc utiliser 0.3)

// Valeurs finales (pour que les sommes = 1)
a1 = 0.6f;
a = -0.2f;
b1 = 0.3f;
b = 0.4f;

std::cout << "\n=== Configuration IFS ===\n";
std::cout << "T0 (segment gauche): a=" << a << ", a1=" << a1 << "\n";
std::cout << "T1 (segment droit): b=" << b << ", b1=" << b1 << "\n";

// Créer les 2 transformations
IFSTransform T0 = IFSTransform::createT0(a, a1);
IFSTransform T1 = IFSTransform::createT1(b, b1);

std::cout << "\nMatrice T0 (segment gauche):\n";
T0.print();
std::cout << "\nMatrice T1 (segment droit):\n";
T1.print();

// Définir les transformations
triangleIFS->setTransforms(T0, T1);

std::cout << "\n=== Génération IFS ===\n";
triangleIFS->generateIFSTriangles(ifsDepth);
std::cout << "=== IFS prêt ===\n\n";

// Générer le maillage 3D IFS initial
regenerateIFSMesh();
    // === CODE IFS ===

    input = new InputManager(scene, triManager);

    glutDisplayFunc(displayWrapper);
    glutMouseFunc([](int b,int s,int x,int y){ input->mouse(b,s,x,y); });
    glutMotionFunc([](int x,int y){ input->mouseMotion(x,y); });
    
    glutKeyboardFunc([](unsigned char k,int x,int y){ 
        input->keyboard(k,x,y);
    });

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(850, 200);
    glutInitWindowSize(300, 300);
    uvwWindowId = glutCreateWindow("Espace Parametrique (UV)");
    glutDisplayFunc(uvwDisplayWrapper);
    
    glutKeyboardFunc([](unsigned char k,int x,int y){ 
        input->keyboard(k,x,y);
        
        // Nouveaux contrôles IFS
        if (k == 'i' || k == 'I') {
            showIFS = !showIFS;
            std::cout << "IFS " << (showIFS ? "ON" : "OFF") << std::endl;
            redrawAll();
        }
        else if (k == '+' || k == '=') {
            ifsDepth++;
            std::cout << "Profondeur IFS: " << ifsDepth << " (régénération...)" << std::endl;
            triangleIFS->generateIFSTriangles(ifsDepth);
            regenerateIFSMesh();
            std::cout << "Fait!" << std::endl;
            redrawAll();
        }
        else if (k == '-' && ifsDepth > 0) {
            ifsDepth--;
            std::cout << "Profondeur IFS: " << ifsDepth << " (régénération...)" << std::endl;
            triangleIFS->generateIFSTriangles(ifsDepth);
            regenerateIFSMesh();
            std::cout << "Fait!" << std::endl;
            redrawAll();
        }
    });

    glutMainLoop();
    return 0;
}