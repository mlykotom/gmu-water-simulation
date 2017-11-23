#ifndef CPARTICLE_H
#define CPARTICLE_H

#include<QObject>
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

class CParticle
//    : QObject
{
//Q_OBJECT

private:
    Qt3DCore::QEntity *rootEntity;
    Qt3DExtras::QSphereMesh *sphereMesh;
    Qt3DCore::QTransform *sphereTransform;
    Qt3DExtras::QPhongMaterial *sphereMaterial;

public:

    QVector3D position = QVector3D(1.0, 1.0, 1.0);

    CParticle(Qt3DCore::QEntity *pEntity);
    ~CParticle();
    void translate(QVector3D d);
    void render();
};

#endif