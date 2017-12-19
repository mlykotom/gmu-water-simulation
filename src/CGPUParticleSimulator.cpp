#include "CGPUParticleSimulator.h"

CGPUParticleSimulator::CGPUParticleSimulator(CScene *scene, QObject *parent)
    : CGPUBaseParticleSimulator(scene, parent),
      m_localWokrgroupSize(64)
{
    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/sph_common.cl",
            APP_RESOURCES"/kernels/sph_grid.cl"
        }
    );

    m_gridSize = {m_grid->xRes(), m_grid->yRes(), m_grid->zRes()};
}

void CGPUParticleSimulator::setupKernels()
{
    //create kernels
    m_updateParticlePositionsKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("update_grid_positions"));
    m_scanLocalKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("scan_local"));
    m_incrementKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("increment_local_scans"));
    m_densityPresureStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_forceStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
    m_integrationStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));


    m_local = cl::NDRange(m_localWokrgroupSize);
    m_global = cl::NDRange(CLCommon::alignTo(m_particlesCount, m_localWokrgroupSize));

    //prepare buffers
    m_gridVector.clear();
    //grid vector and grid scan needs to be power of two so that the blelloch scan can work
    m_gridCountToPowerOfTwo = qNextPowerOfTwo(m_grid->getCellCount());
    m_gridVector.resize(m_gridCountToPowerOfTwo, 0);

    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());

    m_particlesSize = m_particlesCount * sizeof(CParticle::Physics);
    m_particlesBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_particlesSize);

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

    //write buffers on GPU
    cl::Event writeEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);


    m_halfBoxSize = {m_boxSize.x() / 2.0f, m_boxSize.y() / 2.0f, m_boxSize.z() / 2.0f};
    //setup kernels arguments
    cl_int arg = 0;

    m_updateParticlePositionsKernel->setArg(arg++, m_particlesBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_particlesCount);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridSize);
    m_updateParticlePositionsKernel->setArg(arg++, m_halfBoxSize);


    arg = 0;
    m_densityPresureStepKernel->setArg(arg++, m_particlesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_gridBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_indicesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_particlesCount);
    m_densityPresureStepKernel->setArg(arg++, m_gridSize);
    m_densityPresureStepKernel->setArg(arg++, m_systemParams.poly6_constant);

    arg = 0;
    m_forceStepKernel->setArg(arg++, m_particlesBuffer);
    m_forceStepKernel->setArg(arg++, m_gridBuffer);
    m_forceStepKernel->setArg(arg++, m_indicesBuffer);
    m_forceStepKernel->setArg(arg++, m_particlesCount);
    m_forceStepKernel->setArg(arg++, m_gridSize);
    m_forceStepKernel->setArg(arg++, m_gravityCL);
    m_forceStepKernel->setArg(arg++, m_systemParams.spiky_constant);
    m_forceStepKernel->setArg(arg++, m_systemParams.viscosity_constant);

    arg = 0;
    m_integrationStepKernel->setArg(arg++, m_particlesBuffer);
    m_integrationStepKernel->setArg(arg++, m_particlesCount);
    m_integrationStepKernel->setArg(arg++, dt);

}

void CGPUParticleSimulator::scanGrid()
{
    cl::Event kernelEvent;

    //scan input array
    m_scanLocalKernel->setArg(0, m_gridBuffer);
    m_scanLocalKernel->setArg(1, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(2, m_gridCountToPowerOfTwo);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * m_elementsProcessedInOneGroup));

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_scanLocalKernel, 0, m_scanGlobal, m_scanLocal, nullptr, &kernelEvent);

    //scan sums
    m_scanLocalKernel->setArg(0, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(1, m_scanSumsBuffer);
    m_scanLocalKernel->setArg(2, m_sumsCount);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * m_elementsProcessedInOneGroup));

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_scanLocalKernel, 0, m_sumsGlobal, m_scanLocal, nullptr, &kernelEvent);

    //increment input scan
    m_incrementKernel->setArg(0, m_gridBuffer);
    m_incrementKernel->setArg(1, m_scanSumsBuffer);
    m_incrementKernel->setArg(2, m_gridCountToPowerOfTwo);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_incrementKernel, 0, m_scanGlobal, m_scanLocal, nullptr, &kernelEvent);

}

void CGPUParticleSimulator::updateGrid()
{
    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    m_gridVector.clear();
    m_gridVector.resize(m_gridCountToPowerOfTwo, 0);

    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_gridBuffer, CL_FALSE, 0, m_gridVectorSize, m_gridVector.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_updateParticlePositionsKernel, 0, m_global, m_local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_TRUE, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

//    m_cl_wrapper->getQueue().enqueueReadBuffer(m_gridBuffer, CL_TRUE, 0, m_gridVectorSize, m_gridVector.data(), nullptr, &readEvent);
//    int l_max = -1;
//    for (int i = 0; i < m_gridVector.size(); ++i)
//    {
//        l_max = std::max(l_max, m_gridVector.at(i));
//    }
//
//    qDebug() << l_max;


    //scan grid
    scanGrid();




    //sort indices
    // initialize original index locations
    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());
    std::iota(m_sortedIndices.begin(), m_sortedIndices.end(), 0);

    // sort indexes, smallest cell index first
    sort(m_sortedIndices.begin(), m_sortedIndices.end(),
         [this](cl_int i1, cl_int i2)
         { return this->m_clParticles[i1].cell_id < this->m_clParticles[i2].cell_id; });

}

void CGPUParticleSimulator::updateDensityPressure()
{
    cl::Event writeEvent;
    cl::Event kernelEvent;

    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_indicesBuffer, CL_FALSE, 0, m_indicesSize, m_sortedIndices.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_densityPresureStepKernel, 0, m_global, m_local, nullptr, &kernelEvent);
}

void CGPUParticleSimulator::updateForces()
{
    m_forceStepKernel->setArg(5, m_gravityCL);  // WARNING: gravityCL must be on 5th position

    cl::Event kernelEvent, readEvent, writeEventAfterCollision;

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_forceStepKernel, 0, m_global, m_local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_TRUE, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    // collision force
#pragma omp parallel for
    for (int i = 0; i < m_particlesCount; ++i) {
        CParticle::Physics &particleCL = m_clParticles[i];
        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);

        m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particleCL);
        //QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);
        //particleCL.acceleration = {particleCL.acceleration.x + f_collision.x(), particleCL.acceleration.y + f_collision.y(), particleCL.acceleration.z + f_collision.z()};
    }

    // need to write buffer because previous step has
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEventAfterCollision);
}

void CGPUParticleSimulator::integrate()
{
    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    // TODO here m_local should be nullRange (because we don't use local memory)
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integrationStepKernel, 0, m_global, m_local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    //all particles are in cell 0 on CPU
    std::vector<CParticle *> &particles = m_grid->getData()[0];

    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}