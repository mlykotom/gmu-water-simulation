#include "CGPUBruteParticleSimulator.h"


CGPUBruteParticleSimulator::CGPUBruteParticleSimulator(CScene *scene, float boxSize, cl::Device device, SimulationScenario scenario, QObject *parent)
    : CGPUBaseParticleSimulator(scene, boxSize, std::move(device), scenario, parent)
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

    // density kernel
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));

    // forces kernel
    m_update_forces_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
}

double CGPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
    auto event = m_cl_wrapper->enqueueWrite(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_FALSE);
    return CLWrapper::getEventDuration(event);
}

double CGPUBruteParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, m_particlesBuffer);
    m_update_density_kernel->setArg(arg++, m_particlesCount);
    m_update_density_kernel->setArg(arg++, m_systemParams.poly6_constant);

    auto event = m_cl_wrapper->enqueueKernel(*m_update_density_kernel, cl::NDRange(m_particlesCount));
    return CLWrapper::getEventDuration(event);
}

double CGPUBruteParticleSimulator::updateForces()
{
    cl_uint arg = 0;
    m_update_forces_kernel->setArg(arg++, m_particlesBuffer);
    m_update_forces_kernel->setArg(arg++, m_particlesCount);
    m_update_forces_kernel->setArg(arg++, m_gravityCL);
    m_update_forces_kernel->setArg(arg++, m_systemParams.spiky_constant);
    m_update_forces_kernel->setArg(arg++, m_systemParams.viscosity_constant);

    auto processEvent = m_cl_wrapper->enqueueKernel(*m_update_forces_kernel, cl::NDRange(m_particlesCount));
    return CLWrapper::getEventDuration(processEvent);
}