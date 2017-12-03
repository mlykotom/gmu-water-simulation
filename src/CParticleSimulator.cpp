#include "CParticleSimulator.h"

#include "CScene.h"

CParticleSimulator::CParticleSimulator(QObject *parent)
    :QObject(parent),
    m_scene(nullptr),
    m_particles_count(0),
    dt(0.01),
    iteration(0),
    surfaceThreshold(0.01),
    boxSize(QVector3D(2, 2, 2))
{
}

CParticleSimulator::CParticleSimulator(CScene *scene, unsigned long particlesCount, QObject *parent)
    :QObject(parent),
    gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
    m_scene(scene),
    m_particles_count(particlesCount),
    m_particles(new std::vector<CParticle *>()),
    dt(0.01),
    iteration(0),
    surfaceThreshold(0.01),
    boxSize(QVector3D(0.4, 0.4, 0.4))
{

    int gridX = (int)ceil(boxSize.x() / CParticle::h);
    int gridY = (int)ceil(boxSize.y() / CParticle::h);
    int gridZ = (int)ceil(boxSize.z() / CParticle::h);

    m_grid = new CGrid(gridX, gridY, gridZ, m_scene->getRootEntity());

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(doWork()));
    setup();
}

CParticleSimulator::~CParticleSimulator()
{

    delete m_particles;

}

void CParticleSimulator::setup()
{
    _walls.emplace_back(QVector3D(0, 0, 1), QVector3D(0, 0, -boxSize.z() / 2.0f)); // back
    _walls.emplace_back(QVector3D(0, 0, -1), QVector3D(0, 0, boxSize.z() / 2.0f)); // front
    _walls.emplace_back(QVector3D(1, 0, 0), QVector3D(-boxSize.x() / 2.0f, 0, 0));     // left
    _walls.emplace_back(QVector3D(-1, 0, 0), QVector3D(boxSize.x() / 2.0f, 0, 0));     // right
    _walls.emplace_back(QVector3D(0, 1, 0), QVector3D(0, -boxSize.y() / 2.0f, 0)); // bottom
                                                                                   //        _walls.emplace_back(QVector3D(0, -1, 0), QVector3D(0, m_grid->yRes() / 2.0f, 0)); // bottom

                                                                                   // BRUTE FORCE
                                                                                   //        for (unsigned long i = 0; i < m_particles_count; ++i) {
                                                                                   //            CParticle *particle = new CParticle(i, m_scene->getRootEntity(), QVector3D(0.0001 * i, 0.0001 * i, 0.2));
                                                                                   //            m_particles->push_back(particle);
                                                                                   //        }

    auto &firstGridCell = m_grid->at(0, 0, 0);

    double halfParticle = CParticle::h / 2.0f;
    // add particles
    unsigned long particleId = 0;
    //        for (float y = -boxSize.y() / 2.0f; y < boxSize.y() / 2.0f; y += halfParticle) {
    //            for (float x = -boxSize.x() / 2.0f; x < -boxSize.x() / 4.0; x += halfParticle) {
    //                for (float z = -boxSize.z() / 2.0f; z < boxSize.z() / 2.0f; z += halfParticle) {

    for (double y = 0; y < boxSize.y(); y += CParticle::h / 2.0) {
        for (double x = -boxSize.x() / 4.0; x < boxSize.x() / 4.0; x += CParticle::h / 2.0) {
            for (double z = -boxSize.z() / 4.0; z < boxSize.z() / 4.0; z += CParticle::h / 2.0) {
                auto particle = new CParticle(particleId, m_scene->getRootEntity(), QVector3D(x, y, z));
                firstGridCell.push_back(particle);
                particleId++;
            }
        }
    }

    qDebug() << "Grid size is " << m_grid->xRes() << "x" << m_grid->yRes() << "x" << m_grid->zRes() << endl;
    qDebug() << "simulating" << particleId << "particles";

    updateGrid();
}

void CParticleSimulator::start()
{
    m_timer.start();
    m_elapsed_timer.start();
}

void CParticleSimulator::toggleSimulation()
{
    if (m_timer.isActive()) {
        qDebug() << "pausing simulation...";
        m_timer.stop();
    }
    else {
        qDebug() << "resuming simulation ...";
        start();
    }
}

void CParticleSimulator::toggleGravity()
{
    if (gravity.length() > 0.0) {
        gravity = QVector3D(0, 0, 0);
    }
    else {
        gravity = QVector3D(0, GRAVITY_ACCELERATION, 0);
    }
}

