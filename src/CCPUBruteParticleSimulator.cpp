#include <include/CLPlatforms.h>
#include <cassert>
#include "CCPUBruteParticleSimulator.h"


CCPUBruteParticleSimulator::CCPUBruteParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
{
    CLPlatforms::printInfoAll();

    gravityCL = {gravity.x(), gravity.y(), gravity.z()};

    auto clDevice = CLPlatforms::getBestGPU();
    m_cl_wrapper = new CLWrapper(clDevice);
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());

    m_cl_wrapper->loadProgram(
        {
            APP_RESOURCES"/kernels/sph.cl"
        }
    );

    m_integration_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("integration_step"));
    m_update_density_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("density_pressure_step"));
    m_update_forces_kernel = std::make_shared<cl::Kernel>(m_cl_wrapper->getKernel("forces_step"));
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

                CParticle::Physics p;
                p.id = particlesCount;
                p.position = {x + offset.x(), y + offset.y(), z + offset.z()};
                p.velocity = {0, 0, 0};
                p.acceleration = {0, 0, 0};
                p.density = 0.0;
                p.pressure = 0;
                device_data[particlesCount] = p;

                auto particle = new CParticle(particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
                particle->m_physics = &device_data[particlesCount];

                firstGridCell.push_back(particle);
                particlesCount++;
            }
        }
    }

    assert(calculatedCount == particlesCount);

    dataBufferSize = particlesCount * sizeof(CParticle::Physics);
//    inputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, dataBufferSize);
    outputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, dataBufferSize);

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << particlesCount << "particles";
}

CCPUBruteParticleSimulator::~CCPUBruteParticleSimulator()
{
    delete[] device_data;
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, outputBuffer);
    m_update_density_kernel->setArg(arg++, particlesCount);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(particlesCount);

    m_cl_wrapper->getQueue().enqueueWriteBuffer(outputBuffer, CL_TRUE, 0, dataBufferSize, device_data, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_density_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, CL_TRUE, 0, dataBufferSize, device_data, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");
}

void CCPUBruteParticleSimulator::updateForces()
{
//    for (int i = 0; i < particlesCount; ++i) {
//        CParticle::Physics &particleCL = device_data[i];
//
//        QVector3D f_gravity = gravity * particleCL.density;
//        QVector3D f_pressure, f_viscosity;
//
//        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);
//
//        for (auto &neighbor : neighborGridCellParticles) {
//            QVector3D distance = CParticle::diffPosition(particleCL.position, neighbor->m_physics->position);
//            double radiusSquared = distance.lengthSquared();
//
//            if (radiusSquared <= CParticle::h * CParticle::h) {
//                QVector3D poly6Gradient = Wpoly6Gradient(distance, radiusSquared);
//                QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);
//
//                if (particleCL.id != neighbor->m_physics->id) {
//                    f_pressure += (particleCL.pressure / pow(particleCL.density, 2) + particleCL.pressure / pow(particleCL.density, 2)) * spikyGradient;
//                    f_viscosity += CParticle::diffPosition(neighbor->m_physics->velocity, particleCL.velocity) * WviscosityLaplacian(radiusSquared) / particleCL.density;
//                }
//            }
//        }
//
//        f_pressure *= -CParticle::mass * particleCL.density;
//        f_viscosity *= CParticle::viscosity * CParticle::mass;
//
//        // ADD IN SPH FORCES
//        QVector3D f_total = (f_pressure + f_viscosity + f_gravity) / particleCL.density;
//        particleCL.acceleration = {f_total.x(), f_total.y(), f_total.z()};
//    }

    {
        cl_uint arg = 0;
        m_update_forces_kernel->setArg(arg++, outputBuffer);
        m_update_forces_kernel->setArg(arg++, particlesCount);
        m_update_forces_kernel->setArg(arg++, gravityCL);

        cl::Event writeEvent;
        cl::Event kernelEvent;
        cl::Event readEvent;

        cl::NDRange local = cl::NullRange;
        cl::NDRange global(particlesCount);

        m_cl_wrapper->getQueue().enqueueWriteBuffer(outputBuffer, CL_TRUE, 0, dataBufferSize, device_data, nullptr, &writeEvent);
        m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_forces_kernel, 0, global, local, nullptr, &kernelEvent);
        m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, CL_TRUE, 0, dataBufferSize, device_data, nullptr, &readEvent);

        CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");
    }

    // collision force
    for (int i = 0; i < particlesCount; ++i) {
        CParticle::Physics &particleCL = device_data[i];
        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);
        QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);

        particleCL.acceleration = {particleCL.acceleration.s[0] + f_collision.x(), particleCL.acceleration.s[1] + f_collision.y(), particleCL.acceleration.s[2] + f_collision.z()};
    }
}

void CCPUBruteParticleSimulator::integrate()
{
    cl_uint arg = 0;
    m_integration_kernel->setArg(arg++, outputBuffer);
    m_integration_kernel->setArg(arg++, particlesCount);
    m_integration_kernel->setArg(arg++, dt);

    cl::Event writeEvent;
    cl::Event kernelEvent;
    cl::Event readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(particlesCount);

    m_cl_wrapper->getQueue().enqueueWriteBuffer(outputBuffer, CL_FALSE, 0, dataBufferSize, device_data, nullptr, &writeEvent);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integration_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(outputBuffer, CL_FALSE, 0, dataBufferSize, device_data, nullptr, &readEvent);

    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    std::vector<CParticle *> &particles = m_grid->getData()[0];
    for (auto &particle : particles) {
        particle->updatePosition();
        particle->updateVelocity();
    }
}
void CCPUBruteParticleSimulator::setGravityVector(QVector3D newGravity)
{
    CBaseParticleSimulator::setGravityVector(newGravity);
    gravityCL = {gravity.x(), gravity.y(), gravity.z()};
}

