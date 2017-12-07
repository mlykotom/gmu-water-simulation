#include <include/CLPlatforms.h>
#include "CCPUBruteParticleSimulator.h"

static int workGroup = 256;

CCPUBruteParticleSimulator::CCPUBruteParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
{
    CLPlatforms::printInfoAll();

    auto clDevice = CLPlatforms::getBestGPU();
    m_cl_wrapper = new CLWrapper(clDevice);
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());

    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/test.cl"
        }
    );

    m_test_kernel = m_cl_wrapper->getKernel("test");

    particlesCL = new std::vector<ParticleCL>();
}

void CCPUBruteParticleSimulator::setupScene()
{
    qDebug() << "asdasdasda";
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;
    // add particles
    for (float y = -boxSize.y() / 2.0f; y < boxSize.y() / 2.0f; y += halfParticle) {
        for (float x = -boxSize.x() / 2.0f; x < -boxSize.x() / 4.0; x += halfParticle) {
            for (float z = -boxSize.z() / 2.0f; z < boxSize.z() / 2.0f; z += halfParticle) {

//    for (double y = -boxSize.y() / 4.0; y < boxSize.y() / 4.0; y += halfParticle) {
//        for (double x = -boxSize.x() / 4.0; x < boxSize.x() / 4.0; x += halfParticle) {
//            for (double z = -boxSize.z() / 4.0; z < boxSize.z() / 4.0; z += halfParticle) {
                auto particle = new CParticle(particlesCount, m_scene->getRootEntity(), QVector3D(x, y, z));
                firstGridCell.push_back(particle);

                auto particleStruct = ParticleCL{
                    .position = {x, y, z},
                    .velocity = {0, 0, 0}
                };

                particlesCL->push_back(particleStruct);
                particlesCount++;
            }
        }
    }

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << particlesCount << "particles";
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    auto &particles = m_grid->at(0, 0, 0);
    for (int i = 0; i < particles.size(); ++i) {
        auto &particle = particles[i];

        particle->density() = 0.0;
        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

        for (auto &neighbor : neighborGridCellParticles) {
            double radiusSquared = particle->diffPosition(neighbor).lengthSquared();

            if (radiusSquared <= CParticle::h * CParticle::h) {
                particle->density() += Wpoly6(radiusSquared);
            }
        }

        particle->density() *= CParticle::mass;
        // p = k(density - density_rest)
        particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
    }
}

void CCPUBruteParticleSimulator::updateForces()
{
    auto &particles = m_grid->at(0, 0, 0);

    for (int i = 0; i < particles.size(); ++i) {
        auto &particle = particles[i];

        QVector3D f_gravity = gravity * particle->density();
        QVector3D f_pressure, f_viscosity, f_surface;

        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

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

        f_pressure *= -CParticle::mass * particle->density();
        f_viscosity *= CParticle::viscosity * CParticle::mass;

        // ADD IN SPH FORCES
        particle->acceleration() = (f_pressure + f_viscosity + f_gravity) / particle->density();
        // collision force
        particle->acceleration() += m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particle->position(), particle->velocity());

        particlesCL->at(i).acceleration = {particle->acceleration().x(), particle->acceleration().y(), particle->acceleration().z()};
    }
}

void CCPUBruteParticleSimulator::test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity)
{
    newPosition = position + (velocity * dt) + acceleration * dt * dt;
    newVelocity = (newPosition - position) / dt;
}

void CCPUBruteParticleSimulator::integrate()
{
    size_t dataBufferSize = particlesCL->size() * sizeof(ParticleCL);
    cl_int err;
    auto dataBuffer = cl::Buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, dataBufferSize, nullptr, &err);
    CLCommon::checkError(err, "dataBuffer creation");

    m_test_kernel.setArg(0, dataBuffer);
    m_test_kernel.setArg(1, dataBufferSize);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl::NDRange local(16);
    cl::NDRange global(CLCommon::alignTo(dataBufferSize, 16));

    ParticleCL *device_data = new ParticleCL[dataBufferSize]();


    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(dataBuffer, true, 0, dataBufferSize, &particlesCL[0], nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(m_test_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(dataBuffer, true, 0, dataBufferSize, device_data, nullptr, &readEvent);
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];

    for (int i = 0; i < particles.size(); ++i) {
        auto &particle = particles[i];
        auto partStru = device_data[i];

        QVector3D newPosition = QVector3D(partStru.position.s[0], partStru.position.s[1], partStru.position.s[2]);;
        QVector3D newVelocity = QVector3D(partStru.velocity.s[0], partStru.velocity.s[1], partStru.velocity.s[2]);;

//        test(dt, particle->position(), particle->velocity(), particle->acceleration(), newPosition, newVelocity);

        particle->translate(newPosition);
        particle->velocity() = newVelocity;
    }
}

