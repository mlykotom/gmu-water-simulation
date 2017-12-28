
#ifndef WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H

#include <CBaseParticleSimulator.h>
#include "CGPUBaseParticleSimulator.h"

class CCPUParticleSimulator: public CBaseParticleSimulator
{
private:
    double Wpoly6(double radiusSquared);
    QVector3D WspikyGradient(QVector3D &diffPosition, double radiusSquared);
    double WviscosityLaplacian(double radiusSquared);

public:
    explicit CCPUParticleSimulator(CScene *scene, float boxSize, SimulationScenario scenario = DAM_BREAK, QObject *parent = nullptr);

    QString getSelectedDevice() override { return "CPU (without OpenCL)"; };
    double updateGrid() override;
    double updateDensityPressure() override;
    double updateForces() override;
    double updateCollisions() override;
    double integrate() override;
};


#endif //WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
