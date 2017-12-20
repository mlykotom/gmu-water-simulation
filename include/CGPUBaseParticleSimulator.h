
#ifndef WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H

#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include "CLPlatforms.h"
#include <cassert>

class CGPUBaseParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;
    cl_float3 m_gravityCL;
    std::vector<CParticle::Physics> m_clParticles;

    virtual void setupKernels() = 0;
public:
    explicit CGPUBaseParticleSimulator(CScene *scene, QObject *parent = nullptr);
    void setGravityVector(QVector3D newGravity) override;
    QString getSelectedDevice() override;
    void setupScene() override;
};

#endif //WATERSURFACESIMULATION_CGPUBASEPARTICLESIMULATOR_H
