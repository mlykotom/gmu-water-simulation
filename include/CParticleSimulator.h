#ifndef WATERSURFACESIMULATION_PARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_PARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QPlaneMesh>
#include "CParticle.h"
#include "CScene.h"
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
        this->render();
    };

private:
    QTimer m_timer;
    CScene *m_scene;
    QVector3D gravity;

    unsigned long m_particles_count;
    std::vector<CParticle *> *m_particles;

    double dt = 0.01;

    std::vector<QPair<QVector3D, QVector3D>> _walls = std::vector<QPair<QVector3D, QVector3D>>();
    QVector3D boxSize = QVector3D(5, 5, 5);

public:
    explicit CParticleSimulator(CScene *scene, unsigned long particlesCount = 1000)
        : gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)),
          m_scene(scene),
          m_particles_count(particlesCount),
          m_particles(new std::vector<CParticle *>())
    {
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(doWork()));
        setup();
    }

    virtual ~CParticleSimulator()
    {
        delete m_particles;
    }

    virtual void setup()
    {
        _walls.emplace_back(QVector3D(0, 0, 1), QVector3D(0, 0, -boxSize.z() / 2.0)); // back
        _walls.emplace_back(QVector3D(0, 0, -1), QVector3D(0, 0, boxSize.z() / 2.0)); // front
        _walls.emplace_back(QVector3D(1, 0, 0), QVector3D(-boxSize.x() / 2.0, 0, 0));     // left
        _walls.emplace_back(QVector3D(-1, 0, 0), QVector3D(boxSize.x() / 2.0, 0, 0));     // right
        _walls.emplace_back(QVector3D(0, 1, 0), QVector3D(0, -boxSize.y() / 2.0, 0)); // bottom
        _walls.emplace_back(QVector3D(0, -1, 0), QVector3D(0, boxSize.y() / 2.0, 0)); // bottom

//         Plane shape data
        Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
        planeMesh->setWidth(boxSize.x());
        planeMesh->setHeight(boxSize.y());

//        Plane        mesh transform
        Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
//        planeTransform->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, 90.0));
        planeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
        planeTransform->setTranslation(_walls.at(0).second);

        Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
        planeMaterial->setDiffuse(QColor(QRgb(0xa69929)));

//        Plane
        Qt3DCore::QEntity *planeEntity = new Qt3DCore::QEntity(m_scene->getRootEntity());
        planeEntity->addComponent(planeMesh);
        planeEntity->addComponent(planeMaterial);
        planeEntity->addComponent(planeTransform);


        for (unsigned long i = 0; i < m_particles_count; ++i) {
            CParticle *particle = new CParticle(i, m_scene->getRootEntity(), QVector3D(0.0001 * i, 0.0001 * i, 0.2));
            m_particles->push_back(particle);
        }
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

    void updateDensityPressure()
    {
        for (auto &particle : *m_particles) {
            particle->density() = 0.0;

            // neighbors
            for (auto &otherParticle : *m_particles) {
                double radiusSquared = particle->distanceTo(otherParticle).lengthSquared();

                if (radiusSquared <= CParticle::h * CParticle::h) {
                    particle->density() += Wpoly6(radiusSquared);
                }
            }

            particle->density() *= CParticle::mass;
            // p = k(density - density_rest)
            particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
        }
    }

    void updateForces()
    {
        for (auto &particle : *m_particles) {
            QVector3D f_gravity = particle->density() * gravity;
            QVector3D f_pressure, f_viscosity;

            // neighbors
            for (auto &neighbor : *m_particles) {
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

    void updateNewPositionVelocity()
    {
        for (auto &particle : *m_particles) {
            QVector3D newPosition = particle->position() + (particle->velocity() * dt) + particle->acceleration() * dt * dt;
            QVector3D newVelocity = (newPosition - particle->position()) / dt;

            particle->translate(newPosition);
            particle->velocity() = newVelocity;
        }
    }

    virtual void step(double dt)
    {
        updateDensityPressure();
        updateForces();
        updateNewPositionVelocity();
    }
};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
