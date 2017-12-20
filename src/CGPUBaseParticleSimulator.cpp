#include <include/CGPUBaseParticleSimulator.h>

CGPUBaseParticleSimulator::CGPUBaseParticleSimulator(CScene *scene, float boxSize, cl::Device device, QObject *parent)
    : CBaseParticleSimulator(scene, boxSize, parent),
      m_gravityCL({gravity.x(), gravity.y(), gravity.z()})
{
    CLPlatforms::printInfoAll();

    m_cl_wrapper = new CLWrapper(device);
}

void CGPUBaseParticleSimulator::setupScene()
{
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;

    unsigned int calculatedCount = (unsigned) (ceil(m_boxSize.z() / halfParticle) * ceil(m_boxSize.y() / halfParticle) * ceil(m_boxSize.x() / 4 / halfParticle));
    m_clParticles.reserve(calculatedCount);

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
                m_clParticles.push_back(p);

                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), QVector3D(x + offset.x(), y + offset.y(), z + offset.z()));
                particle->m_physics = &m_clParticles.back();

                firstGridCell.push_back(particle);
                m_particlesCount++;
            }
        }
    }

    assert(calculatedCount == m_particlesCount);
    setupKernels();
}

void CGPUBaseParticleSimulator::setGravityVector(QVector3D newGravity)
{
    CBaseParticleSimulator::setGravityVector(newGravity);
    m_gravityCL = {gravity.x(), gravity.y(), gravity.z()};
}

QString CGPUBaseParticleSimulator::getSelectedDevice()
{
    return CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());
}
