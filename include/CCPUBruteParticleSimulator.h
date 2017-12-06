#ifndef WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"

class CCPUBruteParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;
public:
    explicit CCPUBruteParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    void test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity);
};


#endif //WATERSURFACESIMULATION_CCPUBRUTEPARTICLESIMULATOR_H
