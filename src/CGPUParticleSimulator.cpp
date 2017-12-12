#include <include/CLPlatforms.h>
#include "CGPUParticleSimulator.h"

// TODO update this shit! yeah
#include <CL/cl.hpp>

CGPUParticleSimulator::CGPUParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
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

    m_updateParticlePositionsKernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("update_grid_positions"));
    m_reduceKernel                  = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("reduce"));
    m_downSweepKernel               = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("down_sweep"));
    m_densityPresureStepKernel      = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));

}

std::vector<cl_int> CGPUParticleSimulator::scan(std::vector<cl_int> input)
{
    cl_int originalSize = input.size();
    //find nearest power of 2 to given count
    cl_int countAsPowerOfTwo = pow(2,ceil(log2(originalSize)));
   
    std::vector<cl_int> output(input.begin(),input.end());
    //resize and append 0s
    output.resize(countAsPowerOfTwo, 0);
    size_t outputSize = countAsPowerOfTwo * sizeof(cl_int);
    cl_int *output_array = output.data();

   // cl::Kernel kernel = cl::Kernel(m_cl_wrapper->getKernel("blelloch_scan"));
    //cl::Kernel kernelReduce = cl::Kernel(m_cl_wrapper->getKernel("reduce"));
    //cl::Kernel kernelDownSweep = cl::Kernel(m_cl_wrapper->getKernel("down_sweep"));

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



    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << m_particlesCount << "particles";
}

