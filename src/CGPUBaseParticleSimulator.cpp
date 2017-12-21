#include "CGPUBaseParticleSimulator.h"

CGPUBaseParticleSimulator::CGPUBaseParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent)
    : CBaseParticleSimulator(scene, boxSize, parent),
      m_gravityCL({gravity.x(), gravity.y(), gravity.z()})
{
    m_cl_wrapper = new CLWrapper(device);
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
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;

    unsigned int calculatedCount = (unsigned) (ceil(m_boxSize.z() / halfParticle) * ceil(m_boxSize.y() / halfParticle) * ceil(m_boxSize.x() / 4 / halfParticle));
    m_clParticles.reserve(calculatedCount);

    QVector3D offset = -m_boxSize / 2.0f;

    for (float y = 0; y < m_boxSize.y(); y += halfParticle) {
        for (float x = 0; x < m_boxSize.x() / 4.0; x += halfParticle) {
            for (float z = 0; z < m_boxSize.z(); z += halfParticle) {

                m_clParticles.emplace_back(x + offset.x(), y + offset.y(), z + offset.z(), m_particlesCount);
                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
                particle->m_physics = &m_clParticles.back();

                firstGridCell.push_back(particle);
                m_particlesCount++;
            }
        }
    }

    assert(calculatedCount == m_particlesCount);

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

    cl::Event writeCollisionEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_wallsBuffer, CL_TRUE, 0, m_wallsBufferSize, m_wallsVector.data(), nullptr, &writeCollisionEvent);
}
void CGPUBaseParticleSimulator::integrate()
{
    cl::Event kernelEvent, readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integrationStepKernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}
