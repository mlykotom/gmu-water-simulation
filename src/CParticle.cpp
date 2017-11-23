#include "CParticle.h"

CParticle::CParticle(Qt3DCore::QEntity *rootEntity)
{
    this->rootEntity = rootEntity;

    // Sphere shape data
    sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(2);

    // Sphere mesh transform
    sphereTransform = new Qt3DCore::QTransform();
    sphereTransform->setScale(.05f);

    sphereMaterial = new Qt3DExtras::QPhongMaterial();
    sphereMaterial->setDiffuse(QColor(QRgb(0x14aaff)));

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(sphereMaterial);
    sphereEntity->addComponent(sphereTransform);
    sphereEntity->setEnabled(true);
}

CParticle::~CParticle()
{
}

void CParticle::translate(QVector3D d)
{
    sphereTransform->setTranslation(d);
}