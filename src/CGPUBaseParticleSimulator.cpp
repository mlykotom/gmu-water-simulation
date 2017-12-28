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
    m_particlesSize = m_maxParticlesCount * sizeof(CParticle::Physics);
    m_particlesBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_particlesSize);

    // integration
    m_integrationStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));

    // collisions
    m_wallsVector = m_grid->getCollisionGeometry()->getBoundingBox().m_walls;
    m_wallsCount = (size_t) m_wallsVector.size();
    m_wallsBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_ONLY, static_cast<size_t>(m_wallsVector.size()));
    m_wallsBufferSize = m_wallsCount * sizeof(sWall);
    m_wallsBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_ONLY, m_wallsBufferSize);

    m_walls_collision_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("walls_collision"));
    m_cl_wrapper->enqueueWrite(m_wallsBuffer, m_wallsBufferSize, m_wallsVector.data(), CL_TRUE);
}

void CGPUBaseParticleSimulator::updateCollisions()
{
    cl_uint argCollision = 0;
    m_walls_collision_kernel->setArg(argCollision++, m_particlesBuffer);
    m_walls_collision_kernel->setArg(argCollision++, m_wallsBuffer);
    m_walls_collision_kernel->setArg(argCollision++, m_particlesCount);
    m_walls_collision_kernel->setArg(argCollision++, m_wallsCount);
    m_walls_collision_kernel->setArg(argCollision++, cl::Local(sizeof(sWall) * m_wallsCount));

    auto local = cl::NDRange(m_wallsCount);
    auto global = cl::NDRange(CLCommon::alignTo(m_maxParticlesCount, m_wallsCount));

    m_cl_wrapper->enqueueKernel(*m_walls_collision_kernel, global, local);
}

void CGPUBaseParticleSimulator::integrate()
{
    cl_uint arg = 0;
    m_integrationStepKernel->setArg(arg++, m_particlesBuffer);
    m_integrationStepKernel->setArg(arg++, m_particlesCount);
    m_integrationStepKernel->setArg(arg++, dt);

    auto processingEvent = m_cl_wrapper->enqueueKernel(*m_integrationStepKernel, cl::NDRange(m_maxParticlesCount));
    auto readEvent = m_cl_wrapper->enqueueRead(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_TRUE);

//    double copyDuration = CLCommon::getEventDuration(readEvent);
//    double processingDuration = CLCommon::getEventDuration(processingEvent);

    for (auto &particle : m_grid->getData()[0]) {
        particle->updatePosition();
        particle->updateVelocity();
    }

//    double gpuDuration = copyDuration + processingDuration;
//    double elapsed = cpuTimer.elapsed();
//    qDebug() << "gpu(copy" << copyDuration << "gpu(process)" << processingDuration << "cpu(with gpu)" << elapsed << "cpu(without gpu)" << elapsed - gpuDuration;
}
