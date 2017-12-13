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
    m_reduceKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("reduce"));
    m_downSweepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("down_sweep"));
    m_densityPresureStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_forceStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
    m_integrationStepKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));

    //prepare buffers
    m_gridVector.clear();
    m_gridVector.resize(m_grid->getCellCount(), 0);
    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());

    m_particlesSize = m_particlesCount * sizeof(CParticle::Physics);
    m_particlesBuffer = (m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_particlesSize));

    m_gridVectorSize = (cl_int)m_gridVector.size() * sizeof(cl_int);
    m_gridBuffer = (m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_gridVectorSize));;

    m_scanSize = (cl_int)m_gridVector.size() * sizeof(cl_int);
    m_scanBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_scanSize);

    m_indicesSize = m_sortedIndices.size() * sizeof(cl_int);
    m_indicesBuffer = (m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_indicesSize));

    //write buffers on GPU
    cl::Event writeEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);


    cl_float3 halfCellSize = { m_cellSize.x() / 2.0f, m_cellSize.y() / 2.0f, m_cellSize.z() / 2.0f };

    //setup kernels arguments
    cl_int arg = 0;

    m_updateParticlePositionsKernel->setArg(arg++, m_particlesBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, m_particlesCount);
    m_updateParticlePositionsKernel->setArg(arg++, m_gridSize);
    m_updateParticlePositionsKernel->setArg(arg++, halfCellSize);


    arg = 0;
    m_densityPresureStepKernel->setArg(arg++, m_particlesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_scanBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_indicesBuffer);
    m_densityPresureStepKernel->setArg(arg++, m_particlesCount);
    m_densityPresureStepKernel->setArg(arg++, m_gridSize);
    m_densityPresureStepKernel->setArg(arg++, m_systemParams.poly6_constant);

    arg = 0;
    m_forceStepKernel->setArg(arg++, m_particlesBuffer);
    m_forceStepKernel->setArg(arg++, m_scanBuffer);
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

std::vector<cl_int> CGPUParticleSimulator::scan(std::vector<cl_int> input)
{
    cl_int originalSize = (cl_int)input.size();
    //find nearest power of 2 to given count
    cl_int countAsPowerOfTwo = pow(2,ceil(log2(originalSize)));
   
    std::vector<cl_int> output(input.begin(),input.end());
    //resize and append 0s
    output.resize(countAsPowerOfTwo, 0);
    size_t outputSize = countAsPowerOfTwo * sizeof(cl_int);
    cl_int *output_array = output.data();

    cl_int err;

    //CLCommon::checkError(err, "inputBuffer creation");
    auto outputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, outputSize, output_array, &err);
    CLCommon::checkError(err, "outputBuffer creation");

    m_reduceKernel->setArg(0, outputBuffer);
    m_reduceKernel->setArg(1, countAsPowerOfTwo);
    //kernelReduce.setArg(2, cl::Local(sizeof(cl_int) * countAsPowerOfTwo));

    m_downSweepKernel->setArg(0, outputBuffer);
    m_downSweepKernel->setArg(1, countAsPowerOfTwo);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl_int localWokrgroupSize = 32;
    cl::NDRange local(localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(countAsPowerOfTwo, localWokrgroupSize));

    int levels = log2(countAsPowerOfTwo);
    int offset = 1;
    for (cl_int i = 0; i  < levels;  ++i)
    {
        m_reduceKernel->setArg(2, offset);
        m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_reduceKernel, 0, global, local, nullptr, &kernelEvent);
        offset <<= 1;
    }

    offset = countAsPowerOfTwo;
    for (cl_int i = 0; i < levels; ++i)
    {
        m_downSweepKernel->setArg(2, offset);
        m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_downSweepKernel, 0, global, local, nullptr, &kernelEvent);
        offset >>= 1;
    }

    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, outputSize, output_array, nullptr, &readEvent);
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    output.resize(originalSize);

    return output;
}

