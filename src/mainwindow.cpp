#include "mainwindow.h"

//std
#include <fstream>
#include <streambuf>

//glm
#include<glm/glm.hpp>


MainWindow::MainWindow()
    :m_initialized(false)
{
    //create window
    m_mainLoop = std::make_shared<ge::ad::SDLMainLoop>();
    m_window = std::make_shared<ge::ad::SDLWindow>();
}

MainWindow::~MainWindow()
{
}


bool MainWindow::initialize()
{
    if (m_initialized)
        return true;

    if (!m_window->createContext("rendering"))
        return false;

    m_mainLoop->addWindow("mainWindow", m_window);

    //init OpenGL
    ge::gl::init(SDL_GL_GetProcAddress);
    ge::gl::setHighDebugMessage();

    //create shader program
    auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, shaderFromFile(APP_RESOURCES"/shaders/vert.glsl"));
    auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, shaderFromFile(APP_RESOURCES"/shaders/frag.glsl"));
    m_program = std::make_shared<ge::gl::Program>(vs, fs);
    
    //buffer data
    std::vector<float>vertices = {
        //triangle vertices
        -1.f,-1.f,
        +1.f,-1.f,
        -1.f,+1.f,

        //diamond vertices
        +0.f,-1.f,
        +1.f,+0.f,
        -1.f,+0.f,
        +0.f,+1.f,
    };
    std::vector<int32_t>drawIds = {
        0,1,2,3,
        4,5,6
    };

    struct ObjectData {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec4 color;
    };
    std::vector<ObjectData>objectData({
        { glm::vec2(+0.5f,-0.5f),glm::vec2(0.03f),glm::vec4(1,0,0,1) },
        { glm::vec2(+0.5f,-0.2f),glm::vec2(0.06f),glm::vec4(0,1,0,1) },
        { glm::vec2(+0.5f,+0.2f),glm::vec2(0.10f),glm::vec4(0,0,1,1) },
        { glm::vec2(+0.5f,+0.5f),glm::vec2(0.13f),glm::vec4(0,1,1,1) },
        { glm::vec2(-0.5f,-0.5f),glm::vec2(0.03f),glm::vec4(1,1,1,1) },
        { glm::vec2(-0.5f,+0.0f),glm::vec2(0.06f),glm::vec4(1,1,0,1) },
        { glm::vec2(-0.5f,+0.5f),glm::vec2(0.10f),glm::vec4(1,0,1,1) },
    });

    //Generate draw commands
    DrawArraysIndirectCommand vDrawCommand[2];

    vDrawCommand[0].count = 3;
    vDrawCommand[0].instanceCount = 4;
    vDrawCommand[0].first = 0;
    vDrawCommand[0].baseInstance = 0;

    vDrawCommand[1].count = 4;
    vDrawCommand[1].instanceCount = 3;
    vDrawCommand[1].first = 3;
    vDrawCommand[1].baseInstance = 4;


    GLuint gIndirectBuffer;

    ge::gl::glGenBuffers(1, &gIndirectBuffer);
    ge::gl::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
    ge::gl::glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_STATIC_DRAW);

    //This buffer contains vertex positions.
    GLuint vbo;
    ge::gl::glCreateBuffers(1, &vbo);
    ge::gl::glNamedBufferData(vbo, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data(), GL_STATIC_DRAW);

    //This buffer contains drawIds.
    GLuint drawIdsBuffer;
    ge::gl::glCreateBuffers(1, &drawIdsBuffer);
    ge::gl::glNamedBufferData(drawIdsBuffer, drawIds.size() * sizeof(decltype(drawIds)::value_type), drawIds.data(), GL_STATIC_DRAW);

    //This buffer contains positions, sizes and colors of all instances.
    ge::gl::glCreateBuffers(1, &m_objectDataBuffer);
    ge::gl::glNamedBufferData(m_objectDataBuffer, objectData.size() * sizeof(decltype(objectData)::value_type), objectData.data(), GL_STATIC_DRAW);


    // vertex array object
    ge::gl::glCreateVertexArrays(1, &m_vao);

    //attrib 0. is vertex positions
    ge::gl::glVertexArrayAttribFormat(m_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    ge::gl::glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, sizeof(float) * 2);
    ge::gl::glVertexArrayAttribBinding(m_vao, 0, 0);
    ge::gl::glEnableVertexArrayAttrib(m_vao, 0);
    ge::gl::glVertexArrayBindingDivisor(m_vao, 0, 0);

    //attrib 1. simulates gl_DrawID
    ge::gl::glVertexArrayAttribIFormat(m_vao, 1, 1, GL_INT, 0);
    ge::gl::glVertexArrayVertexBuffer(m_vao, 1, drawIdsBuffer, 0, sizeof(int32_t));
    ge::gl::glVertexArrayAttribBinding(m_vao, 1, 1);
    ge::gl::glEnableVertexArrayAttrib(m_vao, 1);
    ge::gl::glVertexArrayBindingDivisor(m_vao, 1, 1);

    ge::gl::glClearColor(0, 0, 0, 1);

    //bind draw loop
    std::function<void(void)> callback = std::bind(&MainWindow::draw,this);
    m_mainLoop->setIdleCallback(callback);

    m_initialized = true;
}

void MainWindow::draw()
{
    using namespace ge::gl;

    glClear(GL_COLOR_BUFFER_BIT);
    m_program->use();

    glBindVertexArray(m_vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_objectDataBuffer);


    ///\todo Homework 1.) Replace these drawing commands with single glMultiDrawArraysIndirect call.
    /// You have to create draw indirect buffer that contains correct values (draw commands).
    /// You will need these OpenGL functions:
    /// glMultiDrawArraysIndirect
    /// glCreateBuffers
    /// glNamedBufferData
    /// glBindBuffer
    //glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP,0,3,4,0);
    //glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP,3,4,3,4);

    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, (GLvoid*)0, 2, 0);

    glBindVertexArray(0);
    m_window->swap();
}

const std::string MainWindow::shaderFromFile(std::string fn)
{
    std::ifstream fs(fn);
    std::string str((std::istreambuf_iterator<char>(fs)),
        std::istreambuf_iterator<char>());

    return str;
}

bool MainWindow::show()
{
    if (!initialize())
        return false;

    (*m_mainLoop)();

    return true;
}

