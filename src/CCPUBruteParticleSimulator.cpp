#include <include/CLPlatforms.h>
#include <cassert>
#include "CCPUBruteParticleSimulator.h"


CCPUBruteParticleSimulator::CCPUBruteParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
{
    CLPlatforms::printInfoAll();

    m_systemParams.poly6_constant = static_cast<cl_float>(315.0f / (64.0f * M_PI * pow(CParticle::h, 9)));
    m_systemParams.spiky_constant = static_cast<cl_float>(-45.0f / (M_PI * pow(CParticle::h, 6)));
    m_systemParams.viscosity_constant = static_cast<cl_float>(45.0f / (M_PI * pow(CParticle::h, 6)));

    m_gravityCL = {gravity.x(), gravity.y(), gravity.z()};

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

    int calculatedCount = (int) (ceil(m_boxSize.z() / halfParticle) * ceil(m_boxSize.y() / halfParticle) * ceil(m_boxSize.x() / 4 / halfParticle));
    m_device_data = new CParticle::Physics[calculatedCount];

    QVector3D offset = -m_boxSize / 2.0f;

    for (float y = 0; y < m_boxSize.y(); y += halfParticle) {
        for (float x = 0; x < m_boxSize.x() / 4.0; x += halfParticle) {
            for (float z = 0; z < m_boxSize.z(); z += halfParticle) {

                CParticle::Physics p;
                p.id = m_particlesCount;
                p.position = {x + offset.x(), y + offset.y(), z + offset.z()};
                p.velocity = {0, 0, 0};
                p.acceleration = {0, 0, 0};
                p.density = 0.0;
                p.pressure = 0;
                m_device_data[m_particlesCount] = p;

                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
                particle->m_physics = &m_device_data[m_particlesCount];

                firstGridCell.push_back(particle);
                m_particlesCount++;
            }
        }
    }

    assert(calculatedCount == m_particlesCount);

    m_dataBufferSize = m_particlesCount * sizeof(CParticle::Physics);
    m_outputBuffer = m_cl_wrapper->createBuffer(CL_MEM_READ_WRITE, m_dataBufferSize);

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << m_particlesCount << "particles";
}

CCPUBruteParticleSimulator::~CCPUBruteParticleSimulator()
{
    delete[] m_device_data;
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
    cl::Event writeEvent;
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_outputBuffer, CL_TRUE, 0, m_dataBufferSize, m_device_data, nullptr, &writeEvent);

}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    cl_uint arg = 0;
    m_update_density_kernel->setArg(arg++, m_outputBuffer);
    m_update_density_kernel->setArg(arg++, m_particlesCount);
    m_update_density_kernel->setArg(arg++, m_systemParams.poly6_constant);

    cl::Event kernelEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_density_kernel, 0, global, local, nullptr, &kernelEvent);
}

void CCPUBruteParticleSimulator::updateForces()
{
    cl_uint arg = 0;
    m_update_forces_kernel->setArg(arg++, m_outputBuffer);
    m_update_forces_kernel->setArg(arg++, m_particlesCount);
    m_update_forces_kernel->setArg(arg++, m_gravityCL);
    m_update_forces_kernel->setArg(arg++, m_systemParams.spiky_constant);
    m_update_forces_kernel->setArg(arg++, m_systemParams.viscosity_constant);

    cl::Event kernelEvent, readEvent, writeEventAfterCollision;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_update_forces_kernel, 0, global, local, nullptr, &kernelEvent);

    // need to read buffer and wait until finish because of collision computations
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_outputBuffer, CL_TRUE, 0, m_dataBufferSize, m_device_data, nullptr, &readEvent);
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    // collision force
    for (int i = 0; i < m_particlesCount; ++i) {
        CParticle::Physics &particleCL = m_device_data[i];
        QVector3D pos = CParticle::clFloatToVector(particleCL.position);
        QVector3D velocity = CParticle::clFloatToVector(particleCL.velocity);

        m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particleCL);
        //QVector3D f_collision = m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(pos, velocity);
        //particleCL.acceleration = {particleCL.acceleration.x + f_collision.x(), particleCL.acceleration.y + f_collision.y(), particleCL.acceleration.z + f_collision.z()};
    }

    // need to write buffer because previous step has
    m_cl_wrapper->getQueue().enqueueWriteBuffer(m_outputBuffer, CL_FALSE, 0, m_dataBufferSize, m_device_data, nullptr, &writeEventAfterCollision);
}

void CCPUBruteParticleSimulator::integrate()
{
    cl_uint arg = 0;
    m_integration_kernel->setArg(arg++, m_outputBuffer);
    m_integration_kernel->setArg(arg++, m_particlesCount);
    m_integration_kernel->setArg(arg++, dt);

    cl::Event kernelEvent, readEvent;

    cl::NDRange local = cl::NullRange;
    cl::NDRange global(m_particlesCount);

    m_cl_wrapper->getQueue().enqueueNDRangeKernel(*m_integration_kernel, 0, global, local, nullptr, &kernelEvent);
    m_cl_wrapper->getQueue().enqueueReadBuffer(m_outputBuffer, CL_FALSE, 0, m_dataBufferSize, m_device_data, nullptr, &readEvent);

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
    m_gravityCL = {gravity.x(), gravity.y(), gravity.z()};
}

