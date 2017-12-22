#include "CParticle.h"

CParticle::CParticle(Physics *physics, unsigned int id, Qt3DCore::QEntity *rootEntity, float x, float y, float z)
    : RenderableEntity(rootEntity),
      m_physics(physics),
      m_position(QVector3D(x, y, z)),
      m_velocity(0.0f, 0.0f, 0.0f),
      m_acceleration(0.0f, 0.0f, 0.0f)
{
    // Sphere shape data
    m_mesh = new Qt3DExtras::QSphereMesh();
    m_mesh->setRings(10);
    m_mesh->setSlices(10);
    m_mesh->setRadius(0.01f);

    // Sphere mesh transform
    m_transform = new Qt3DCore::QTransform();
    m_transform->setTranslation(m_position);

    // material
    m_material = new Qt3DExtras::QPhongMaterial();
    m_material->setDiffuse(QColor(QRgb(0x14aaff)));

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    sphereEntity->addComponent(m_mesh);
    sphereEntity->addComponent(m_material);
    sphereEntity->addComponent(m_transform);
    sphereEntity->setEnabled(true);

}

CParticle::~CParticle() = default;