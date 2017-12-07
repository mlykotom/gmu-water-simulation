#ifndef WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"

struct ParticleCL
{
    cl_float3 position;
    cl_float3 velocity;
    cl_float3 acceleration;
};

class CCPUBruteParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;

    cl::Kernel m_test_kernel;

    std::vector<ParticleCL> *particlesCL;
public:
    explicit CCPUBruteParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void setupScene() override;
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    void test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity);
};


#endif //WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
