#ifndef WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H

#include "CScene.h"
#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"
#include <memory>

class CGPUParticleSimulator: public CBaseParticleSimulator
{
protected:
    CLWrapper *m_cl_wrapper;
    std::shared_ptr<cl::Kernel> m_kernel;;

public:
    explicit CGPUParticleSimulator(CScene *scene, QObject *parent = nullptr);

    void updateGrid() override;
    void updateDensityPressure() override;
    void updateForces() override;
    void integrate() override;
    void test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity);

    //TODO: TEST - DELETE
    virtual void test() override;

private: //methods
    std::vector<cl_int> scan(std::vector<cl_int> input);
};



#endif //WATERSURFACESIMULATION_CGPUPARTICLESIMULATOR_H
