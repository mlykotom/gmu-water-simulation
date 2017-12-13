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
   // virtual void test() override;

private: //methods
    std::vector<cl_int> scan(std::vector<cl_int> input);
    void scanGrid();

    void setupKernels();

    
protected: //methods
    void setGravityVector(QVector3D newGravity) override;

protected:
    cl_int m_localWokrgroupSize;
    CLWrapper *m_cl_wrapper;
    std::shared_ptr<cl::Kernel> m_updateParticlePositionsKernel;
    std::shared_ptr<cl::Kernel> m_reduceKernel;
    std::shared_ptr<cl::Kernel> m_downSweepKernel;
    std::shared_ptr<cl::Kernel> m_densityPresureStepKernel;
    std::shared_ptr<cl::Kernel> m_forceStepKernel;
    std::shared_ptr<cl::Kernel> m_integrationStepKernel;

    std::vector<CParticle::Physics> m_clParticles;
    std::vector<cl_int> m_gridVector;
    std::vector<cl_int> m_sortedIndices;
    std::vector<cl_int> m_gridScan;

    cl_int m_gridCountToPowerOfTwo;
    cl_int3 m_gridSize;
    cl_float3 m_gravityCL;

    size_t m_particlesSize;
    size_t m_scanSize;
    size_t m_indicesSize;
    size_t m_gridVectorSize;

    cl::Buffer m_particlesBuffer;
    cl::Buffer m_scanBuffer;
    cl::Buffer m_indicesBuffer;
    cl::Buffer m_gridBuffer;
};



#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
