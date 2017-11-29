#include "CScene.h"

#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qpointlight.h>

#include <Qt3DCore/qtransform.h>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/qentity.h>
#include <include/CParticle.h>
#include <QPlaneMesh>
#include <QThread>


#include "CGrid.h"

CScene::CScene()
{
    // Root entity
    m_rootEntity = new Qt3DCore::QEntity();

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0, 0, 100.0f));
    lightEntity->addComponent(lightTransform);

    //m_grid = new CGrid(m_rootEntity);


}

CScene::~CScene()
{
    delete m_rootEntity;
}

void CScene::createSphere()
{
    // Sphere shape data
    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(2);

    // Sphere mesh transform
    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform();

    sphereTransform->setScale(1.3f);
    sphereTransform->setTranslation(QVector3D(-5.0f, -4.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *sphereMaterial = new Qt3DExtras::QPhongMaterial();
    sphereMaterial->setDiffuse(QColor(QRgb(0xa69929)));

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(m_rootEntity);
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(sphereMaterial);
    sphereEntity->addComponent(sphereTransform);
    sphereEntity->setEnabled(true);
}

void CScene::createScene()
{
    // Plane shape data
    Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(2);
    planeMesh->setHeight(2);

//     Plane mesh transform
    Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
    planeTransform->setScale(5.0f);
    planeTransform->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, 90.0));
//    planeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
    planeTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
    planeMaterial->setDiffuse(QColor(QRgb(0xa69929)));

    // Plane
    Qt3DCore::QEntity *planeEntity = new Qt3DCore::QEntity(m_rootEntity);
    planeEntity->addComponent(planeMesh);
    planeEntity->addComponent(planeMaterial);
    planeEntity->addComponent(planeTransform);
}

void CScene::addGrid()
{
}
