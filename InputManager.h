#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glut.h>
#include <ctime> 

class Scene; 
class UVTriangleManager; 

class InputManager {
public:
    InputManager(Scene* scene, UVTriangleManager* triManager);
    float getT() const { return t; }


    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);
    void mouseMotion(int x, int y);

    float t = 0.5f; 
    bool explosionEnabled = false;

private:
    Scene* scene;
    UVTriangleManager* triManager; 
    
    bool mouseLeftDown = false;
    bool mouseRightDown = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
};

#endif
