
#ifndef WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H

#include <CBaseParticleSimulator.h>
#include "CGPUBaseParticleSimulator.h"

class CCPUParticleSimulator: public CBaseParticleSimulator
{
public:
    explicit CCPUParticleSimulator(CScene *scene, QObject *parent = nullptr);

    QString getSelectedDevice() override { return "CPU (without OpenCL)"; };
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
};


#endif //WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
