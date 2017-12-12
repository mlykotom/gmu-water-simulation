#ifndef WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include <memory>

class CGPUParticleSimulator: public CBaseParticleSimulator
{


public:
    explicit CGPUParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void setupScene() override;
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    void test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity);

    //TODO: TEST - DELETE
    virtual void test() override;

private: //methods
    std::vector<cl_int> scan(std::vector<cl_int> input);

protected:
    CLWrapper *m_cl_wrapper;
    std::shared_ptr<cl::Kernel> m_updateParticlePositionsKernel;
    std::shared_ptr<cl::Kernel> m_reduceKernel;
    std::shared_ptr<cl::Kernel> m_downSweepKernel;

    std::vector<CParticle::Physics> m_clParticles;
    std::vector<cl_int> m_gridVector;
    std::vector<cl_int> m_sortedIndices;
    std::vector<cl_int> m_gridScan;
};



#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
