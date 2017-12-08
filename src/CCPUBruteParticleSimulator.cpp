#include <include/CLPlatforms.h>
#include <cassert>
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

    m_integration_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
}

void CCPUBruteParticleSimulator::setupScene()
{
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;

    int calculatedCount = (int) (ceil(boxSize.z() / halfParticle) * ceil(boxSize.y() / halfParticle) * ceil(boxSize.x() / 4 / halfParticle));
    device_data = new CParticle::Physics[calculatedCount];

    QVector3D offset = -boxSize / 2.0f;

    for (float y = 0; y < boxSize.y(); y += halfParticle) {
        for (float x = 0; x < boxSize.x() / 4.0; x += halfParticle) {
            for (float z = 0; z < boxSize.z(); z += halfParticle) {
                device_data[particlesCount] = CParticle::Physics{
                    .position       = {x + offset.x(), y + offset.y(), z + offset.z()},
                    .velocity       = {0, 0, 0},
                    .acceleration   = {0, 0, 0},
                    .density        = 0.0,
                    .pressure       = 0
                };

                auto particle = new CParticle(particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
                particle->m_physics = &device_data[particlesCount];

                firstGridCell.push_back(particle);
                particlesCount++;
            }
        }
    }

    assert(calculatedCount == particlesCount);

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << particlesCount << "particles";

    dataBufferSize = particlesCount * sizeof(CParticle::Physics);
    inputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, dataBufferSize);
    outputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, dataBufferSize);
}

CCPUBruteParticleSimulator::~CCPUBruteParticleSimulator()
{
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    auto &particles = m_grid->at(0, 0, 0);
    for (int i = 0; i < particles.size(); ++i) {
//        auto &particle = particles[i];
        CParticle::Physics &particleCL = device_data[i];

//        particle->density() = 0.0;
        particleCL.density = 0.0;

        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

        for (auto &neighbor : neighborGridCellParticles) {
            double radiusSquared = CParticle::diffPosition(particleCL.position, neighbor->m_physics->position).lengthSquared();

            if (radiusSquared <= CParticle::h * CParticle::h) {
//                particle->density() += Wpoly6(radiusSquared);
                particleCL.density += Wpoly6(radiusSquared);
            }
        }

//        particle->density() *= CParticle::mass;
        particleCL.density *= CParticle::mass;
//         p = k(density - density_rest)
//        particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
        particleCL.pressure = CParticle::gas_stiffness * (particleCL.density - CParticle::rest_density);

//        qDebug() << particle->density() << particleCL.density << "|" << particle->pressure() << particleCL.pressure;
    }

    m_update_density_kernel->setArg(0, outputBuffer);
    m_update_density_kernel->setArg(1, inputBuffer);
    m_update_density_kernel->setArg(2, dataBufferSize);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl::NDRange local(16);
    cl::NDRange global(CLCommon::alignTo(dataBufferSize, 16));

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(inputBuffer, true, 0, dataBufferSize, device_data, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_density_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, dataBufferSize, device_data, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");
}

void CCPUBruteParticleSimulator::updateForces()
{
    auto &particles = m_grid->at(0, 0, 0);

    for (int i = 0; i < particles.size(); ++i) {
        auto &particle = particles[i];
        CParticle::Physics &particleCL = device_data[i];

//        QVector3D f_gravity = gravity * particle->density();
        QVector3D f_gravity = gravity * particleCL.density;
        QVector3D f_pressure, f_viscosity;

        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

        for (auto &neighbor : neighborGridCellParticles) {

//            QVector3D distance = particle->diffPosition(neighbor);
            QVector3D distance = CParticle::diffPosition(particleCL.position, neighbor->m_physics->position);
            double radiusSquared = distance.lengthSquared();

            if (radiusSquared <= CParticle::h * CParticle::h) {
                QVector3D poly6Gradient = Wpoly6Gradient(distance, radiusSquared);
                QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);

                if (particle->getId() != neighbor->getId()) {
//                    f_pressure += (particle->pressure() / pow(particle->density(), 2) + neighbor->pressure() / pow(neighbor->density(), 2)) * spikyGradient;
//                    f_viscosity += (neighbor->velocity() - particle->velocity()) * WviscosityLaplacian(radiusSquared) / neighbor->density();
                    f_pressure += (particleCL.pressure / pow(particleCL.density, 2) + particleCL.pressure / pow(particleCL.density, 2)) * spikyGradient;
                    f_viscosity += CParticle::diffPosition(neighbor->m_physics->velocity, particleCL.velocity) * WviscosityLaplacian(radiusSquared) / particleCL.density;
                }
            }
        }

        f_pressure *= -CParticle::mass * particleCL.density;
        f_viscosity *= CParticle::viscosity * CParticle::mass;

        // ADD IN SPH FORCES
//        particle->acceleration() = (f_pressure + f_viscosity + f_gravity) / particle->density();
        QVector3D f_total = (f_pressure + f_viscosity + f_gravity) / particleCL.density;
        // collision force
//        particle->acceleration() += m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particle->position(), particle->velocity());

        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);
        QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);

        f_total += f_collision;

        particleCL.acceleration = {f_total.x(), f_total.y(), f_total.z()};

    }
}

void CCPUBruteParticleSimulator::integrate()
{
    m_integration_kernel->setArg(0, outputBuffer);
    m_integration_kernel->setArg(1, inputBuffer);
    m_integration_kernel->setArg(2, dataBufferSize);
    m_integration_kernel->setArg(3, dt);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl::NDRange local(16);
    cl::NDRange global(CLCommon::alignTo(dataBufferSize, 16));

    // TODO nastaveno blocking = true .. vsude bylo vzdycky false
    m_cl_wrapper->getQueue().enqueueWriteBuffer(inputBuffer, true, 0, dataBufferSize, device_data, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integration_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, true, 0, dataBufferSize, device_data, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}