//TODO: TEST - DELETE
void CGPUParticleSimulator::test()
{
    //std::vector<cl_int> input;

    //input.push_back(1);
    //input.push_back(2);
    //input.push_back(3);
    //input.push_back(4);

    //input.push_back(5);
    //input.push_back(6);
    //input.push_back(7); 
    //input.push_back(8);

    //input.push_back(1);
    //input.push_back(2);
    //input.push_back(3);
    //input.push_back(4);

    //input.push_back(5);
    //input.push_back(6);
    //input.push_back(7);
    //input.push_back(8);

    //input.push_back(1);
    //input.push_back(2);
    //input.push_back(3);
    //input.push_back(4);

    //input.push_back(5);
    //input.push_back(6);
    //input.push_back(7);
    //input.push_back(8);

    //input.push_back(1);
    //input.push_back(2);
    //input.push_back(3);
    //input.push_back(4);

    //input.push_back(5);
    //input.push_back(6);
    //input.push_back(7);
    //input.push_back(8);


    //std::vector<cl_int> output =  scan(input);


    setupScene();
    updateGrid();
   


    //for (auto p : m_clParticles)
    //    qDebug() << p.cell_id;

    //qDebug() << "==============================";

    //for (cl_int i : m_sortedIndices)
    //    qDebug() << m_clParticles[i].cell_id;
    //qDebug() << "==============================";


    for (cl_int i : m_gridVector)
        qDebug() << i;

    qDebug() << "==============================";

    for (cl_int i : m_gridScan)
        qDebug() << i;
    qDebug() << "==============================";

    
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

void CGPUParticleSimulator::updateGrid()
{
    m_gridVector.clear();
    m_gridVector.resize(m_grid->getCellCount(), 0);

    cl_int *output_array = m_gridVector.data();
    CParticle::Physics *input_array = m_clParticles.data();

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(m_localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(m_particlesCount, m_localWokrgroupSize));
    cl::NDRange offset(0);

    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_gridBuffer, CL_FALSE, 0, m_gridVectorSize, output_array, nullptr, &writeEvent);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_updateParticlePositionsKernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_gridBuffer, CL_FALSE, 0, m_gridVectorSize, output_array, nullptr, &readEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, input_array, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    //scan grid
    //TODO: remove this parameters
    m_gridScan.clear();
    m_gridScan = scan(m_gridVector);

    //sort indices
    // initialize original index locations
    m_sortedIndices.clear();
    m_sortedIndices.resize(m_clParticles.size());
    std::iota(m_sortedIndices.begin(), m_sortedIndices.end(), 0);

    //// sort indexes, smallest cell index first
    sort(m_sortedIndices.begin(), m_sortedIndices.end(),
        [this](cl_int i1, cl_int i2) {return this->m_clParticles[i1].cell_id < this->m_clParticles[i2].cell_id; });

}

void CGPUParticleSimulator::updateDensityPressure()
{
    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(m_localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(m_particlesCount, m_localWokrgroupSize));

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    //m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_scanBuffer, CL_FALSE, 0, m_scanSize, m_gridScan.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_indicesBuffer, CL_FALSE, 0, m_indicesSize, m_sortedIndices.data(), nullptr, &writeEvent);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_densityPresureStepKernel, 0, global, local, nullptr, &kernelEvent);

  //  m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

}

void CGPUParticleSimulator::updateForces()
{

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(m_localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(m_particlesCount, m_localWokrgroupSize));

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    //m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);
    //m_cl_wrapper->getQueue().enqueueWriteBuffer(m_scanBuffer, true, 0, m_scanSize, m_gridScan.data(), nullptr, &writeEvent);
    //m_cl_wrapper->getQueue().enqueueWriteBuffer(m_indicesBuffer, true, 0, m_indicesSize, m_sortedIndices.data(), nullptr, &writeEvent);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_forceStepKernel, 0, global, local, nullptr, &kernelEvent);

    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

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


    cl::NDRange local(m_localWokrgroupSize);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(m_particlesCount, m_localWokrgroupSize));

    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_particlesBuffer, CL_FALSE, 0, m_particlesSize, m_clParticles.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integrationStepKernel, 0, global, local, nullptr, &kernelEvent);

    m_cl_wrapper->getQueue().enqueueReadBuffer(m_particlesBuffer, true, 0, m_particlesSize, m_clParticles.data(), nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    //all particles are in cell 0 on CPU
    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }

}

void CGPUParticleSimulator::setGravityVector(QVector3D newGravity)
{
    CBaseParticleSimulator::setGravityVector(newGravity);
    m_gravityCL = { gravity.x(), gravity.y(), gravity.z() };
}