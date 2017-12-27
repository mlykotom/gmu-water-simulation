
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
    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void updateCollisions() override;
    void integrate() override;
};


#endif //WATERSURFACESIMULATION_CCPUPARTICLESIMULATOR_H
