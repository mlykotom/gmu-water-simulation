#include "CGPUBaseParticleSimulator.h"

CGPUBaseParticleSimulator::CGPUBaseParticleSimulator(CScene *scene, float boxSize, cl::Device device, SimulationScenario scenario, QObject *parent)
    : CBaseParticleSimulator(scene, boxSize, scenario, parent),
      m_gravityCL({gravity.x(), gravity.y(), gravity.z()})
{
    m_cl_wrapper = new CLWrapper(std::move(device));
}

void CGPUBaseParticleSimulator::setGravityVector(QVector3D newGravity)
{
    CBaseParticleSimulator::setGravityVector(newGravity);
    m_gravityCL = {gravity.x(), gravity.y(), gravity.z()};
}

QString CGPUBaseParticleSimulator::getSelectedDevice()
{
    return CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());
}

void CGPUBaseParticleSimulator::setupScene()
{
    CBaseParticleSimulator::setupScene();
    setupKernels();
}

void CGPUBaseParticleSimulator::step()
{
    try {
        CBaseParticleSimulator::step();
    }
    catch (CLException &exc) {
        emit errorOccured(exc.what());
        stop();
    }
}

void CGPUBaseParticleSimulator::setupKernels()
{
    // particles
    m_particlesSize = m_particlesCount * sizeof(CParticle::Physics);
    m_particlesBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_particlesSize);

    // integration
    m_integrationStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));
    cl_uint arg = 0;
    m_integrationStepKernel->setArg(arg++, m_particlesBuffer);
    m_integrationStepKernel->setArg(arg++, m_particlesCount);
    m_integrationStepKernel->setArg(arg++, dt);

    // collisions
    m_wallsVector = m_grid->getCollisionGeometry()->getBoundingBox().m_walls;
    m_wallsBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_ONLY, static_cast<size_t>(m_wallsVector.size()));
    m_wallsBufferSize = m_wallsVector.size() * sizeof(sWall);
    m_wallsBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_ONLY, m_wallsBufferSize);

    m_walls_collision_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("walls_collision"));

    cl_uint argCollision = 0;
    m_walls_collision_kernel->setArg(argCollision++, m_particlesBuffer);
    m_walls_collision_kernel->setArg(argCollision++, m_wallsBuffer);
    m_walls_collision_kernel->setArg(argCollision++, m_particlesCount);
    m_walls_collision_kernel->setArg(argCollision++, m_wallsVector.size());

    m_cl_wrapper->enqueueWrite(m_wallsBuffer, m_wallsBufferSize, m_wallsVector.data(), CL_TRUE);
}
void CGPUBaseParticleSimulator::integrate()
{
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->enqueueKernel(*m_integrationStepKernel, global);
    m_cl_wrapper->enqueueRead(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_FALSE);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}
