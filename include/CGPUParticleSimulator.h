#ifndef WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include <memory>
#include "CGPUBaseParticleSimulator.h"

class CGPUParticleSimulator: public CGPUBaseParticleSimulator
{
public:
    explicit CGPUParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent = nullptr);
    ~CGPUParticleSimulator()
    {
    }

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
private: //methods
    void scanGrid();

protected:
    void setupKernels() override;

    cl_int m_localWokrgroupSize;

    std::shared_ptr<cl::Kernel> m_updateParticlePositionsKernel;
    std::shared_ptr<cl::Kernel> m_scanLocalKernel;
    std::shared_ptr<cl::Kernel> m_incrementKernel;
    std::shared_ptr<cl::Kernel> m_densityPresureStepKernel;
    std::shared_ptr<cl::Kernel> m_forceStepKernel;
    std::shared_ptr<cl::Kernel> m_integrationStepKernel;

    std::vector<cl_int> m_gridVector;
    std::vector<cl_int> m_sortedIndices;
    std::vector<cl_int> m_sums;

    cl_int m_gridCountToPowerOfTwo;
    cl_int3 m_gridSize;
    cl_int m_localScanWokrgroupSize;
    cl_int m_sumsCount;
    cl_int m_elementsProcessedInOneGroup;
    cl_float3 m_halfBoxSize;

    cl::NDRange m_scanLocal;
    cl::NDRange m_scanGlobal;
    cl::NDRange m_sumsGlobal;
    cl::NDRange m_local;
    cl::NDRange m_global;

    size_t m_sumsSize;
    size_t m_particlesSize;
    size_t m_indicesSize;
    size_t m_gridVectorSize;

    cl::Buffer m_particlesBuffer;
    cl::Buffer m_indicesBuffer;
    cl::Buffer m_gridBuffer;
    cl::Buffer m_scanSumsBuffer;
};


#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
