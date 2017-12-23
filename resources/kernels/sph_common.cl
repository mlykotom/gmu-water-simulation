#if defined(__JETBRAINS_IDE__) && !defined(__kernel) // so that IDE doesn't complain and provides some help
#define __kernel
#define __global
#define __local
#define __constant
#define __private
#endif


// extensiony pro atomicke instrukce jsou potrebne pouze pro zarizeni s podporou OpenCL 1.0 s verzi OpenCL >= 1.1 neni potreba
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__constant float particle_mass = 0.02f;         // kg
__constant float particle_h = 0.0457f;          // 0.25 //0.02 //0.045
__constant float particle_h2 = 0.00208849f;     // half of top
__constant float viscosity = 3.5f;              // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
__constant float gas_stiffness = 3.0f;          // 20.0 // 461.5  // Nm/kg is gas constant of water vapor
__constant float rest_density = 998.29f;        // kg/m^3 is rest density of water particle
__constant float wall_k = 10000.0f;             // wall spring constant
__constant float wall_damping = -0.9f;          // wall damping constant

typedef struct __attribute__((aligned(16))) tag_WallCL
{
    float3 normal;
    float3 position;
} WallCL;

typedef struct __attribute__((aligned(16))) tag_ParticleCL
{
    float3 position;
    float3 velocity;
    float3 acceleration;
    int3 grid_position;
    float density;
    float pressure;
    uint id;
    uint cell_id;
} ParticleCL;

float Wpoly6(float radiusSquared, float poly6_constant)
{
    return poly6_constant * pow(particle_h2 - radiusSquared, 3);
}

float3 WspikyGradient(float3 diffPosition, float radiusSquared, float spiky_constant)
{
    float radius = sqrt(radiusSquared);
    return spiky_constant * pow(particle_h - radius, 2) * diffPosition / radius;
}

float WviscosityLaplacian(float radiusSquared, float viscosity_constant)
{
    return viscosity_constant * (particle_h - sqrt(radiusSquared));
}


__kernel void walls_collision(__global ParticleCL *particles, __global WallCL *walls, int size, int wallsSize)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        float3 acceleration = (float3)(1.0f, 1.0f, 0.0f);

        for (int i = 0; i < wallsSize; i++) {
            float3 inverseNormal = walls[i].normal * (-1.0f);

            float d = dot(walls[i].position - particles[global_x].position, inverseNormal) + 0.01f;
            if (d > 0.0f) {
                acceleration += wall_k * inverseNormal * d;
                acceleration += wall_damping * dot(particles[global_x].velocity, inverseNormal) * inverseNormal;
            }
        }

        particles[global_x].acceleration += acceleration;
    }
}


/**
* Verlet integration
* http://archive.gamedev.net/archive/reference/programming/features/verlet/
* @param output
* @param size
* @param dt
*/
__kernel void integration_step(__global ParticleCL *particles, int size, float dt)
{
    int global_x = (int) get_global_id(0);
    if (global_x < size) {
        __private float3 newPosition = particles[global_x].position + (particles[global_x].velocity * dt) + (particles[global_x].acceleration * dt * dt);

        particles[global_x].velocity = (newPosition - particles[global_x].position) / dt;
        particles[global_x].position = newPosition;
    }
}