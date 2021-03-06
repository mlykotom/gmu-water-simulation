#ifndef CPARTICLESIMULATOR_H
#define CPARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QtMath>
#include <cassert>
#include <QSphereMesh>

#include <iostream>
#include <QElapsedTimer>

#include "CScene.h"
#include "CParticle.h"
#include "CGrid.h"
#include "SProfilingEvent.h"
#include "config.h"

#define GRAVITY_ACCELERATION (-9.80665f)

enum SimulationScenario
{
    DAM_BREAK = 0,
    FOUNTAIN,
};

class CScene;

class CBaseParticleSimulator: public QObject
{
Q_OBJECT

public:
    explicit CBaseParticleSimulator(CScene *scene, float boxSize, SimulationScenario scenario = DAM_BREAK, QObject *parent = nullptr);
    ~CBaseParticleSimulator() override
    {
        delete m_grid;
    }

    virtual void setupScene();
    virtual void setGravityVector(QVector3D newGravity);
    virtual QString getSelectedDevice() = 0;

    void start();
    void stop();
    virtual void step();
    void toggleSimulation();
    void toggleGravity();

    qint64 getElapsedTime() { return m_elapsed_timer.elapsed(); }
    double getFps();
    unsigned long getParticlesCount() { return m_particlesCount; }
    unsigned long getMaxParticlesCount() { return m_maxParticlesCount; }
    QList<sProfilingEvent> events;
    int eventLoggerStride = 10;
    SimulationScenario m_scenario;

    Qt3DExtras::QSphereMesh *particle_mesh;
    Qt3DExtras::QPhongMaterial *particle_material;
signals:
    void iterationChanged(unsigned long iteration);
    void errorOccured(const char *error);

public slots:
    virtual void onKeyPressed(Qt::Key key);

protected:

#ifdef WIN32
    typedef __declspec(align(16)) struct sSystemParams
#else
    typedef struct __attribute__((aligned(16))) sSystemParams
#endif
    {
        cl_float poly6_constant;
        cl_float spiky_constant;
        cl_float viscosity_constant;
    } SystemParams;

    CScene *m_scene;
    QVector3D gravity;

    cl_float dt;
    CGrid *m_grid;
    QVector3D m_boxSize;
    cl_float m_surfaceThreshold;
    cl_uint m_maxParticlesCount = 0;
    cl_int m_particlesCount = 0;
    SystemParams m_systemParams;
    std::vector<CParticle::Physics> m_clParticles;

    virtual double updateGrid() = 0;
    virtual double updateDensityPressure() = 0;
    virtual double updateForces() = 0;
    virtual double updateCollisions() = 0;
    virtual double integrate() = 0;

    bool isRunning() { return m_timer.isActive(); }
private slots:
    void doWork();

private: //attributes
    QTimer m_timer;
    QElapsedTimer m_elapsed_timer;
    unsigned long iterationSincePaused;
    unsigned long totalIteration;

    void addParticle(float x, float y, float z, cl_float3 initialVelocity = {0, 0, 0});
    void generateParticles();
};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
