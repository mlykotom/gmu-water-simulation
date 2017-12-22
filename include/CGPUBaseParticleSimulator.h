
#ifndef WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H

#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include "CLPlatforms.h"
#include <cassert>
#include <memory>

class CGPUBaseParticleSimulator: public CBaseParticleSimulator
{
public:
    explicit CGPUBaseParticleSimulator(CScene *scene, float boxSize, cl::Device device, SimulationScenario scenario = DAM_BREAK, QObject *parent = nullptr);
    ~CGPUBaseParticleSimulator() override = default;
    void setGravityVector(QVector3D newGravity) override;
    QString getSelectedDevice() override;
    void setupScene() override;
    void step() override;

protected:
    CLWrapper *m_cl_wrapper;
    cl_float3 m_gravityCL;

    cl::Buffer m_particlesBuffer;
    size_t m_particlesSize;

    // walls for collisions
    QVector<sWall> m_wallsVector;
    cl::Buffer m_wallsBuffer;
    size_t m_wallsBufferSize;
    std::shared_ptr<cl::Kernel> m_walls_collision_kernel;

    std::shared_ptr<cl::Kernel> m_integrationStepKernel;

    virtual void setupKernels();
    void integrate() override;
};

#endif //WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
