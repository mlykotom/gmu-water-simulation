#include "CGPUBruteParticleSimulator.h"

CGPUBruteParticleSimulator::CGPUBruteParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent)
    : CGPUBaseParticleSimulator(scene, boxSize, device, parent)
{
    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/sph_common.cl",
            APP_RESOURCES"/kernels/sph_brute.cl"
        }
    );
}

void CGPUBruteParticleSimulator::setupKernels()
{
    CGPUBaseParticleSimulator::setupKernels();

    m_integration_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_update_forces_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
}

void CGPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
    cl::Event writeEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, CL_TRUE, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);
}

void CGPUBruteParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, m_particlesBuffer);
    m_update_density_kernel->setArg(arg++, m_particlesCount);
    m_update_density_kernel->setArg(arg++, m_systemParams.poly6_constant);

    cl::Event kernelEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_density_kernel, 0, global, local, nullptr, &kernelEvent);
}

void CGPUBruteParticleSimulator::updateForces()
{
    cl_uint arg = 0;
    m_update_forces_kernel->setArg(arg++, m_particlesBuffer);
    m_update_forces_kernel->setArg(arg++, m_particlesCount);
    m_update_forces_kernel->setArg(arg++, m_gravityCL);
    m_update_forces_kernel->setArg(arg++, m_systemParams.spiky_constant);
    m_update_forces_kernel->setArg(arg++, m_systemParams.viscosity_constant);

    cl::Event kernelEvent, readEvent, kernelCollisionEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_forces_kernel, 0, global, local, nullptr, &kernelEvent);

    // collision forces
    cl::NDRange collisionLocal = cl::NullRange;
    cl::NDRange collisionGlobal(m_particlesCount);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_walls_collision_kernel, 0, collisionGlobal, collisionLocal, nullptr, &kernelCollisionEvent);
}

void CGPUBruteParticleSimulator::integrate()
{
    cl_uint arg = 0;
    m_integration_kernel->setArg(arg++, m_particlesBuffer);
    m_integration_kernel->setArg(arg++, m_particlesCount);
    m_integration_kernel->setArg(arg++, dt);

    cl::Event kernelEvent, readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integration_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}
