#ifndef CPARTICLESIMULATOR_H
#define CPARTICLESIMULATOR_H

#include<QObject>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <CScene.h>
#include <CParticle.h>
#include <cmath>
#include <QKeyEvent>
#include "FluidCube.h"
#include <vector>

typedef QPair<Qt3DCore::QEntity *, Qt3DExtras::QPhongMaterial *> t_banana;

class CParticleSimulator: QObject
{
Q_OBJECT
    typedef std::vector<CParticle *> particleVector;

private slots:
    void doWork()
    {
        this->step();
        this->render();
    };

private:
    QTimer timer;
    CScene *scene;
    FluidCube *cube;

    particleVector *particles;

    int size = 18;
    int particlesNumber = 1000;

public:
    CParticleSimulator(CScene *pScene);
    ~CParticleSimulator();

    void start() { timer.start(); }

    void toggleSimulation()
    {
        if (timer.isActive()) {
            qDebug() << "pausing simulation...";
            timer.stop();
        }
        else {
            qDebug() << "resuming simulation ...";
            start();
        }
    }

    unsigned long iteration = 0;

    // simulation
    void setup();
    void step();
    void render();
    QPair<Qt3DCore::QEntity *, Qt3DExtras::QPhongMaterial *> createPlane(CScene *scene, float x, float y, float z);

    std::vector<t_banana> *chrobak;

    void addDensity(int x, int y, int z);
};

#endif