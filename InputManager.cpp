#include "InputManager.h"
#include "Scene.h"
#include "UVTriangleManager.h" 
#include <cstdlib>
#include <ctime> 


extern void redrawAll(); 

InputManager::InputManager(Scene* sc, UVTriangleManager* tm) : t(0.5f), scene(sc), triManager(tm) {}

void InputManager::keyboard(unsigned char key, int, int) {
    if (key == 'r' || key == 'R') {
        if (triManager) {
            // Régénère 25 nouveaux sites avec une seed basée sur l'heure actuelle
            //triManager->generateVoronoiSites(25, std::time(nullptr)); 
            redrawAll(); 
        }
        return; 
    }

    

    switch(key){
        
        case 'f': glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
        case 'p': glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
        case 's': glPolygonMode(GL_FRONT_AND_BACK,GL_POINT); break;
        case 'q': exit(0);
        case 'e':  // Toggle extrusion on/off
            case 'E':
                triManager->setExtrusionEnabled(!triManager->isExtrusionEnabled());
                std::cout << "Extrusion: " 
                          << (triManager->isExtrusionEnabled() ? "ON" : "OFF") 
                          << std::endl;
                redrawAll();
                break;
                
            case '+':  // Augmenter la profondeur d'extrusion
            case '=':
                {
                    float newDepth = triManager->getExtrusionDepth() + 0.02f;
                    triManager->setExtrusionDepth(newDepth);
                    std::cout << "Profondeur d'extrusion: " << newDepth << std::endl;
                    redrawAll();
                }
                break;
                
            case '-':  // Diminuer la profondeur d'extrusion
            case '_':
                {
                    float newDepth = std::max(0.01f, triManager->getExtrusionDepth() - 0.02f);
                    triManager->setExtrusionDepth(newDepth);
                    std::cout << "Profondeur d'extrusion: " << newDepth << std::endl;
                    redrawAll();
                }
                break;
                
            case 'r':  // Régénérer la triangulation
            case 'R':
                triManager->generateDelaunayTriangulation(30, std::time(nullptr));
                std::cout << "Triangulation régénérée" << std::endl;
                redrawAll();
                break;
            case 'w':
                triManager->setWireframeEnabled(!triManager->isWireframeEnabled());
                std::cout << "Wireframe: " 
                          << (triManager->isWireframeEnabled() ? "ON" : "OFF") 
                          << std::endl;
                redrawAll();
                break;
            case 'x': 
        case 'X':
            triManager->setExplosionEnabled(!triManager->isExplosionEnabled());
            std::cout << "Vue éclatée: " 
                      << (triManager->isExplosionEnabled() ? "ON" : "OFF") 
                      << std::endl;
            redrawAll();
            break;
            
        case '[':  // Diminuer le facteur d'éclatement
            {
                float newFactor = std::max(0.0f, triManager->getExplosionFactor() - 0.1f);
                triManager->setExplosionFactor(newFactor);
                std::cout << "Facteur d'éclatement: " << newFactor << std::endl;
                redrawAll();
            }
            break;
            
        case ']':  // Augmenter le facteur d'éclatement
            {
                float newFactor = triManager->getExplosionFactor() + 0.1f;
                triManager->setExplosionFactor(newFactor);
                std::cout << "Facteur d'éclatement: " << newFactor << std::endl;
                redrawAll();
            }
            break;
    }
    
    
    glutPostRedisplay(); 
}

void InputManager::mouse(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON) mouseLeftDown = (state == GLUT_DOWN);
    if(button == GLUT_RIGHT_BUTTON) mouseRightDown = (state == GLUT_DOWN);
}

void InputManager::mouseMotion(int x, int y) {
    if(mouseLeftDown){
        scene->cameraAngleY += (x - mouseX);
        scene->cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown){
        scene->cameraDistance += (y - mouseY) * 0.02f; // zoom
        mouseY = y;
    }
    glutPostRedisplay();
}
