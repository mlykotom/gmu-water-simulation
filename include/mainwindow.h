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

class MainWindow
{

    typedef std::shared_ptr<ge::ad::SDLMainLoop> tMainLoop;
    typedef std::shared_ptr<ge::ad::SDLWindow> tWindow;
    typedef std::shared_ptr<ge::gl::Program> tProgram;

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

    bool m_initialized;

private: //methods
    bool initialize();
    void draw();

};

#endif // MAINWINDOW_H