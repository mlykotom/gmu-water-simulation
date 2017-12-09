#ifndef WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H

#include <memory>
#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"


class CCPUBruteParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;

    std::shared_ptr<cl::Kernel> m_integration_kernel;
    std::shared_ptr<cl::Kernel> m_update_density_kernel;
    std::shared_ptr<cl::Kernel> m_update_forces_kernel;

    CParticle::Physics *device_data;

    cl_float3 gravityCL;
public:
    explicit CCPUBruteParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void setupScene() override;
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    ~CCPUBruteParticleSimulator() override;

    void toggleGravity();

    cl::Buffer outputBuffer;
//    cl::Buffer inputBuffer;
    size_t dataBufferSize;
};


#endif //WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
