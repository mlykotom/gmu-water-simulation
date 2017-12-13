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

    CParticle::Physics *m_device_data;

    cl_float3 m_gravityCL;
public:

    explicit CCPUBruteParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void setupScene() override;
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    ~CCPUBruteParticleSimulator() override;

    void setGravityVector(QVector3D newGravity) override;

    cl::Buffer m_outputBuffer;

    size_t m_dataBufferSize;
};


#endif //WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
