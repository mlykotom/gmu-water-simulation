#include <include/CLPlatforms.h>
#include "CGPUParticleSimulator.h"

// TODO update this shit! yeah
#include <CL/cl.hpp>

CGPUParticleSimulator::CGPUParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent),
    m_localWokrgroupSize(64)
{
    CLPlatforms::printInfoAll();

    cl::Device clDevice = CLPlatforms::getBestGPU();
    m_cl_wrapper = new CLWrapper(clDevice);
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());

    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/scan.cl"
        }
    );


    m_gridSize = { m_grid->xRes(), m_grid->yRes(), m_grid->zRes() };
    m_gravityCL = { gravity.x(), gravity.y(), gravity.z() };


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

    m_gridVectorSize = (cl_int)m_gridVector.size() * sizeof(cl_int);
    m_gridBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_gridVectorSize);

    m_indicesSize = m_sortedIndices.size() * sizeof(cl_int);
    m_indicesBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_indicesSize);


    m_elementsProcessedInOneGroup = qNextPowerOfTwo((cl_int)ceil(qSqrt(m_gridCountToPowerOfTwo)));
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


    m_halfCellSize = { m_cellSize.x() / 2.0f, m_cellSize.y() / 2.0f, m_cellSize.z() / 2.0f };

    //setup kernels arguments
    cl_int arg = 0;

    m_updateParticlePositionsKernel->setArg(arg++, m_particlesBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_particlesCount);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridSize);
    m_updateParticlePositionsKernel->setArg(arg++, m_halfCellSize);


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

void CGPUParticleSimulator::scan(std::vector<cl_int> input)
{

    m_scanLocalKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("scan_local"));
    m_incrementKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("increment_local_scans"));

    cl_int originalSize = (cl_int)input.size();
    //find nearest power of 2 to given count
    cl_int countAsPowerOfTwo = pow(2,ceil(log2(originalSize)));

    //cl_int elementsProcessedInOneGroup = CLCommon::alignTo(qNextPowerOfTwo((cl_int)ceil(qSqrt(countAsPowerOfTwo))),8);
    cl_int elementsProcessedInOneGroup = qNextPowerOfTwo((cl_int)ceil(qSqrt(countAsPowerOfTwo)));
    //we need only half the threads of processed elements
    cl_int localWokrgroupSize = elementsProcessedInOneGroup/2;
    cl::NDRange local(localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(countAsPowerOfTwo/2, localWokrgroupSize));

    //resize input array, this algorithm works only in input is power of 2
    input.resize(countAsPowerOfTwo,0);
    size_t inputSize = countAsPowerOfTwo * sizeof(cl_int);

    cl_int sumCount = global[0] / local[0];
    size_t sumSize = sumCount * sizeof(cl_int);
    std::vector<cl_int> sums;
    sums.resize(sumCount,0);

    cl::NDRange sumsGlobal(CLCommon::alignTo(sumCount, localWokrgroupSize));

    cl_int err;

    //CLCommon::checkError(err, "inputBuffer creation");
    auto inputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, inputSize, input.data(), &err);
    CLCommon::checkError(err, "outputBuffer creation");
    auto sumsBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE , sumSize, nullptr, &err);
    CLCommon::checkError(err, "buffer creation");

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    //scan input array
    m_scanLocalKernel->setArg(0, inputBuffer);
    m_scanLocalKernel->setArg(1, sumsBuffer);
    m_scanLocalKernel->setArg(2, countAsPowerOfTwo);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * elementsProcessedInOneGroup));

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_scanLocalKernel, 0, global, local, nullptr, &kernelEvent);

    //read input buffer
    m_cl_wrapper->getQueue().enqueueReadBuffer(inputBuffer, true, 0, inputSize, input.data(), nullptr, &readEvent);
    ////read sums buffer
    m_cl_wrapper->getQueue().enqueueReadBuffer(sumsBuffer, true, 0, sumSize, sums.data(), nullptr, &readEvent);

    //scan sums
    m_scanLocalKernel->setArg(0, sumsBuffer);
    m_scanLocalKernel->setArg(1, sumsBuffer);
    m_scanLocalKernel->setArg(2, sumCount);
    m_scanLocalKernel->setArg(3, cl::Local(sizeof(cl_int) * elementsProcessedInOneGroup));

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_scanLocalKernel, 0, sumsGlobal, local, nullptr, &kernelEvent);

    ////read sums buffer
    m_cl_wrapper->getQueue().enqueueReadBuffer(sumsBuffer, true, 0, sumSize, sums.data(), nullptr, &readEvent);

    //increment input scan
    m_incrementKernel->setArg(0, inputBuffer);
    m_incrementKernel->setArg(1, sumsBuffer);
    m_incrementKernel->setArg(2, countAsPowerOfTwo);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_incrementKernel, 0, global, local, nullptr, &kernelEvent);

    //read input buffer
    m_cl_wrapper->getQueue().enqueueReadBuffer(inputBuffer, true, 0, inputSize, input.data(), nullptr, &readEvent);

    //read sums buffer
    m_cl_wrapper->getQueue().enqueueReadBuffer(sumsBuffer, true, 0, sumSize, sums.data(), nullptr, &readEvent);

    
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

 //   output.resize(originalSize);
    return;
}


