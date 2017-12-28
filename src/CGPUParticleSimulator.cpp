#include "CGPUParticleSimulator.h"

CGPUParticleSimulator::CGPUParticleSimulator(CScene *scene, float boxSize, cl::Device device, SimulationScenario scenario, QObject *parent)
    : CGPUBaseParticleSimulator(scene, boxSize, device, scenario, parent),
      m_halfBoxSize({m_boxSize.x() / 2.0f, m_boxSize.y() / 2.0f, m_boxSize.z() / 2.0f}),
      m_gridSize({m_grid->xRes(), m_grid->yRes(), m_grid->zRes()})
{
    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/sph_common.cl",
            APP_RESOURCES"/kernels/sph_grid.cl"
        }
    );
}

void CGPUParticleSimulator::setupKernels()
{
    CGPUBaseParticleSimulator::setupKernels();

    //create kernels
    m_updateParticlePositionsKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("update_grid_positions"));
    m_scanLocalKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("scan_local"));
    m_incrementKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("increment_local_scans"));
    m_densityPresureStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_forceStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));

    //prepare buffers
    m_gridVector.clear();
    //grid vector and grid scan needs to be power of two so that the blelloch scan can work
    m_gridCountToPowerOfTwo = qNextPowerOfTwo(m_grid->getCellCount());
    m_gridVector.resize(m_gridCountToPowerOfTwo, 0);

    m_sortedIndices.clear();
    m_sortedIndices.resize(m_maxParticlesCount);

    m_gridVectorSize = (cl_int) m_gridVector.size() * sizeof(cl_int);
    m_gridBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_gridVectorSize);

    m_indicesSize = m_sortedIndices.size() * sizeof(cl_int);
    m_indicesBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_indicesSize);

    m_elementsProcessedInOneGroup = qNextPowerOfTwo((cl_int) ceil(qSqrt(m_gridCountToPowerOfTwo)));
    //we need only half the threads of processed elements
    m_localScanWokrgroupSize = m_elementsProcessedInOneGroup / 2;
    m_scanLocal = cl::NDRange(m_localScanWokrgroupSize);
    //we need only half the threads of the input count
    m_scanGlobal = cl::NDRange(CLCommon::alignTo(m_gridCountToPowerOfTwo / 2, m_localScanWokrgroupSize));

    m_sumsCount = m_scanGlobal[0] / m_scanLocal[0];
    size_t m_sumsSize = m_sumsCount * sizeof(cl_int);
    m_sums.resize(m_sumsCount, 0);
    m_sumsGlobal = cl::NDRange(CLCommon::alignTo(m_sumsCount, m_localScanWokrgroupSize));

    m_scanSumsBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_sumsSize);
}

double CGPUParticleSimulator::updateGrid()
{
    double totalDuration = 0;

    auto eventWrite = m_cl_wrapper->enqueueWrite(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_TRUE);

    m_gridVector.clear();
    m_gridVector.resize(m_gridCountToPowerOfTwo, 0);

    cl_uint arg = 0;
    m_updateParticlePositionsKernel->setArg(arg++, m_particlesBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_particlesCount);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridSize);
    m_updateParticlePositionsKernel->setArg(arg++, m_halfBoxSize);

    auto eventWrite2 = m_cl_wrapper->enqueueWrite(m_gridBuffer, m_gridVectorSize, m_gridVector.data(), CL_TRUE);

    auto global = cl::NDRange(m_maxParticlesCount);
    auto eventProcess = m_cl_wrapper->enqueueKernel(*m_updateParticlePositionsKernel, global);

    //scan grid
    totalDuration += scanGrid();

    //sort indices
    totalDuration += sortIndices();

    totalDuration += CLCommon::getEventDuration({eventWrite, eventWrite2, eventProcess});
    return totalDuration;
}

double CGPUParticleSimulator::scanGrid()
{
    //scan input array
    m_scanLocalKernel->setArg(0, m_gridBuffer);
    m_scanLocalKernel->setArg(1, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(2, m_gridCountToPowerOfTwo);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * m_elementsProcessedInOneGroup));

    auto eventScan1 = m_cl_wrapper->enqueueKernel(*m_scanLocalKernel, m_scanGlobal, m_scanLocal);

    //scan sums
    m_scanLocalKernel->setArg(0, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(1, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(2, m_sumsCount);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * m_elementsProcessedInOneGroup));

    auto eventScan2 = m_cl_wrapper->enqueueKernel(*m_scanLocalKernel, m_sumsGlobal, m_scanLocal);

    //increment input scan
    m_incrementKernel->setArg(0, m_gridBuffer);
    m_incrementKernel->setArg(1, m_scanSumsBuffer);
    m_incrementKernel->setArg(2, m_gridCountToPowerOfTwo);

    auto eventIncrement = m_cl_wrapper->enqueueKernel(*m_incrementKernel, m_scanGlobal, m_scanLocal);

    return CLCommon::getEventDuration({eventScan1, eventScan2, eventIncrement});
}

double CGPUParticleSimulator::sortIndices()
{
    auto readEvent = m_cl_wrapper->enqueueRead(m_particlesBuffer, m_particlesSize, m_clParticles.data(), CL_TRUE);

    QElapsedTimer cpuTimer;
    cpuTimer.start();

    // initialize original index locations
    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());
    std::iota(m_sortedIndices.begin(), m_sortedIndices.end(), 0);

    // sort indexes, smallest cell index firstOp
    sort(m_sortedIndices.begin(), m_sortedIndices.end(),
         [this](cl_int i1, cl_int i2)
         { return this->m_clParticles[i1].cell_id < this->m_clParticles[i2].cell_id; }
    );

    double cpuTime = cpuTimer.elapsed();

    auto writeEvent = m_cl_wrapper->enqueueWrite(m_indicesBuffer, m_indicesSize, m_sortedIndices.data(), CL_FALSE);

    return cpuTime + CLCommon::getEventDuration({readEvent, writeEvent});
}

double CGPUParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_densityPresureStepKernel->setArg(arg++, m_particlesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_gridBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_indicesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_particlesCount);
    m_densityPresureStepKernel->setArg(arg++, m_gridSize);
    m_densityPresureStepKernel->setArg(arg++, m_systemParams.poly6_constant);

    auto global = cl::NDRange(m_maxParticlesCount);
    auto event = m_cl_wrapper->enqueueKernel(*m_densityPresureStepKernel, global);

    return CLCommon::getEventDuration(event);
}

double CGPUParticleSimulator::updateForces()
{
    cl_uint arg = 0;
    m_forceStepKernel->setArg(arg++, m_particlesBuffer);
    m_forceStepKernel->setArg(arg++, m_gridBuffer);
    m_forceStepKernel->setArg(arg++, m_indicesBuffer);
    m_forceStepKernel->setArg(arg++, m_particlesCount);
    m_forceStepKernel->setArg(arg++, m_gridSize);
    m_forceStepKernel->setArg(arg++, m_gravityCL);
    m_forceStepKernel->setArg(arg++, m_systemParams.spiky_constant);
    m_forceStepKernel->setArg(arg++, m_systemParams.viscosity_constant);

    auto global = cl::NDRange(m_maxParticlesCount);
    auto event = m_cl_wrapper->enqueueKernel(*m_forceStepKernel, global);

    return CLCommon::getEventDuration(event);
}