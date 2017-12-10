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

}

std::vector<cl_int> CGPUParticleSimulator::scan(std::vector<cl_int> input)
{
    size_t originalSize = input.size();
    //find nearest power of 2 to given count
    size_t countAsPowerOfTwo = pow(ceil(log2(originalSize)),2);

    //resize and append 0s
    input.resize(countAsPowerOfTwo, 0);

    cl_int *input_array = input.data();
    cl_int inputCount = input.size();
    size_t inputSize = inputCount * sizeof(cl_int);


    std::vector<cl_int> output;
    output.resize(inputCount, 0);

    cl_int *output_array = output.data();

    cl::Kernel kernel = cl::Kernel(m_cl_wrapper->getKernel("blelloch_scan"));

    cl_int err;

    auto inputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, inputSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    auto outputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, inputSize, input_array, &err);
    CLCommon::checkError(err, "outputBuffer creation");

    kernel.setArg(0, inputBuffer);
    kernel.setArg(1, inputCount);
    kernel.setArg(2, outputBuffer);
    kernel.setArg(3, cl::Local(inputSize));

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(16);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(inputCount, 16));
    cl::NDRange offset(0);

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(inputBuffer, true, 0, inputSize, input_array, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, inputSize, output_array, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");


    input.resize(originalSize);
    output.resize(originalSize);

    return output;
}

//TODO: TEST - DELETE
void CGPUParticleSimulator::test()
{
  //  std::vector<cl_int> input;

  //  input.push_back(1);
  //  input.push_back(2);
  //  input.push_back(3);
  //  input.push_back(4);

  //  input.push_back(5);
  //  input.push_back(6);
  //  input.push_back(7); 
  //  input.push_back(8);

  //  input.push_back(1);
  //  input.push_back(2);
  //  input.push_back(3);
  //  input.push_back(4);

  ////  input.push_back(5);
  //  input.push_back(6);
  //  input.push_back(7);
  //  //input.push_back(8);


  //  std::vector<cl_int> output =  scan(input);

  //  for (cl_int i : output)
  //      qDebug() << i;

    setupScene();
    updateGrid();



    return;


    
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

    //for (int i = 0; i < m_particlesCount; ++i)
    //{
    //    qDebug() << m_device_data[i].position.x << m_device_data[i].position.y << m_device_data[i].position.z;
    //}

    qDebug() << sizeof(m_clParticles.at(0));
    qDebug() << sizeof(CParticle::Physics);
    qDebug() << m_particlesCount << m_clParticles.size();

    cl::Kernel kernel = cl::Kernel(m_cl_wrapper->getKernel("update_grid_positions"));

    cl_int pariclesCount = m_particlesCount;
    size_t particlesSize = pariclesCount * sizeof(CParticle::Physics);

    cl_float3 halfCellSize = { m_cellSize.x() / 2.0, m_cellSize.y() / 2.0, m_cellSize.z() / 2.0 };
    cl_int3 gridSize = { m_grid->xRes(),m_grid->yRes() ,m_grid->zRes() };

    std::vector<cl_int> output;
    cl_int outputCount = m_grid->getCellCount();
    output.resize(outputCount,0);
    cl_int *output_array = output.data();
    size_t outputSize = outputCount * sizeof(cl_int);

    cl_int err;

    auto inputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, particlesSize, nullptr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    auto outputBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, outputSize, output_array, &err);
    CLCommon::checkError(err, "outputBuffer creation");



    cl_int arg = 0;
    kernel.setArg(arg++, inputBuffer);
    kernel.setArg(arg++, outputBuffer);
    kernel.setArg(arg++, (cl_int)pariclesCount);
    kernel.setArg(arg++, gridSize);
    kernel.setArg(arg++, halfCellSize);
    kernel.setArg(arg++, (cl_float)CParticle::h);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;


    cl::NDRange local(16);
    //we need only half the threads of the input count
    cl::NDRange global(CLCommon::alignTo(pariclesCount, 16));
    cl::NDRange offset(0);


    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(inputBuffer, true, 0, particlesSize, m_clParticles.data(), nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, outputSize, output_array, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    for (cl_int i : output)
        qDebug() << i;

    //for (int x = 0; x < m_grid->xRes(); x++) {
    //    for (int y = 0; y < m_grid->yRes(); y++) {
    //        for (int z = 0; z < m_grid->zRes(); z++) {

    //            std::vector<CParticle *> &particles = m_grid->at(x, y, z);

    //            for (unsigned long p = 0; p < particles.size(); p++) {
    //                CParticle *particle = particles[p];

    //                int newGridCellX = (int) floor((particle->position().x() + m_cellSize.x() / 2.0) / CParticle::h);
    //                int newGridCellY = (int) floor((particle->position().y() + m_cellSize.y() / 2.0) / CParticle::h);
    //                int newGridCellZ = (int) floor((particle->position().z() + m_cellSize.z() / 2.0) / CParticle::h);
    //                //                        qDebug() << x << y << z << "NEW" << newGridCellX << newGridCellY << newGridCellZ;
    //                //cout << "particle position: " << particle->position() << endl;
    //                //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;

    //                if (newGridCellX < 0) {
    //                    newGridCellX = 0;
    //                }
    //                else if (newGridCellX >= m_grid->xRes()) {
    //                    newGridCellX = m_grid->xRes() - 1;
    //                }

    //                if (newGridCellY < 0) {
    //                    newGridCellY = 0;
    //                }
    //                else if (newGridCellY >= m_grid->yRes()) {
    //                    newGridCellY = m_grid->yRes() - 1;
    //                }

    //                if (newGridCellZ < 0) {
    //                    newGridCellZ = 0;
    //                }
    //                else if (newGridCellZ >= m_grid->zRes()) {
    //                    newGridCellZ = m_grid->zRes() - 1;
    //                }

    //                //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;


    //                // check if particle has moved

    //                if (x != newGridCellX || y != newGridCellY || z != newGridCellZ) {

    //                    // move the particle to the new grid cell

    //                    std::vector<CParticle *> &what = m_grid->at(newGridCellX, newGridCellY, newGridCellZ);
    //                    what.push_back(particle);
    //                    // remove it from it's previous grid cell

    //                    particles.at(p) = particles.back();
    //                    particles.pop_back();

    //                    p--; // important! make sure to redo this index, since a new particle will (probably) be there
    //                }
    //            }
    //        }
    //    }
    //}
}

void CGPUParticleSimulator::updateDensityPressure()
{
    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                auto &particles = m_grid->at(x, y, z);
                for (auto &particle : particles) {

                    particle->density() = 0.0;

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
                                    double radiusSquared = particle->diffPosition(neighbor).lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h) {
                                        particle->density() += Wpoly6(radiusSquared);
                                    }
                                }
                            }
                        }
                    }

                    particle->density() *= CParticle::mass;
                    // p = k(density - density_rest)
                    particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
                }
            }
        }
    }
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