//TODO: TEST - DELETE
void CGPUParticleSimulator::test()
{
    
    std::vector<cl_int> input;

    input.push_back(1);
    input.push_back(2);
    input.push_back(3);
    input.push_back(4);

    input.push_back(5);
    input.push_back(6);
    input.push_back(7); 
    input.push_back(8);

    input.push_back(1);
    input.push_back(2);
    input.push_back(3);
    input.push_back(4);

    input.push_back(5);
    input.push_back(6);
    input.push_back(7);
    input.push_back(8);

    input.push_back(1);
    input.push_back(2);
    input.push_back(3);
    input.push_back(4);

    input.push_back(5);
    input.push_back(6);
    input.push_back(7);
    input.push_back(8);

    input.push_back(1);
    input.push_back(2);
    input.push_back(3);
    input.push_back(4);

    input.push_back(5);
    input.push_back(6);
    input.push_back(7);
    input.push_back(8);


     scan(input);

}


void CGPUParticleSimulator::setupScene()
{
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;

    int calculatedCount = (int)(ceil(m_boxSize.z() / halfParticle) * ceil(m_boxSize.y() / halfParticle) * ceil(m_boxSize.x() / 4 / halfParticle));
    //m_device_data = new CParticle::Physics[calculatedCount];
    m_clParticles.reserve(calculatedCount);

    QVector3D offset = -m_boxSize / 2.0f;

    for (float y = 0; y < m_boxSize.y(); y += halfParticle) {
        for (float x = 0; x < m_boxSize.x() / 4.0; x += halfParticle) {
            for (float z = 0; z < m_boxSize.z(); z += halfParticle) {

                CParticle::Physics p;
                p.id = m_particlesCount;
                p.position = { x + offset.x(), y + offset.y(), z + offset.z() };
                p.velocity = { 0, 0, 0 };
                p.acceleration = { 0, 0, 0 };
                p.density = 0.0;
                p.pressure = 0;
                //m_device_data[m_particlesCount] = p;

                m_clParticles.push_back(p);

                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
               // particle->m_physics = &m_device_data[m_particlesCount];
                particle->m_physics = &m_clParticles.back();

                firstGridCell.push_back(particle);
                m_particlesCount++;

            }
        }
    }

    setupKernels();

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << m_particlesCount << "particles";
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


    //scan grid
    scanGrid();

    //m_cl_wrapper->getQueue().enqueueReadBuffer(m_gridBuffer, CL_TRUE, 0, m_gridVectorSize, m_gridVector.data(), nullptr, &readEvent);

    //sort indices
    // initialize original index locations
    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());
    std::iota(m_sortedIndices.begin(), m_sortedIndices.end(), 0);

    // sort indexes, smallest cell index first
    sort(m_sortedIndices.begin(), m_sortedIndices.end(),
        [this](cl_int i1, cl_int i2) {return this->m_clParticles[i1].cell_id < this->m_clParticles[i2].cell_id; });

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
    cl::Event kernelEvent;
    cl::Event readEvent;

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_forceStepKernel, 0, m_global, m_local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    // collision force
#pragma omp parallel for
    for (int i = 0; i < m_particlesCount; ++i) 
    {
        CParticle::Physics &particleCL = m_clParticles[i];
        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);

        m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particleCL);
        //QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);
        //particleCL.acceleration = {particleCL.acceleration.x + f_collision.x(), particleCL.acceleration.y + f_collision.y(), particleCL.acceleration.z + f_collision.z()};
    }
}

void CGPUParticleSimulator::test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity)
{
    newPosition = position + (velocity * dt) + acceleration * dt * dt;
    newVelocity = (newPosition - position) / dt;
}


void CGPUParticleSimulator::integrate()
{
    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integrationStepKernel, 0, m_global, m_local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

   // CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    //all particles are in cell 0 on CPU
    std::vector<CParticle *> &particles = m_grid->getData()[0];

    //pragma does not work here for some reason...
    for (int i = 0; i < particles.size(); ++i)
    {
        particles[i]->updatePosition();
        particles[i]->updateVelocity();
    }

    //for (auto &particle : particles) {
    //    particle->updatePosition();
    //    particle->updateVelocity();
    //}
}

void CGPUParticleSimulator::setGravityVector(QVector3D newGravity)
{
    CBaseParticleSimulator::setGravityVector(newGravity);
    m_gravityCL = { gravity.x(), gravity.y(), gravity.z() };
}