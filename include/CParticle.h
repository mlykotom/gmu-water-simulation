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
    {
        cl_float3 position;
        cl_float3 velocity;
        cl_float3 acceleration;
        cl_float density;
        cl_float pressure;
        cl_uint id;
    }Physics;
#else
    typedef struct __attribute__((aligned(16))) sPhysics
    {
        cl_float3 position;
        cl_float3 velocity;
        cl_float3 acceleration;
        cl_float density;
        cl_float pressure;
        cl_uint id;
    }Physics;
#endif
    explicit CParticle(unsigned int id, Qt3DCore::QEntity *rootEntity, QVector3D initialPosition = QVector3D(0, 0, 0));
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

    static inline cl_float3 qVectortoClFloat(QVector3D vec)
    {
        return{ vec.x(), vec.y(), vec.z() };
    }

    static inline QVector3D diffPosition(cl_float3 a, cl_float3 b)
    {
        return clFloatToVector(a) - clFloatToVector(b);
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

    unsigned int m_id;

};


#endif //WATERSURFACESIMULATION_PARTICLE_H
