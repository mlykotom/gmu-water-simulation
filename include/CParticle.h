#ifndef CPARTICLE_H
#define CPARTICLE_H

#include <QEntity>
#include <QSphereMesh>
#include <QPhongMaterial>
#include <Qt3DCore/qtransform.h>
#include <CL/cl.hpp>
#include "renderableentity.h"

#include <stddef.h>
#include <stdio.h>

class CParticle: RenderableEntity
{
public:
    // WARNING: must be same as in kernels

#ifdef WIN32
    typedef __declspec(align(16)) struct sPhysics
#else
    typedef struct __attribute__((aligned(16))) sPhysics
#endif
    {
        cl_float3 position;
        cl_float3 velocity;
        cl_float3 acceleration;
        cl_int3 grid_position;
        cl_float density;
        cl_float pressure;
        cl_uint id;
        cl_uint cell_id;

        sPhysics(float x, float y, float z, cl_uint id) :
            position({x, y, z}),
            id(id),
            acceleration({0, 0, 0}),
            density(0.0f),
            pressure(0.0f),
            velocity({0, 0, 0}),
            grid_position({0, 0, 0}),
            cell_id(0) {}
    } Physics;

    explicit CParticle(Physics *physics, unsigned int id, Qt3DCore::QEntity *rootEntity, float x, float y, float z);
    ~CParticle() override;

public: //methods

    unsigned int getId() const { return m_id; }
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
        m_position.setX(m_physics->position.x);
        m_position.setY(m_physics->position.y);
        m_position.setZ(m_physics->position.z);

        m_transform->setTranslation(m_position);
    }

    void updateVelocity()
    {
        m_velocity.setX(m_physics->velocity.x);
        m_velocity.setY(m_physics->velocity.y);
        m_velocity.setZ(m_physics->velocity.z);
    }

    inline QVector3D diffPosition(CParticle *otherParticle)
    {
        return position() - otherParticle->position();
    }

    static inline QVector3D clFloatToVector(cl_float3 vec)
    {
        return {vec.x, vec.y, vec.z};
    }

    static inline cl_float3 QVectorToClFloat(QVector3D vec)
    {
        return {vec.x(), vec.y(), vec.z()};
    }

    static inline QVector3D diffPosition(cl_float3 a, cl_float3 b)
    {
        return clFloatToVector(a) - clFloatToVector(b);
    }


//TODOL urobit getre a settre
public: //attributes
    static constexpr float h = 0.0457f;    //0.25    //0.02 //0.045
    static constexpr float viscosity = 3.5f; // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
    static constexpr float mass = 0.02f; // kg
    static constexpr float gas_stiffness = 3.0f; //20.0 // 461.5  // Nm/kg is gas constant of water vapor
    static constexpr float rest_density = 998.29f; // kg/m^3 is rest density of water particle

private:
    Physics *m_physics;
    QVector3D m_position;
    QVector3D m_velocity;
    QVector3D m_acceleration;
    double m_density;
    double m_pressure;

    Qt3DExtras::QSphereMesh *m_mesh;
    Qt3DExtras::QPhongMaterial *m_material;

    unsigned int m_id;

};


#endif //WATERSURFACESIMULATION_PARTICLE_H
