#include <QPlaneMesh>
#include <iostream>
#include "CParticleSimulator.h"

CParticleSimulator::CParticleSimulator(CScene *pScene)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
    scene = pScene;

    chrobak = new std::vector<t_banana>();
    setup();
}

CParticleSimulator::~CParticleSimulator()
{
    delete[] particles;
    if (cube) delete cube;
}

t_banana CParticleSimulator::createPlane(CScene *scene, float x, float y, float z)
{
    // Sphere shape data
    auto sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(1);

    // Sphere mesh transform
    auto sphereTransform = new Qt3DCore::QTransform();
    sphereTransform->setScale(0.3f);
    sphereTransform->setTranslation(QVector3D(x, y, z));

    auto sphereMaterial = new Qt3DExtras::QPhongMaterial();
//    sphereMaterial->setDiffuse(QColor(QRgb(0x14aaffcc)));

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(scene->m_rootEntity);
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(sphereMaterial);
    sphereEntity->addComponent(sphereTransform);
    sphereEntity->setEnabled(true);


//    // Plane shape data
//    Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
//    planeMesh->setWidth(1);
//    planeMesh->setHeight(1);
//
//    //     Plane mesh transform
//    Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
//    planeTransform->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, 90.0));
//    planeTransform->setTranslation(QVector3D(x, y, z));
//
//    Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
//    planeMaterial->setDiffuse(QColor("white"));
//    planeMaterial->setShininess(0);
//
//    // Plane
//    Qt3DCore::QEntity *planeEntity = new Qt3DCore::QEntity(scene->m_rootEntity);
//    planeEntity->addComponent(planeMesh);
//    planeEntity->addComponent(planeMaterial);
//    planeEntity->addComponent(planeTransform);

    return t_banana(sphereEntity, sphereMaterial);
}

void CParticleSimulator::setup()
{
    cube = new FluidCube(size, 0.2, 0.1, 0.001);

    for (int z = 0; z < size; ++z) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                auto entity = createPlane(scene, x, y, -z);
                chrobak->push_back(entity);
            }
        }
    }

    qDebug() << chrobak->size();
}

void CParticleSimulator::step()
{
    if (iteration % 100 == 0) {
        qDebug() << "iteration: " << iteration;
    }

//    if (iteration % 3000 < 2500) {
    cube->addDensity(4, 4, 1, 0.9);
//    cube->addVelocity(4, 4, 1, 1, 1, 0);
//    }

    cube->step();

    iteration++;
}

static double density_eps = 0.01;

void CParticleSimulator::render()
{

    for (unsigned long z = 0; z < size; ++z) {
        for (unsigned long y = 0; y < size; ++y) {
            for (unsigned long x = 0; x < size; ++x) {
                float density = cube->density[IX(size, x, y, z)];
                auto particle = chrobak->at(IX(size, x, y, z));

                if (density > density_eps) {
                    int c = (int) (density) * 255;

                    if (c > 255) c = 255;

                    particle.second->setDiffuse(QColor(0, 0, c));

                    particle.first->setEnabled(true);
                }
                else {
                    particle.first->setEnabled(false);
                }

                if (x == 0) {
                    particle.second->setDiffuse(QColor("orange"));
                    particle.first->setEnabled(true);
                }
                if (y == 0) {
                    particle.second->setDiffuse(QColor("yellow"));
                    particle.first->setEnabled(true);
                }
//                if (z == 0) {
//                    particle.second->setDiffuse(QColor("purple"));
//                    particle.first->setEnabled(true);
//                }

            }
        }
    }
}
void CParticleSimulator::addDensity(int x, int y, int z)
{
    auto old = cube->density[IX(size, x, y, z)];

    cube->addDensity(4, 4, 1, 100);
//    cube->addVelocity(4, 4, 1, 0, -100, 0);
    qDebug() << old << "|" << cube->density[IX(size, x, y, z)];
}

// -- random moving particles
//void CParticleSimulator::setup()
//{
//    particles = new particleVector();
//
//    for (int i = 0; i < particlesNumber; ++i) {
//        particles->push_back(new CParticle(scene->m_rootEntity));
//    }
//}


//void CParticleSimulator::step()
//{
//    for (int i = 0; i < particlesNumber; ++i) {
//        auto particle = particles->at(i);
//
//        float raX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
//        float raY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
//        float raZ = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / size));
//
//        particle->position.setX(raX);
//        particle->position.setY(raY);
//        particle->position.setZ(raZ);
//    }
//}
//
//void CParticleSimulator::render()
//{
//    for (int i = 0; i < particlesNumber; ++i) {
//        auto particle = particles->at(i);
//        particle->translate(particle->position);
//    }
//}
