
#ifndef WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H

#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include "CLPlatforms.h"
#include <cassert>

class CGPUBaseParticleSimulator: public CBaseParticleSimulator
{
public:
    explicit CGPUBaseParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent = nullptr);
    ~CGPUBaseParticleSimulator() {}
    void setGravityVector(QVector3D newGravity) override;
    QString getSelectedDevice() override;
    void setupScene() override;

protected:
    CLWrapper *m_cl_wrapper;
    cl_float3 m_gravityCL;
    std::vector<CParticle::Physics> m_clParticles;

    virtual void setupKernels() = 0;

};

#endif //WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
