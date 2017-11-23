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


class CParticleSimulator: QObject
{
Q_OBJECT

    typedef std::vector<CParticle *> particleVector;


private slots:
    void doWork();


private:
    QTimer timer;
    CScene *scene;
    FluidCube *cube;

    particleVector *particles;

    int size = 10;
    int particlesNumber = 1000;

public:

    CParticleSimulator(CScene *pScene);
    ~CParticleSimulator();

    void toggleSimulation()
    {
        if (timer.isActive()) {
            qDebug() << "pausing simulation...";
            timer.stop();
        }
        else {
            qDebug() << "resuming simulation ...";
            timer.start();
        }
    }

    void start();

    void setupParticles();
    void step();
    void render();
};

#endif