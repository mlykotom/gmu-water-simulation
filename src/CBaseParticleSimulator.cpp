#include "CBaseParticleSimulator.h"

CBaseParticleSimulator::CBaseParticleSimulator(CScene *scene, float boxSize, QObject *parent)
    : QObject(parent),
      gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
      m_scene(scene),
      dt(0.01f),
      totalIteration(0),
      m_surfaceThreshold(0.01f),
      m_boxSize(QVector3D(boxSize, boxSize, boxSize))
{

    m_systemParams.poly6_constant = (cl_float) (315.0f / (64.0f * M_PI * pow(CParticle::h, 9)));
    m_systemParams.spiky_constant = (cl_float) (-45.0f / (M_PI * pow(CParticle::h, 6)));
    m_systemParams.viscosity_constant = (cl_float) (45.0f / (M_PI * pow(CParticle::h, 6)));

    QVector3D gridResolution(
        (int) ceil(m_boxSize.x() / CParticle::h),
        (int) ceil(m_boxSize.y() / CParticle::h),
        (int) ceil(m_boxSize.z() / CParticle::h)
    );


    m_grid = new CGrid(m_boxSize, gridResolution, m_scene->getRootEntity());

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(doWork()));
}

void CBaseParticleSimulator::setupScene()
{
    auto &firstGridCell = m_grid->at(0, 0, 0);
    double halfParticle = CParticle::h / 2.0f;

    // add particles
    for (float y = -m_boxSize.y() / 2.0f; y < m_boxSize.y() / 2.0f; y += halfParticle) {
        for (float x = -m_boxSize.x() / 2.0f; x < -m_boxSize.x() / 4.0; x += halfParticle) {
            for (float z = -m_boxSize.z() / 2.0f; z < m_boxSize.z() / 2.0f; z += halfParticle) {

                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), x, y, z);
                m_particles.push_back(particle);
                firstGridCell.push_back(particle);
                m_particlesCount++;
            }
        }
    }
}

void CBaseParticleSimulator::start()
{
    iterationSincePaused = 0;
    m_timer.start();
    m_elapsed_timer.start();
}

void CBaseParticleSimulator::stop()
{
    m_timer.stop();
}

void CBaseParticleSimulator::toggleSimulation()
{
    if (m_timer.isActive()) {
        qDebug() << "pausing simulation...";
        m_timer.stop();
    }
    else {
        qDebug() << "resuming simulation ...";
        m_elapsed_timer.restart();
        start();
    }
}

void CBaseParticleSimulator::toggleGravity()
{
    if (gravity.length() > 0.0) {
        setGravityVector(QVector3D(0, 0, 0));
    }
    else {
        setGravityVector(QVector3D(0, GRAVITY_ACCELERATION, 0));
    }
}

void CBaseParticleSimulator::setGravityVector(QVector3D newGravity)
{
    gravity = newGravity;
}

void CBaseParticleSimulator::step()
{
    updateGrid();
    updateDensityPressure();
    updateForces();
    integrate();
}

void CBaseParticleSimulator::doWork()
{
    this->step();
    ++totalIteration;
    ++iterationSincePaused;
    emit iterationChanged(totalIteration);
    if (totalIteration % eventLoggerStride == 0) {
        events << QPair<unsigned long, double>(totalIteration, getFps());
    }
};

void CBaseParticleSimulator::onKeyPressed(Qt::Key key)
{
    switch (key) {
        case Qt::Key_Space:
            toggleSimulation();
            break;

        case Qt::Key_G:
            toggleGravity();
            break;

        case Qt::Key_O:
            gravity.setX(gravity.x() - 1);
            setGravityVector(gravity);
            break;

        case Qt::Key_P:
            gravity.setX(gravity.x() + 1);
            setGravityVector(gravity);
            break;
    }
}

double CBaseParticleSimulator::getFps()
{
    double elapsed = getElapsedTime() / 1000.0;
    return iterationSincePaused / elapsed;
}


