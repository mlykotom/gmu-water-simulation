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
    explicit CGPUParticleSimulator(CScene *scene, float boxSize, cl::Device device, SimulationScenario scenario = DAM_BREAK, QObject *parent = nullptr);
    ~CGPUParticleSimulator() override = default;

    double updateGrid() override;
    double updateDensityPressure() override;
    double updateForces() override;
private: //methods
    double scanGrid();
    double sortIndices();

protected:
    void setupKernels() override;

    std::shared_ptr<cl::Kernel> m_updateParticlePositionsKernel;
    std::shared_ptr<cl::Kernel> m_scanLocalKernel;
    std::shared_ptr<cl::Kernel> m_incrementKernel;
    std::shared_ptr<cl::Kernel> m_densityPresureStepKernel;
    std::shared_ptr<cl::Kernel> m_forceStepKernel;

    std::vector<cl_int> m_gridVector;
    std::vector<cl_int> m_sortedIndices;
    std::vector<cl_int> m_sums;

    cl_int m_gridCountToPowerOfTwo;
    cl_int3 m_gridSize;
    size_t m_localScanWokrgroupSize;
    size_t m_sumsCount;
    size_t m_elementsProcessedInOneGroup;
    cl_float3 m_halfBoxSize;

    cl::NDRange m_scanLocal;
    cl::NDRange m_scanGlobal;
    cl::NDRange m_sumsGlobal;

    size_t m_indicesSize;
    size_t m_gridVectorSize;

    cl::Buffer m_indicesBuffer;
    cl::Buffer m_gridBuffer;
    cl::Buffer m_scanSumsBuffer;
};


#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
