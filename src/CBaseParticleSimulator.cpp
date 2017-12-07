#include "CBaseParticleSimulator.h"

CBaseParticleSimulator::CBaseParticleSimulator(CScene *scene, QObject *parent)
    : QObject(parent),
      gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
      m_scene(scene),
      dt(0.01),
      totalIteration(0),
      surfaceThreshold(0.01),
      boxSize(QVector3D(0.3, 0.3, 0.3))
{
    QVector3D gridResolution(
        (int) ceil(boxSize.x() / CParticle::h),
        (int) ceil(boxSize.y() / CParticle::h),
        (int) ceil(boxSize.z() / CParticle::h)
    );

    m_cellSize = QVector3D(boxSize.x() / gridResolution.x(), boxSize.y() / gridResolution.y(), boxSize.z() / gridResolution.z());
    m_grid = new CGrid(boxSize, gridResolution, m_scene->getRootEntity());

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(doWork()));
}

void CBaseParticleSimulator::setupScene()
{
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
                particlesCount++;
            }
        }
    }

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << particlesCount << "particles";
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
        gravity = QVector3D(0, 0, 0);
    }
    else {
        gravity = QVector3D(0, GRAVITY_ACCELERATION, 0);
    }
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
    }
}

double CBaseParticleSimulator::getFps()
{
    double elapsed = getElapsedTime() / 1000.0;
    return iterationSincePaused / elapsed;
}
