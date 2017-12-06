
#ifndef WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H

#include <CParticleSimulator.h>

class CCPUParticleSimulator: public CParticleSimulator
{
public:
    CCPUParticleSimulator(CScene *scene, QObject *parent);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void updateNewPositionVelocity() override;
};


#endif //WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
