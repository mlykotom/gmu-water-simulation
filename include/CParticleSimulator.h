#ifndef WATERSURFACESIMULATION_PARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_PARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QPlaneMesh>
#include "CParticle.h"
#include "CScene.h"
#include "CGrid.h"
#include <QtMath>

#define GRAVITY_ACCELERATION (-9.80665f)
#define WALL_K 10000.0 // wall spring constant
#define WALL_DAMPING (-0.9) // wall damping constant

static double Wpoly6(double radiusSquared)
{
    static double coefficient = 315.0 / (64.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

static const QVector3D Wpoly6Gradient(QVector3D &diffPosition, double radiusSquared)
{
    static double coefficient = -945.0 / (32.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 2) * diffPosition;
}

static const QVector3D WspikyGradient(QVector3D &diffPosition, double radiusSquared)
{
    static double coefficient = -45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * pow(CParticle::h - radius, 2) * diffPosition / radius;
}

static double WviscosityLaplacian(double radiusSquared)
{
    static double coefficient = 45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * (CParticle::h - radius);
}

class CParticleSimulator: QObject
{
Q_OBJECT

private slots:
    void doWork()
    {
        this->step(dt);
    };

private:
    QTimer m_timer;
    CScene *m_scene;
    QVector3D gravity;

    unsigned long m_particles_count;
    std::vector<CParticle *> *m_particles;

    double dt = 0.01;

    std::vector<QPair<QVector3D, QVector3D>> _walls = std::vector<QPair<QVector3D, QVector3D>>();

    CGrid *m_grid;

    QVector3D boxSize = QVector3D(0.4f, 0.4f, 0.4f);

public:
    explicit CParticleSimulator(CScene *scene, unsigned long particlesCount = 1000)
        : gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
          m_scene(scene),
          m_particles_count(particlesCount),
          m_particles(new std::vector<CParticle *>())
    {

        int gridX = (int) ceil(boxSize.x() / CParticle::h);
        int gridY = (int) ceil(boxSize.y() / CParticle::h);
        int gridZ = (int) ceil(boxSize.z() / CParticle::h);

        m_grid = new CGrid(gridX, gridY, gridZ);

        connect(&m_timer, SIGNAL(timeout()), this, SLOT(doWork()));
        setup();
    }

    virtual ~CParticleSimulator()
    {
        delete m_particles;
    }

    virtual void setup()
    {
        _walls.emplace_back(QVector3D(0, 0, 1), QVector3D(0, 0, -m_grid->zRes() / 2.0f)); // back
        _walls.emplace_back(QVector3D(0, 0, -1), QVector3D(0, 0, m_grid->zRes() / 2.0f)); // front
        _walls.emplace_back(QVector3D(1, 0, 0), QVector3D(-m_grid->xRes() / 2.0f, 0, 0));     // left
        _walls.emplace_back(QVector3D(-1, 0, 0), QVector3D(m_grid->xRes() / 2.0f, 0, 0));     // right
        _walls.emplace_back(QVector3D(0, 1, 0), QVector3D(0, -m_grid->yRes() / 2.0f, 0)); // bottom
        _walls.emplace_back(QVector3D(0, -1, 0), QVector3D(0, m_grid->yRes() / 2.0f, 0)); // bottom
//
////         Plane shape data
//        Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
//        planeMesh->setWidth(m_grid->xRes());
//        planeMesh->setHeight(m_grid->yRes());
//
////        Plane        mesh transform
//        Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
////        planeTransform->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, 90.0));
//        planeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
//        planeTransform->setTranslation(_walls.at(0).second);
//
//        Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
//        planeMaterial->setDiffuse(QColor(QRgb(0xa69929)));
//
////        Plane
//        Qt3DCore::QEntity *planeEntity = new Qt3DCore::QEntity(m_scene->getRootEntity());
//        planeEntity->addComponent(planeMesh);
//        planeEntity->addComponent(planeMaterial);
//        planeEntity->addComponent(planeTransform);

// BRUTE FORCE
//        for (unsigned long i = 0; i < m_particles_count; ++i) {
//            CParticle *particle = new CParticle(i, m_scene->getRootEntity(), QVector3D(0.0001 * i, 0.0001 * i, 0.2));
//            m_particles->push_back(particle);
//        }

        auto &firstGridCell = m_grid->at(0, 0, 0);

        // add particles
        unsigned long particleId = 0;
        for (double y = -boxSize.y() / 2.0; y < boxSize.y() / 2.0; y += CParticle::h / 2.0) {
            for (double x = -boxSize.x() / 2.0; x < -boxSize.x() / 4.0; x += CParticle::h / 2.0) {
                for (double z = -boxSize.z() / 2.0; z < boxSize.z() / 2.0; z += CParticle::h / 2.0) {
                    auto particle = new CParticle(particleId, m_scene->getRootEntity(), QVector3D(x, y, z));
                    firstGridCell.push_back(particle);
                    particleId++;
                }
            }
        }

        updateGrid();
    }

    void start() { m_timer.start(); }

    void toggleSimulation()
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

    void updateGrid()
    {
        for (unsigned int x = 0; x < m_grid->xRes(); x++) {
            for (unsigned int y = 0; y < m_grid->yRes(); y++) {
                for (unsigned int z = 0; z < m_grid->zRes(); z++) {

                    auto &particles = m_grid->at(x, y, z);

                    for (int p = 0; p < particles.size(); p++) {
                        auto &particle = particles[p];

                        int newGridCellX = (int) floor((particle->position().x() + boxSize.x() / 2.0) / CParticle::h);
                        int newGridCellY = (int) floor((particle->position().y() + boxSize.y() / 2.0) / CParticle::h);
                        int newGridCellZ = (int) floor((particle->position().z() + boxSize.z() / 2.0) / CParticle::h);

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

                            // move the particle to the nem_grid ->ll

                            m_grid->at(newGridCellX, newGridCellY, newGridCellZ).push_back(particle);

                            // remove it from it's previous grid cell

                            particles[p] = particles.back();
                            particles.pop_back();
                            p--; // important! make sure to redo this index, since a new particle will (probably) be there
                        }

                    }
                }
            }
        }
    }

    void updateDensityPressure()
    {
        for (unsigned int x = 0; x < m_grid->xRes(); x++) {
            for (unsigned int y = 0; y < m_grid->yRes(); y++) {
                for (unsigned int z = 0; z < m_grid->zRes(); z++) {

                    auto &particles = m_grid->at(x, y, z);

                    for (int p = 0; p < particles.size(); p++) {
                        auto &particle = particles[p];

                        particle->density() = 0.0;

                        // neighbors
                        for (auto &neighbor : m_grid->getNeighborsCells(x, y, z)) {
                            double radiusSquared = particle->distanceTo(neighbor).lengthSquared();

                            if (radiusSquared <= CParticle::h * CParticle::h) {
                                particle->density() += Wpoly6(radiusSquared);
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

    void updateForces()
    {
        for (unsigned int x = 0; x < m_grid->xRes(); x++) {
            for (unsigned int y = 0; y < m_grid->yRes(); y++) {
                for (unsigned int z = 0; z < m_grid->zRes(); z++) {

                    auto &particles = m_grid->at(x, y, z);

                    for (int p = 0; p < particles.size(); p++) {
                        auto &particle = particles[p];

                        QVector3D f_gravity = particle->density() * gravity;
                        QVector3D f_pressure, f_viscosity;

                        // neighbors
                        for (auto &neighbor : m_grid->getNeighborsCells(x, y, z)) {
                            QVector3D distance = particle->distanceTo(neighbor);
                            double radiusSquared = distance.lengthSquared();

                            if (radiusSquared <= CParticle::h * CParticle::h) {
                                if (particle->getId() != neighbor->getId()) {
//                        QVector3D poly6Gradient = Wpoly6Gradient(distance, radiusSquared);
                                    QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);

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
                        for (auto wall : _walls) {
                            double d = QVector3D::dotProduct(wall.second - particle->position(), wall.first) + 0.01; // particle radius

                            if (d > 0.0) {
                                // This is an alernate way of calculating collisions of particles against walls, but produces some jitter at boundaries
                                particle->position() += d * wall.first;
                                particle->velocity() -= QVector3D::dotProduct(particle->velocity(), wall.first) * 1.9 * wall.first;

//                    particle->acceleration() += WALL_K * wall.first * d;
//                    particle->acceleration() += WALL_DAMPING * QVector3D::dotProduct(particle->velocity(), wall.first) * wall.first;
                            }
                        }
                    }
                }
            }
        }
    }

    void updateNewPositionVelocity()
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

    virtual void step(double dt)
    {
        updateDensityPressure();
        updateForces();
        updateNewPositionVelocity();
        updateGrid();
    }
};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
