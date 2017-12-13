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


    CScene *m_scene;
    QVector3D gravity;

    QVector3D m_cellSize; // TODO is it anyhow useful?

    cl_float dt;
    CGrid *m_grid;
    QVector3D m_boxSize;
    cl_float m_surfaceThreshold;
    cl_int m_particlesCount = 0;

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

    qint64 getElapsedTime() { return m_elapsed_timer.elapsed(); }
    double getFps();
    unsigned long getParticlesCount() { return m_particlesCount; }
    int getGridSizeX() { return m_grid->xRes(); }
    int getGridSizeY() { return m_grid->yRes(); }
    int getGridSizeZ() { return m_grid->zRes(); }
    void step();

    // TODO  not sure if every implementation has it (or it should be static)
    double Wpoly6(double radiusSquared);
    QVector3D Wpoly6Gradient(QVector3D &diffPosition, double radiusSquared);
    QVector3D WspikyGradient(QVector3D &diffPosition, double radiusSquared);
    double WviscosityLaplacian(double radiusSquared);

    //TODO: DELETE
    virtual void test() {}
};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
