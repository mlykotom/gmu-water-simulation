#ifdef __JETBRAINS_IDE__ // so that IDE doesn't complain and provides some help
#define __kernel
#define __global
#define __local
#define __constant
#endif

#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

typedef struct __attribute__((aligned(16))) tag_ParticleCL
{
    float3 position;
    float3 velocity;
    float3 acceleration;
    float density;
    float pressure;
    uint id;
} ParticleCL;

__constant float particle_mass = 0.02f; // kg
__constant float particle_h = 0.0457f;                //0.25 //0.02 //0.045
__constant float particle_h2 = 0.00208849f;                //0.25 //0.02 //0.045
__constant float viscosity = 3.5f;           // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
__constant float gas_stiffness = 3.0f;       //20.0 // 461.5  // Nm/kg is gas constant of water vapor
__constant float rest_density = 998.29f;     // kg/m^3 is rest density of water particle

float Wpoly6(double radiusSquared, float poly6_constant)
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

__kernel void density_pressure_step(__global ParticleCL *output, int size, float poly6_constant)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        output[global_x].density = 0.0;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = output[global_x].position - output[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2) {
                output[global_x].density += Wpoly6(radiusSquared, poly6_constant);
            }
        }

        output[global_x].density *= particle_mass;
        output[global_x].pressure = gas_stiffness * (output[global_x].density - rest_density);
    }
}

__kernel void forces_step(__global ParticleCL *output, int size, float3 gravity, float spiky_constant, float viscosity_constant)
{
    int global_x = (int) get_global_id(0);
    if (global_x < size) {
        float3 f_gravity = gravity * output[global_x].density;
        float3 f_pressure, f_viscosity;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = output[global_x].position - output[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2) {
                float3 spikyGradient = WspikyGradient(distance, radiusSquared, spiky_constant);
                float viscosityLaplacian = WviscosityLaplacian(radiusSquared, viscosity_constant);

                if (output[global_x].id != output[i].id) {
                    f_pressure += output[global_x].pressure / pow(output[global_x].density, 2) + output[global_x].pressure / pow(output[global_x].density, 2) * spikyGradient;
                    f_viscosity += (output[i].velocity - output[global_x].velocity) * viscosityLaplacian / output[global_x].density;
                }
            }
        }

        f_pressure *= -particle_mass * output[global_x].density;
        f_viscosity *= viscosity * particle_mass;

        float3 f_total = (f_pressure + f_viscosity + f_gravity) / output[global_x].density;

        output[global_x].acceleration = f_total;
    }
}

__kernel void integration_step(__global ParticleCL *output, int size, float dt)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
// alternative, but particle doesn't have force for now
//        particles[i].velocity += dt * particles[i].force / particles[i].density;
//        particles[i].position += dt * particles[i].velocity;

        float3 newPosition = output[global_x].position + (output[global_x].velocity * dt) + (output[global_x].acceleration * dt * dt);
        output[global_x].velocity = (newPosition - output[global_x].position) / dt;
        output[global_x].position = newPosition;
    }
}