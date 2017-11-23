#include "CParticleSimulator.h"

CParticleSimulator::CParticleSimulator(CScene *pScene)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
    scene = pScene;

//    cube = new FluidCube(100, 2, 3, 0.01);
//    cube->addDensity(1, 1, 1, 1);
//    cube->addVelocity(2, 2, 2, 1, 2, 3);

    setupParticles();
}

CParticleSimulator::~CParticleSimulator()
{
    delete[] particles;
//    delete cube;
}

void CParticleSimulator::setupParticles()
{
    particles = new particleVector();

    for (int i = 0; i < particlesNumber; ++i) {
        particles->push_back(new CParticle(scene->m_rootEntity));
    }
}

void CParticleSimulator::start()
{
    timer.start();
}

void CParticleSimulator::doWork()
{
    this->step();
    this->render();
}

void CParticleSimulator::step()
{
    for (int i = 0; i < particlesNumber; ++i) {
        auto particle = particles->at(i);

        float raX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
        float raY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
        float raZ = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));

        particle->position.setX(raX);
        particle->position.setY(raY);
        particle->position.setZ(raZ);
    }
}

void CParticleSimulator::render()
{
    for (int i = 0; i < particlesNumber; ++i) {
        auto particle = particles->at(i);
        particle->translate(particle->position);
    }
}
