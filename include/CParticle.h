#ifndef CPARTICLE_H
#define CPARTICLE_H

#include <QEntity>
#include <QSphereMesh>
#include <QPhongMaterial>
#include <Qt3DCore/qtransform.h>
#include <CL/cl.hpp>
#include "renderableentity.h"

class CParticle: RenderableEntity
{
public:
    // WARNING: must be same as in kernels
    struct Physics
    {
        cl_float3 position;
        cl_float3 velocity;
        cl_float3 acceleration;
        cl_double density;
        cl_double pressure;
    };

    explicit CParticle(const unsigned long id, Qt3DCore::QEntity *rootEntity, QVector3D initialPosition = QVector3D(0, 0, 0));
    ~CParticle();

public: //methods

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

    void updatePosition()
    {
        m_position.setX(m_physics->position.s[0]);
        m_position.setY(m_physics->position.s[1]);
        m_position.setZ(m_physics->position.s[2]);

        m_transform->setTranslation(m_position);
    }

    void updateVelocity()
    {
        m_velocity.setX(m_physics->velocity.s[0]);
        m_velocity.setY(m_physics->velocity.s[1]);
        m_velocity.setZ(m_physics->velocity.s[2]);
    }

    inline QVector3D diffPosition(CParticle *otherParticle)
    {
        return position() - otherParticle->position();
    }

//TODOL urobit getre a settre
public: //attributes
    static constexpr double h = 0.0457;    //0.25    //0.02 //0.045
    static constexpr double viscosity = 3.5; // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
    static constexpr double mass = 0.02; // kg
    static constexpr double gas_stiffness = 3.0; //20.0 // 461.5  // Nm/kg is gas constant of water vapor
    static constexpr double rest_density = 998.29; // kg/m^3 is rest density of water particle

    const Physics *m_physics;
private:

    QVector3D m_position;
    QVector3D m_velocity;
    QVector3D m_acceleration;
    double m_density;
    double m_pressure;

    Qt3DExtras::QSphereMesh *m_mesh;
    Qt3DExtras::QPhongMaterial *m_material;

    const unsigned long m_id;

};


#endif //WATERSURFACESIMULATION_PARTICLE_H
