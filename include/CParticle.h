#ifndef WATERSURFACESIMULATION_PARTICLE_H
#define WATERSURFACESIMULATION_PARTICLE_H

#include <QEntity>
#include <QSphereMesh>
#include <QPhongMaterial>
#include <Qt3DCore/qtransform.h>


class CParticle
{
private:
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DExtras::QSphereMesh *m_mesh;
    Qt3DCore::QTransform *m_transform;
    Qt3DExtras::QPhongMaterial *m_material;

    const unsigned long m_id;
public:
    explicit CParticle(const unsigned long id, Qt3DCore::QEntity *rootEntity,
                       QVector3D initialPosition = QVector3D(0, 0, 0))
        : m_id(id), m_rootEntity(rootEntity), m_position(initialPosition), m_velocity(0.0f, 0.0f, 0.0f),
          m_acceleration(0.0f, 0.0f, 0.0f), m_density(0.0), m_pressure(0.0)
    {
        // Sphere shape data
        m_mesh = new Qt3DExtras::QSphereMesh();
        m_mesh->setRings(20);
        m_mesh->setSlices(20);
        m_mesh->setRadius(1);

        // Sphere mesh transform
        m_transform = new Qt3DCore::QTransform();
        m_transform->setTranslation(initialPosition);
        m_transform->setScale(0.1);

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

    virtual ~CParticle()
    {
        delete m_material;
        delete m_mesh;
        delete m_transform;
    }

private:
    QVector3D m_position;
    QVector3D m_velocity;
    QVector3D m_acceleration;
    double m_density;
    double m_pressure;

public:
    static constexpr double h = 0.0457; //0.02 //0.045
    static constexpr double viscosity = 3.5; // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
    static constexpr double mass = 0.02; // kg
    static constexpr double gas_stiffness = 3.0; //20.0 // 461.5  // Nm/kg is gas constant of water vapor
    static constexpr double rest_density = 998.29; // kg/m^3 is rest density of water particle

    const unsigned long getId() const { return m_id; }

    QVector3D &position() { return m_position; }
    QVector3D &acceleration() { return m_acceleration; };
    QVector3D &velocity() { return m_velocity; };
    double &density() { return m_density; };
    double &pressure() { return m_pressure; };


    void translate(QVector3D to)
    {
        m_position = to;
        m_transform->setTranslation(to);
    }

    inline QVector3D distanceTo(CParticle *otherParticle)
    {
        return position() - otherParticle->position();
    }
};


#endif //WATERSURFACESIMULATION_PARTICLE_H