void CGPUParticleSimulator::updateGrid()
{
    cl_int pariclesCount = m_particlesCount;
    size_t particlesSize = pariclesCount * sizeof(CParticle::Physics);

    cl_float3 halfCellSize = { m_cellSize.x() / 2.0, m_cellSize.y() / 2.0, m_cellSize.z() / 2.0 };
    cl_int3 gridSize = { m_grid->xRes(),m_grid->yRes() ,m_grid->zRes() };

   // std::vector<cl_int> output;
    cl_int outputCount = m_grid->getCellCount();
    m_gridVector.clear();
    m_gridVector.resize(outputCount,0);
    cl_int *output_array = m_gridVector.data();
    size_t outputSize = outputCount * sizeof(cl_int);

    CParticle::Physics *input_array = m_clParticles.data();

    cl_int err;

    auto inputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, particlesSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    auto outputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, outputSize, output_array, &err);
    CLCommon::checkError(err, "outputBuffer creation");



    cl_int arg = 0;
    m_updateParticlePositionsKernel->setArg(arg++, inputBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, outputBuffer);
    m_updateParticlePositionsKernel->setArg(arg++, (cl_int)pariclesCount);
    m_updateParticlePositionsKernel->setArg(arg++, gridSize);
    m_updateParticlePositionsKernel->setArg(arg++, halfCellSize);
    //m_updateParticlePositionsKernel->setArg(arg++, (cl_float)CParticle::h);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(16);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(pariclesCount, 16));
    cl::NDRange offset(0);

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(inputBuffer, true, 0, particlesSize, input_array, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_updateParticlePositionsKernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, outputSize, output_array, nullptr, &readEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(inputBuffer, true, 0, particlesSize, input_array, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    //scan grid
    //TODO: remove this parameters
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
    size_t particlesSize = m_particlesCount * sizeof(CParticle::Physics);
    size_t scanSize = m_gridScan.size() * sizeof(int);
    size_t indicesSize = m_sortedIndices.size() * sizeof(int);

    cl_int3 gridSize = { m_grid->xRes(), m_grid->yRes(), m_grid->zRes() };



    CParticle::Physics *particles_array = m_clParticles.data();
    int *scan_array = m_gridScan.data();
    int *indices_array = m_sortedIndices.data();

    cl_int err;

    auto particlesBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, particlesSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    auto scanBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, scanSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    auto indicesBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, indicesSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");



    cl_int arg = 0;
    m_densityPresureStepKernel->setArg(arg++, particlesBuffer);
    m_densityPresureStepKernel->setArg(arg++, scanBuffer);
    m_densityPresureStepKernel->setArg(arg++, indicesBuffer);

    m_densityPresureStepKernel->setArg(arg++, (cl_int)m_particlesCount);
    m_densityPresureStepKernel->setArg(arg++, gridSize);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(16);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(m_particlesCount, 16));
    cl::NDRange offset(0);

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(particlesBuffer, true, 0, particlesSize, particles_array, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueWriteBuffer(scanBuffer, true, 0, scanSize, scan_array, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueWriteBuffer(indicesBuffer, true, 0, indicesSize, indices_array, nullptr, &writeEvent);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_densityPresureStepKernel, 0, global, local, nullptr, &kernelEvent);

    m_cl_wrapper->getQueue().enqueueReadBuffer(particlesBuffer, true, 0, particlesSize, particles_array, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");


    //for (int x = 0; x < m_grid->xRes(); x++) {
    //    for (int y = 0; y < m_grid->yRes(); y++) {
    //        for (int z = 0; z < m_grid->zRes(); z++) {

    //            auto &particles = m_grid->at(x, y, z);
    //            for (auto &particle : particles) {

    //                particle->density() = 0.0;

    //                // neighbors
    //                for (int offsetX = -1; offsetX <= 1; offsetX++) {
    //                    if (x + offsetX < 0) continue;
    //                    if (x + offsetX >= m_grid->xRes()) break;

    //                    for (int offsetY = -1; offsetY <= 1; offsetY++) {
    //                        if (y + offsetY < 0) continue;
    //                        if (y + offsetY >= m_grid->yRes()) break;

    //                        for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
    //                            if (z + offsetZ < 0) continue;
    //                            if (z + offsetZ >= m_grid->zRes()) break;

    //                            auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
    //                            for (auto &neighbor : neighborGridCellParticles) {
    //                                double radiusSquared = particle->diffPosition(neighbor).lengthSquared();

    //                                if (radiusSquared <= CParticle::h * CParticle::h) {
    //                                    particle->density() += Wpoly6(radiusSquared);
    //                                }
    //                            }
    //                        }
    //                    }
    //                }

    //                particle->density() *= CParticle::mass;
    //                // p = k(density - density_rest)
    //                particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
    //            }
    //        }
    //    }
    //}
}

void CGPUParticleSimulator::updateForces()
{
    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                auto &particles = m_grid->at(x, y, z);

                for (auto &particle : particles) {
                    QVector3D f_gravity = gravity * particle->density();
                    QVector3D f_pressure, f_viscosity, f_surface;

                    // neighbors
                    for (int offsetX = -1; offsetX <= 1; offsetX++) {
                        if (x + offsetX < 0) continue;
                        if (x + offsetX >= m_grid->xRes()) break;

                        for (int offsetY = -1; offsetY <= 1; offsetY++) {
                            if (y + offsetY < 0) continue;
                            if (y + offsetY >= m_grid->yRes()) break;

                            for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                                if (z + offsetZ < 0) continue;
                                if (z + offsetZ >= m_grid->zRes()) break;

                                auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                                for (auto &neighbor : neighborGridCellParticles) {

                                    QVector3D distance = particle->diffPosition(neighbor);
                                    double radiusSquared = distance.lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h) {
                                        QVector3D poly6Gradient = Wpoly6Gradient(distance, radiusSquared);
                                        QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);

                                        if (particle->getId() != neighbor->getId()) {
                                            f_pressure += (particle->pressure() / pow(particle->density(), 2) + neighbor->pressure() / pow(neighbor->density(), 2)) * spikyGradient;
                                            f_viscosity += (neighbor->velocity() - particle->velocity()) * WviscosityLaplacian(radiusSquared) / neighbor->density();
                                        }
                                    }
                                }
                            }
                        }
                    }

                    f_pressure *= -CParticle::mass * particle->density();
                    f_viscosity *= CParticle::viscosity * CParticle::mass;

                    // ADD IN SPH FORCES
                    particle->acceleration() = (f_pressure + f_viscosity + f_gravity) / particle->density();
                    // collision force
                    particle->acceleration() += m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particle->position(), particle->velocity());

                }
            }
        }
    }
}

void CGPUParticleSimulator::test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity)
{
    newPosition = position + (velocity * dt) + acceleration * dt * dt;
    newVelocity = (newPosition - position) / dt;
}


void CGPUParticleSimulator::integrate()
{
    for (unsigned int gridCellIndex = 0; gridCellIndex < m_grid->getCellCount(); gridCellIndex++) {
        std::vector<CParticle *> &particles = m_grid->getData()[gridCellIndex];

        for (auto &particle : particles) {
            QVector3D newPosition, newVelocity;

            test(dt, particle->position(), particle->velocity(), particle->acceleration(), newPosition, newVelocity);

            particle->translate(newPosition);
            particle->velocity() = newVelocity;
        }
    }
}

