#include "CParticleSimulator.h"

CParticleSimulator::CParticleSimulator(CScene *pScene)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
    scene = pScene;

    cube = new FluidCube(100, 2, 3, 0.01);
    cube->addDensity(1, 1, 1, 1);
    cube->addVelocity(2, 2, 2, 1, 2, 3);


    setupParticles();
    start();
}

CParticleSimulator::~CParticleSimulator()
{
    delete[] particles;

//    FluidCubeFree(cube);
}

void CParticleSimulator::start()
{
    timer.start();
}

void CParticleSimulator::doWork()
{
//    this->step();
//    this->render();

//    cube->step();

    randomParticles();
}

void CParticleSimulator::randomParticles()
{
    for (int i = 0; i < particlesNumber; ++i) {
        auto particle = particles->at(i);

        float raX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
        float raY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));

        particle.position.setX(raX);
        particle.position.setY(raY);
        particle.translate(particle.position);
    }

    iteration++;
}
