
#ifndef WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H

#include <CBaseParticleSimulator.h>

class CCPUParticleSimulator: public CBaseParticleSimulator
{
public:
    CCPUParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
};


#endif //WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
