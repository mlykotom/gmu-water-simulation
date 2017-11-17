#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//std
#include <memory>
#include<cstdlib>

//GPUEngine
#include<geAd/SDLWindow/SDLWindow.h>
#include<geAd/SDLWindow/SDLMainLoop.h>
#include<geGL/StaticCalls.h>
#include<geGL/geGL.h>

//camera
#include<geUtil/FreeLookCamera.h>
#include<geUtil/PerspectiveCamera.h>


#include "CameraManipulator.h"

class MainWindow
{

    typedef std::shared_ptr<ge::ad::SDLMainLoop> tMainLoop;
    typedef std::shared_ptr<ge::ad::SDLWindow> tWindow;
    typedef std::shared_ptr<ge::gl::Program> tProgram;
    //typedef std::shared_ptr<fsg::OrbitObjectManipulator> tCamera;
    typedef std::shared_ptr<ge::util::FreeLookCamera> tFreeLookCamera;
    typedef std::shared_ptr<ge::util::PerspectiveCamera> tPerspectiveCameraCamera;

    
    typedef  struct {
        GLuint  count;
        GLuint  instanceCount;
        GLuint  first;
        GLuint  baseInstance;
    } DrawArraysIndirectCommand;

public:
    MainWindow();
    ~MainWindow();

    bool show();
    
private: //attributes

    tMainLoop m_mainLoop;
    tWindow m_window;
    tProgram m_program;
    GLuint m_vao;
    GLuint m_objectDataBuffer;
    tFreeLookCamera m_cameraFL;
    tPerspectiveCameraCamera m_cameraP;

    bool m_initialized;
    bool m_needsToRedraw;

private: //methods
    bool initialize();
    void draw();

    const std::string shaderFromFile(std::string fn);

};

#endif // MAINWINDOW_H