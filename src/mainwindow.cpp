#include "mainwindow.h"

//std
#include <fstream>
#include <streambuf>

//GPUEngine
//#include<geGL/geGL.h>


//glm
#include<glm/glm.hpp>


MainWindow::MainWindow()
    :m_initialized(false),
    m_needsToRedraw(false)
{
    //create window
    //glm::uvec2 windowSize = glm::uvec2(1024, 768);

    m_mainLoop = std::make_shared<ge::ad::SDLMainLoop>();
    m_window = std::make_shared<ge::ad::SDLWindow>();
    //m_camera = std::make_shared<fsg::OrbitObjectManipulator>();

    m_cameraFL = std::make_shared<ge::util::FreeLookCamera>();
    m_cameraP = std::make_shared<ge::util::PerspectiveCamera>();


    // Matrices
    //float fovy = glm::pi<float>() / 4.0f; // 45 degrees
    //std::shared_ptr<glm::mat4> perspectiveMat(std::make_shared<glm::mat4>());
    //*perspectiveMat = glm::perspective(fovy, (float)(m_window->getWidth()) / (float)m_window->getHeight(), 1.0f, 1000.0f);
    ////std::shared_ptr<glm::mat4> viewMatrix(std::make_shared<glm::mat4>());

    //m_camera->setLocalUp(glm::vec3(0.0, 1.0, 0.0));
    //m_camera->setEye(glm::vec3(0.0, 0.0, -2.0));
    //m_camera->setCenter(glm::vec3(0.0,0.0,1.0));
   
    //m_camera->sensitivityZ = 0.5;
    //m_camera->updateViewMatrix();

    //m_window->setEventCallback(SDL_KEYDOWN, [&](SDL_Event const&e) {
    //    m_camera->rotate(-0.1, 0.1);
    //    m_camera->updateViewMatrix();
    //    return true;
    //});

    //m_window->setEventCallback(SDL_MOUSEWHEEL, [&](SDL_Event const&e) {
    //    m_camera->zoom(0.1);
    //    m_camera->updateViewMatrix();
    //    return true;
    //});

    m_window->setWindowEventCallback(SDL_WINDOWEVENT_RESIZED, [&](SDL_Event const&e) {
        
        m_needsToRedraw = true;
        return true;
    });

    m_window->setEventCallback(SDL_MOUSEWHEEL, [&](SDL_Event const&e) {
        //glm::vec3 newPos = m_cameraFL->getPosition() - glm::vec3(0.0, 0.0, -0.1);
        //m_cameraFL->setPosition(newPos);

        if (e.wheel.y == 1) // scroll up
        {
            // Pull up code here!
            m_cameraFL->move(2, -0.1);

        }
        else if (e.wheel.y == -1) // scroll down
        {
            // Pull down code here!
            m_cameraFL->move(2, 0.1);
        }


        m_needsToRedraw = true;
        return true;
    });
    
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
    auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, shaderFromFile(APP_RESOURCES"/shaders/vert2.glsl"));
    auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, shaderFromFile(APP_RESOURCES"/shaders/frag.glsl"));
    m_program = std::make_shared<ge::gl::Program>(vs, fs);
    
    if (ge::gl::glGetError() != 0)
    {
        std::cout << "glerr " << ge::gl::glGetError() << std::endl;

    }

    ////buffer data
    //std::vector<float>vertices = {
    //    //triangle vertices
    //    -1.f,-1.f,
    //    +1.f,-1.f,
    //    -1.f,+1.f,

    //    //diamond vertices
    //    +0.f,-1.f,
    //    +1.f,+0.f,
    //    -1.f,+0.f,
    //    +0.f,+1.f,
    //};
    //std::vector<int32_t>drawIds = {
    //    0,1,2,3,
    //    4,5,6
    //};

    //struct ObjectData {
    //    glm::vec2 position;
    //    glm::vec2 size;
    //    glm::vec4 color;
    //};
    //std::vector<ObjectData>objectData({
    //    { glm::vec2(+0.5f,-0.5f),glm::vec2(0.03f),glm::vec4(1,0,0,1) },
    //    { glm::vec2(+0.5f,-0.2f),glm::vec2(0.06f),glm::vec4(0,1,0,1) },
    //    { glm::vec2(+0.5f,+0.2f),glm::vec2(0.10f),glm::vec4(0,0,1,1) },
    //    { glm::vec2(+0.5f,+0.5f),glm::vec2(0.13f),glm::vec4(0,1,1,1) },
    //    { glm::vec2(-0.5f,-0.5f),glm::vec2(0.03f),glm::vec4(1,1,1,1) },
    //    { glm::vec2(-0.5f,+0.0f),glm::vec2(0.06f),glm::vec4(1,1,0,1) },
    //    { glm::vec2(-0.5f,+0.5f),glm::vec2(0.10f),glm::vec4(1,0,1,1) },
    //});

    ////Generate draw commands
    //DrawArraysIndirectCommand vDrawCommand[2];

    //vDrawCommand[0].count = 3;
    //vDrawCommand[0].instanceCount = 4;
    //vDrawCommand[0].first = 0;
    //vDrawCommand[0].baseInstance = 0;

    //vDrawCommand[1].count = 4;
    //vDrawCommand[1].instanceCount = 3;
    //vDrawCommand[1].first = 3;
    //vDrawCommand[1].baseInstance = 4;


    //GLuint gIndirectBuffer;

    //ge::gl::glGenBuffers(1, &gIndirectBuffer);
    //ge::gl::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
    //ge::gl::glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_STATIC_DRAW);

    ////This buffer contains vertex positions.
    //GLuint vbo;
    //ge::gl::glCreateBuffers(1, &vbo);
    //ge::gl::glNamedBufferData(vbo, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data(), GL_STATIC_DRAW);

    ////This buffer contains drawIds.
    //GLuint drawIdsBuffer;
    //ge::gl::glCreateBuffers(1, &drawIdsBuffer);
    //ge::gl::glNamedBufferData(drawIdsBuffer, drawIds.size() * sizeof(decltype(drawIds)::value_type), drawIds.data(), GL_STATIC_DRAW);

    ////This buffer contains positions, sizes and colors of all instances.
    //ge::gl::glCreateBuffers(1, &m_objectDataBuffer);
    //ge::gl::glNamedBufferData(m_objectDataBuffer, objectData.size() * sizeof(decltype(objectData)::value_type), objectData.data(), GL_STATIC_DRAW);


    //// vertex array object
    //ge::gl::glCreateVertexArrays(1, &m_vao);

    ////attrib 0. is vertex positions
    //ge::gl::glVertexArrayAttribFormat(m_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    //ge::gl::glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, sizeof(float) * 2);
    //ge::gl::glVertexArrayAttribBinding(m_vao, 0, 0);
    //ge::gl::glEnableVertexArrayAttrib(m_vao, 0);
    //ge::gl::glVertexArrayBindingDivisor(m_vao, 0, 0);

    ////attrib 1. simulates gl_DrawID
    //ge::gl::glVertexArrayAttribIFormat(m_vao, 1, 1, GL_INT, 0);
    //ge::gl::glVertexArrayVertexBuffer(m_vao, 1, drawIdsBuffer, 0, sizeof(int32_t));
    //ge::gl::glVertexArrayAttribBinding(m_vao, 1, 1);
    //ge::gl::glEnableVertexArrayAttrib(m_vao, 1);
    //ge::gl::glVertexArrayBindingDivisor(m_vao, 1, 1);



    //CUBE
    std::vector<float>vertices = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };

    //This buffer contains vertex positions.
    GLuint vbo;
    ge::gl::glCreateBuffers(1, &vbo);
    ge::gl::glNamedBufferData(vbo, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data(), GL_STATIC_DRAW);

    // vertex array object
    ge::gl::glCreateVertexArrays(1, &m_vao);

    //attrib 0. is vertex positions
    ge::gl::glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    ge::gl::glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, sizeof(float) * 3);
    ge::gl::glVertexArrayAttribBinding(m_vao, 0, 0);
    ge::gl::glEnableVertexArrayAttrib(m_vao, 0);
    ge::gl::glVertexArrayBindingDivisor(m_vao, 0, 0);

    //CUBE



    ge::gl::glClearColor(0, 0, 0, 1);

    //bind draw loop
    std::function<void(void)> callback = std::bind(&MainWindow::draw,this);
    m_mainLoop->setIdleCallback(callback);

    m_initialized = true;
    m_needsToRedraw = true;
}

void MainWindow::draw()
{
    if (!m_needsToRedraw)
        return;

    using namespace ge::gl;

    glClear(GL_COLOR_BUFFER_BIT);
    m_program->use();


    GLenum err = glGetError();
    if (err != 0)
    {
        std::cout << "glerr " << err << std::endl;
    }

    glBindVertexArray(m_vao);


    //glm::mat4 mvp;
    glm::mat4 mvp = m_cameraFL->getView() * m_cameraP->getProjection();
    m_program->setMatrix4fv("mvp", glm::value_ptr(mvp), 1, GL_FALSE);
    err = glGetError();

    if (err != 0)
    {
        std::cout << "glerr " << err << std::endl;
        exit(-1);
    }

    glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares

    err = glGetError();

    if (err != 0)
    {
        std::cout << "glerr " << err << std::endl;
        exit(-1);

    }
    //
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_objectDataBuffer);


    //glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, (GLvoid*)0, 2, 0);

    glBindVertexArray(0);
    m_window->swap();

    m_needsToRedraw = false;

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

