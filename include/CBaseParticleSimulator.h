#ifndef CPARTICLESIMULATOR_H
#define CPARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QPlaneMesh>
#include <QtMath>

#include <iostream>
#include <QElapsedTimer>

#include "CScene.h"
#include "CParticle.h"
#include "CGrid.h"

#define GRAVITY_ACCELERATION (-9.80665f)

class CScene;

class CBaseParticleSimulator: public QObject
{
Q_OBJECT

private slots:
    void doWork();

public slots:
    virtual void onKeyPressed(Qt::Key key);

signals:
    void iterationChanged(unsigned long iteration);

private: //attributes
    QTimer m_timer;
    QElapsedTimer m_elapsed_timer;
    unsigned long iterationSincePaused;
    unsigned long totalIteration;

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

    QVector3D m_cellSize; // TODO is it anyhow useful?

    cl_float dt;
    CGrid *m_grid;
    std::vector<CParticle *> m_particles;
    QVector3D m_boxSize;
    cl_float m_surfaceThreshold;
    cl_uint m_particlesCount = 0;
    SystemParams m_systemParams;

    virtual void updateGrid() = 0;
    virtual void updateDensityPressure() = 0;
    virtual void updateForces() = 0;
    virtual void integrate() = 0;

public:
    explicit CBaseParticleSimulator(CScene *scene, QObject *parent);
    ~CBaseParticleSimulator() override
    {
        delete m_grid;
    }

    virtual void setupScene();
    virtual void start();

    void toggleSimulation();
    void toggleGravity();
    virtual void setGravityVector(QVector3D newGravity);

    virtual QString getSelectedDevice() = 0;

    qint64 getElapsedTime() { return m_elapsed_timer.elapsed(); }
    double getFps();
    unsigned long getParticlesCount() { return m_particlesCount; }
    int getGridSizeX() { return m_grid->xRes(); }
    int getGridSizeY() { return m_grid->yRes(); }
    int getGridSizeZ() { return m_grid->zRes(); }
    void step();
};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H