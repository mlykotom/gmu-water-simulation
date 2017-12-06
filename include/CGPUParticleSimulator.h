#ifndef WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CGPUParticleSimulator.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"

class CGPUParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;
public:
    explicit CGPUParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void updateNewPositionVelocity() override;
};


#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
