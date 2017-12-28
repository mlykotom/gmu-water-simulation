#include "CParticle.h"

CParticle::CParticle(Qt3DExtras::QSphereMesh *mesh,Qt3DExtras::QPhongMaterial *material, Physics *physics, unsigned int id, Qt3DCore::QEntity *rootEntity, float x, float y, float z)
    : RenderableEntity(rootEntity),
      m_physics(physics),
      m_position(x, y, z),
      m_velocity(physics->velocity.x, physics->velocity.y, physics->velocity.z),
      m_acceleration(0.0f, 0.0f, 0.0f)
{
    // Sphere mesh transform
    m_transform = new Qt3DCore::QTransform();
    m_transform->setTranslation(m_position);

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    sphereEntity->addComponent(mesh);
    sphereEntity->addComponent(material);
    sphereEntity->addComponent(m_transform);
    sphereEntity->setEnabled(true);
}

CParticle::~CParticle() = default;