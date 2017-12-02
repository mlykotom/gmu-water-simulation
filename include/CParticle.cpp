#include "CParticle.h"

CParticle::CParticle(const unsigned long id, Qt3DCore::QEntity * rootEntity, QVector3D initialPosition)
    : m_id(id),
    m_rootEntity(rootEntity),
    m_position(initialPosition), 
    m_velocity(0.0f, 0.0f, 0.0f),
    m_acceleration(0.0f, 0.0f, 0.0f), 
    m_density(0.0), m_pressure(0.0)
{
    // Sphere shape data
    m_mesh = new Qt3DExtras::QSphereMesh();
    m_mesh->setRings(10);
    m_mesh->setSlices(10);
    m_mesh->setRadius(0.014f);

    // Sphere mesh transform
    m_transform = new Qt3DCore::QTransform();
    m_transform->setTranslation(initialPosition);
    //        m_transform->setScale(0.1);

    // material
    m_material = new Qt3DExtras::QPhongMaterial();
    m_material->setDiffuse(QColor(QRgb(0x14aaff)));

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(m_rootEntity);
    sphereEntity->addComponent(m_mesh);
    sphereEntity->addComponent(m_material);
    sphereEntity->addComponent(m_transform);
    sphereEntity->setEnabled(true);
}

CParticle::~CParticle()
{
    //delete m_material;
    //delete m_mesh;
    //delete m_transform;
}