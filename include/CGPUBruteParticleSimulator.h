#ifndef WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H

#include <memory>
#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include "CGPUBaseParticleSimulator.h"


class CGPUBruteParticleSimulator: public CGPUBaseParticleSimulator
{
public:

    explicit CGPUBruteParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent = nullptr);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;

protected:
    std::shared_ptr<cl::Kernel> m_update_density_kernel;
    std::shared_ptr<cl::Kernel> m_update_forces_kernel;

    void setupKernels() override;
};


#endif //WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
