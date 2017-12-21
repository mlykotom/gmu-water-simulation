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

    m_global = cl::NDRange(m_particlesCount);

    // density kernel
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));

    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, m_particlesBuffer);
    m_update_density_kernel->setArg(arg++, m_particlesCount);
    m_update_density_kernel->setArg(arg++, m_systemParams.poly6_constant);

    // forces kernel
    m_update_forces_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));

    arg = 0;
    m_update_forces_kernel->setArg(arg++, m_particlesBuffer);
    m_update_forces_kernel->setArg(arg++, m_particlesCount);
    m_update_forces_kernel->setArg(arg++, m_gravityCL);
    m_update_forces_kernel->setArg(arg++, m_systemParams.spiky_constant);
    m_update_forces_kernel->setArg(arg++, m_systemParams.viscosity_constant);
}

void CGPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
    m_cl_wrapper->enqueueWrite(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_TRUE);
}

void CGPUBruteParticleSimulator::updateDensityPressure()
{
    m_cl_wrapper->enqueueKernel(*m_update_density_kernel, m_global);
}

void CGPUBruteParticleSimulator::updateForces()
{
    m_update_forces_kernel->setArg(2, m_gravityCL);  // WARNING: gravityCL must be the same as in first setup!

    m_cl_wrapper->enqueueKernel(*m_update_forces_kernel, m_global);
    m_cl_wrapper->enqueueKernel(*m_walls_collision_kernel, m_global);
}