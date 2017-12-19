#include "CBaseParticleSimulator.h"

CBaseParticleSimulator::CBaseParticleSimulator(CScene *scene, QObject *parent)
    : QObject(parent),
      gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
      m_scene(scene),
      dt(0.01f),
      totalIteration(0),
      m_surfaceThreshold(0.01f),
      //m_boxSize(QVector3D(0.3f, 0.3f, 0.3f))
      //m_boxSize(QVector3D(0.4, 0.4, 0.4))
      //m_boxSize(QVector3D(0.5, 0.5, 0.5))
      //m_boxSize(QVector3D(0.6f, 0.6f, 0.6f))
      m_boxSize(QVector3D(1.0f, 1.0f, 1.0f))
{

    m_systemParams.poly6_constant = (cl_float)(315.0f / (64.0f * M_PI * pow(CParticle::h, 9)));
    m_systemParams.spiky_constant = (cl_float)(-45.0f / (M_PI * pow(CParticle::h, 6)));
    m_systemParams.viscosity_constant = (cl_float)(45.0f / (M_PI * pow(CParticle::h, 6)));

    QVector3D gridResolution(
        (int) ceil(m_boxSize.x() / CParticle::h),
        (int) ceil(m_boxSize.y() / CParticle::h),
        (int) ceil(m_boxSize.z() / CParticle::h)
    );

    m_cellSize = QVector3D(m_boxSize.x() / gridResolution.x(), m_boxSize.y() / gridResolution.y(), m_boxSize.z() / gridResolution.z());
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

//    for (double y = -boxSize.y() / 4.0; y < boxSize.y() / 4.0; y += halfParticle) {
//        for (double x = -boxSize.x() / 4.0; x < boxSize.x() / 4.0; x += halfParticle) {
//            for (double z = -boxSize.z() / 4.0; z < boxSize.z() / 4.0; z += halfParticle) {
                auto particle = new CParticle(m_particlesCount, m_scene->getRootEntity(), QVector3D(x, y, z));
                m_particles.push_back(particle);
                firstGridCell.push_back(particle);
                m_particlesCount++;

            }
        }
    }

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << m_particlesCount << "particles";
}

void CBaseParticleSimulator::start()
{
    m_timer.start();
    m_elapsed_timer.start();
}

void CBaseParticleSimulator::toggleSimulation()
{
    if (m_timer.isActive()) {
        qDebug() << "pausing simulation...";
        m_timer.stop();
    }
    else {
        qDebug() << "resuming simulation ...";
        iterationSincePaused = 0;
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

double CBaseParticleSimulator::Wpoly6(double radiusSquared)
{
    static double coefficient = 315.0 / (64.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

QVector3D CBaseParticleSimulator::Wpoly6Gradient(QVector3D &diffPosition, double radiusSquared)
{
    static double coefficient = -945.0 / (32.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 2) * diffPosition;
}

QVector3D CBaseParticleSimulator::WspikyGradient(QVector3D &diffPosition, double radiusSquared)
{
    static double coefficient = -45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * pow(CParticle::h - radius, 2) * diffPosition / radius;
}

double CBaseParticleSimulator::WviscosityLaplacian(double radiusSquared)
{
    static double coefficient = 45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * (CParticle::h - radius);
}

void CBaseParticleSimulator::doWork()
{
    this->step();
    ++totalIteration;
    ++iterationSincePaused;
    emit iterationChanged(totalIteration);
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