void CParticleSimulator::updateGrid()
{
    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                std::vector<CParticle *> &particles = m_grid->at(x, y, z);
                //                    qDebug() << x << y << z << particles.size();

                for (int p = 0; p < particles.size(); p++) {
                    CParticle *particle = particles[p];

                    int newGridCellX = (int)floor((particle->position().x() + boxSize.x() / 2.0) / CParticle::h);
                    int newGridCellY = (int)floor((particle->position().y() + boxSize.y() / 2.0) / CParticle::h);
                    int newGridCellZ = (int)floor((particle->position().z() + boxSize.z() / 2.0) / CParticle::h);
                    //                        qDebug() << x << y << z << "NEW" << newGridCellX << newGridCellY << newGridCellZ;
                    //cout << "particle position: " << particle->position() << endl;
                    //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;

                    if (newGridCellX < 0) {
                        newGridCellX = 0;
                    }
                    else if (newGridCellX >= m_grid->xRes()) {
                        newGridCellX = m_grid->xRes() - 1;
                    }

                    if (newGridCellY < 0) {
                        newGridCellY = 0;
                    }
                    else if (newGridCellY >= m_grid->yRes()) {
                        newGridCellY = m_grid->yRes() - 1;
                    }

                    if (newGridCellZ < 0) {
                        newGridCellZ = 0;
                    }
                    else if (newGridCellZ >= m_grid->zRes()) {
                        newGridCellZ = m_grid->zRes() - 1;
                    }

                    //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;


                    // check if particle has moved

                    if (x != newGridCellX || y != newGridCellY || z != newGridCellZ) {

                        // move the particle to the new grid cell

                        std::vector<CParticle *> &what = m_grid->at(newGridCellX, newGridCellY, newGridCellZ);
                        what.push_back(particle);
                        // remove it from it's previous grid cell

                        particles.at(p) = particles.back();
                        particles.pop_back();

                        //                            qDebug() << particles.size() << what.size();

                        //                            if (particles.size() == what.size()) {
                        //                                qDebug() << x << y << z << newGridCellX << newGridCellY << newGridCellZ;
                        //                            }

                        p--; // important! make sure to redo this index, since a new particle will (probably) be there
                    }
                }
            }
        }
    }
}

void CParticleSimulator::updateDensityPressure()
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

void CParticleSimulator::updateForces()
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
                    for (auto wall : _walls) {
                        double d = QVector3D::dotProduct(wall.second - particle->position(), wall.first) + 0.01; // particle radius

                        if (d > 0.0) {
                            // This is an alernate way of calculating collisions of particles against walls, but produces some jitter at boundaries
                            //                                particle->position() += d * wall.first;
                            //                                particle->velocity() -= QVector3D::dotProduct(particle->velocity(), wall.first) * 1.9 * wall.first;

                            particle->acceleration() += WALL_K * wall.first * d;
                            particle->acceleration() += WALL_DAMPING * QVector3D::dotProduct(particle->velocity(), wall.first) * wall.first;
                        }
                    }
                }
            }
        }
    }
}

void CParticleSimulator::updateNewPositionVelocity()
{
    for (unsigned int gridCellIndex = 0; gridCellIndex < m_grid->getCellCount(); gridCellIndex++) {
        auto &particles = m_grid->getData()[gridCellIndex];

        for (auto &particle : particles) {
            QVector3D newPosition = particle->position() + (particle->velocity() * dt) + particle->acceleration() * dt * dt;
            QVector3D newVelocity = (newPosition - particle->position()) / dt;

            particle->translate(newPosition);
            particle->velocity() = newVelocity;
        }
    }
}

void CParticleSimulator::step(double dt)
{
    //        qDebug() << iteration;
    updateDensityPressure();
    updateForces();
    updateNewPositionVelocity();
    updateGrid();
}

double CParticleSimulator::Wpoly6(double radiusSquared)
{
    static double coefficient = 315.0 / (64.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

QVector3D CParticleSimulator::Wpoly6Gradient(QVector3D & diffPosition, double radiusSquared)
{
    static double coefficient = -945.0 / (32.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 2) * diffPosition;
}

QVector3D CParticleSimulator::WspikyGradient(QVector3D & diffPosition, double radiusSquared)
{
    static double coefficient = -45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * pow(CParticle::h - radius, 2) * diffPosition / radius;
}

double CParticleSimulator::WviscosityLaplacian(double radiusSquared)
{
    static double coefficient = 45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * (CParticle::h - radius);
}

void CParticleSimulator::doWork()
{
    this->step(dt);
    emit iterationChanged(++iteration);
};


void CParticleSimulator::onKeyPressed(Qt::Key key)
{
    switch (key)
    {
        case Qt::Key_Space:
            toggleSimulation();
            break;

        case Qt::Key_G:
            toggleGravity();
            break;
    }
}