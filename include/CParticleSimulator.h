#ifndef WATERSURFACESIMULATION_PARTICLESIMULATOR_H
#define WATERSURFACESIMULATION_PARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QPlaneMesh>
#include "CParticle.h"
#include "CScene.h"
#include <QtMath>

#define GRAVITY_ACCELERATION (-9.80665f)


#define GRID_SIZE 1.0
#define WALL_K 10000.0 // wall spring constant
#define WALL_DAMPING -0.9 // wall damping constant


#define PARTICLES_COUNT 100

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

    std::vector<CParticle *> *m_particles;

    double dt = 0.01;

    std::vector<QPair<QVector3D, QVector3D>> _walls = std::vector<QPair<QVector3D, QVector3D>>();
    QVector3D boxSize = QVector3D(10, 10, 10);

public:
    explicit CParticleSimulator(CScene *scene)
        : gravity(QVector3D(0, GRAVITY_ACCELERATION, 0)), m_scene(scene), m_particles(new std::vector<CParticle *>())
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

        // Plane shape data
        Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
        planeMesh->setWidth(boxSize.x());
        planeMesh->setHeight(boxSize.y());

//     Plane mesh transform
        Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
        planeTransform->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, 90.0));
//    planeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
        planeTransform->setTranslation(_walls.at(0).second);

        Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
        planeMaterial->setDiffuse(QColor(QRgb(0xa69929)));

        // Plane
        Qt3DCore::QEntity *planeEntity = new Qt3DCore::QEntity(m_scene->getRootEntity());
        planeEntity->addComponent(planeMesh);
        planeEntity->addComponent(planeMaterial);
        planeEntity->addComponent(planeTransform);


        for (int i = 0; i < PARTICLES_COUNT; ++i) {
            CParticle *particle = new CParticle(m_scene->getRootEntity());

            float raX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1));
            float raY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1));
            float raZ = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1));

            particle->translate(QVector3D(raX, raY, raZ));
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
                double radiusSquared = particle->getRadiusSquared(otherParticle);

                if (radiusSquared <= CParticle::h * CParticle::h) {
                    particle->density() += Wpoly6(radiusSquared);
                }
            }

            particle->density() *= CParticle::mass;
            // p = k(density - density_rest)
            particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
        }
    }

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

    static double WviscosityLaplacian(double radiusSquared)
    {
        static double coefficient = 45.0 / (M_PI * pow(CParticle::h, 6));
        double radius = sqrt(radiusSquared);

        return coefficient * (CParticle::h - radius);
    }

    void updateForces()
    {

        for (auto &particle : *m_particles) {
            QVector3D f_gravity(0.0, particle->density() * -9.80665, 0.0);

            QVector3D f_pressure, f_viscosity, f_surface;

            // neighbors
            for (auto &neighbor : *m_particles) {
                QVector3D distance = particle->distanceTo(neighbor);
                double radiusSquared = QVector3D::dotProduct(distance, distance);

                if (radiusSquared <= CParticle::h * CParticle::h) {
                    if (radiusSquared > 0.0) {
                        QVector3D gradient = Wpoly6Gradient(distance, radiusSquared);
                        f_pressure += (particle->pressure() + neighbor->pressure()) / (2.0 * neighbor->density()) * gradient;
                    }

                    f_viscosity += (neighbor->velocity() - particle->velocity()) * WviscosityLaplacian(radiusSquared) / neighbor->density();
                }
            }

            f_pressure *= -CParticle::mass;
            f_viscosity *= CParticle::viscosity * CParticle::mass;

            // ADD IN SPH FORCES
            particle->acceleration() = (f_pressure + f_viscosity + f_surface + f_gravity) / particle->density();

            // collision force
            for (auto wall : _walls) {
                double d = QVector3D::dotProduct(wall.second - particle->position(), wall.first) + 0.01; // particle radius

                if (d > 0.0) {
                    // This is an alernate way of calculating collisions of particles against walls, but produces some jitter at boundaries
                    //particle.position() += d * wall.normal();
                    //particle.velocity() -= particle.velocity().dot(wall.normal()) * 1.9 * wall.normal();

                    particle->acceleration() += WALL_K * wall.first * d;
                    particle->acceleration() += WALL_DAMPING * QVector3D::dotProduct(particle->velocity(), wall.first) * wall.first;
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

//            qDebug() << particle->velocity();
        }
    }

    virtual void step(double dt)
    {
        updateDensityPressure();
        updateForces();
        updateNewPositionVelocity();
    }
    virtual void render() {}

};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
