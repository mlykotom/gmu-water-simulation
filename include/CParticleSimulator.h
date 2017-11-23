#ifndef CPARTICLESIMULATOR_H
#define CPARTICLESIMULATOR_H

#include<QObject>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <CScene.h>
#include <CParticle.h>
#include <cmath>
#include "FluidCube.h"


class CParticleSimulator: QObject
{
Q_OBJECT

    typedef std::vector<CParticle> particleVector;


private slots:
    void doWork();

private:
    QTimer timer;
    CScene *scene;

    particleVector *particles;

    double dt = 0.01;
    unsigned long iteration = 0;
    int size = 10;
    int particlesNumber = 2000;


public:

    CParticleSimulator(CScene *pScene);
    ~CParticleSimulator();

    void start();

    void setupParticles()
    {
        particles = new particleVector();

        for (int i = 0; i < particlesNumber; ++i) {
            particles->push_back(CParticle(scene->m_rootEntity));
        }
    }
    void randomParticles();
    FluidCube *cube;
};

#endif