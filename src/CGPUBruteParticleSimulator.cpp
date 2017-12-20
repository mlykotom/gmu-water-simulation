#include "CGPUBruteParticleSimulator.h"

CCPUBruteParticleSimulator::CCPUBruteParticleSimulator(CScene *scene, QObject *parent)
    : CGPUBaseParticleSimulator(scene, parent)
{
    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/sph_common.cl",
            APP_RESOURCES"/kernels/sph_brute.cl"
        }
    );
}

void CCPUBruteParticleSimulator::setupKernels()
{
    m_dataBufferSize = m_particlesCount * sizeof(CParticle::Physics);
    m_outputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_dataBufferSize);

    m_integration_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_update_forces_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
    cl::Event writeEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_outputBuffer, CL_TRUE, 0, m_dataBufferSize, m_clParticles.data(), nullptr, &writeEvent);
}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, m_outputBuffer);
    m_update_density_kernel->setArg(arg++, m_particlesCount);
    m_update_density_kernel->setArg(arg++, m_systemParams.poly6_constant);

    cl::Event kernelEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_density_kernel, 0, global, local, nullptr, &kernelEvent);
}

void CCPUBruteParticleSimulator::updateForces()
{
    cl_uint arg = 0;
    m_update_forces_kernel->setArg(arg++, m_outputBuffer);
    m_update_forces_kernel->setArg(arg++, m_particlesCount);
    m_update_forces_kernel->setArg(arg++, m_gravityCL);
    m_update_forces_kernel->setArg(arg++, m_systemParams.spiky_constant);
    m_update_forces_kernel->setArg(arg++, m_systemParams.viscosity_constant);

    cl::Event kernelEvent, readEvent, writeEventAfterCollision;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_forces_kernel, 0, global, local, nullptr, &kernelEvent);

    // need to read buffer and wait until finish because of collision computations
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_outputBuffer, CL_TRUE, 0, m_dataBufferSize, m_clParticles.data(), nullptr, &readEvent);
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    // collision force
    for (int i = 0; i < m_particlesCount; ++i) {
        CParticle::Physics &particleCL = m_clParticles[i];
        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);

        m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particleCL);
        //QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);
        //particleCL.acceleration = {particleCL.acceleration.x + f_collision.x(), particleCL.acceleration.y + f_collision.y(), particleCL.acceleration.z + f_collision.z()};
    }

    // need to write buffer because previous step has
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_outputBuffer, CL_FALSE, 0, m_dataBufferSize, m_clParticles.data(), nullptr, &writeEventAfterCollision);
}

void CCPUBruteParticleSimulator::integrate()
{
    cl_uint arg = 0;
    m_integration_kernel->setArg(arg++, m_outputBuffer);
    m_integration_kernel->setArg(arg++, m_particlesCount);
    m_integration_kernel->setArg(arg++, dt);

    cl::Event kernelEvent, readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integration_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_outputBuffer, CL_FALSE, 0, m_dataBufferSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}
