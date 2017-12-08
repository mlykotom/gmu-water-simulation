#ifdef __JETBRAINS_IDE__ // so that IDE doesn't complain and provides some help
#define __kernel
#define __global
#define __local
#define __constant
#endif

#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

typedef struct tag_ParticleCL
{
    float3 position;
    float3 velocity;
    float3 acceleration;
    double density;
    double pressure;
} ParticleCL;

#define GRAVITY_ACCELERATION (-9.80665f)
#define h  0.0457f    //0.25    //0.02 //0.045
#define viscosity  3.5f // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
#define mass  0.02f // kg
#define gas_stiffness  3.0f //20.0 // 461.5  // Nm/kg is gas constant of water vapor
#define rest_density  998.29f // kg/m^3 is rest density of water particle


// TODO static things
double Wpoly6(double radiusSquared)
{
    double coefficient = 315.0 / (64.0 * M_PI * pow(h, 9));
    double hSquared = h * h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

__kernel void integration_step(__global ParticleCL *output, __global ParticleCL *input, int size, float dt)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        float3 newPosition = input[global_x].position + (input[global_x].velocity * dt) + (input[global_x].acceleration * dt * dt);

// alternative, but particle doesn't have force for now
//        particles[i].velocity += dt * particles[i].force / particles[i].density;
//        particles[i].position += dt * particles[i].velocity;

        output[global_x].position = newPosition;
        output[global_x].velocity = (newPosition - input[global_x].position) / dt;
    }
}

__kernel void density_pressure_step(__global ParticleCL *output, __global ParticleCL *input, int size)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        // for all particles
//        for (int i = global_x; i < global_x + size; i++) {
//            float3 distance = input[global_x].position - input[i].position;
//            float radiusSquared = dot(distance, distance);
//
//            if (radiusSquared <= h * h) {
//                output[global_x].density += Wpoly6(radiusSquared);
//            }
//        }

        double newDensity = input[global_x].density * mass;

        output[global_x].density = newDensity;
        output[global_x].pressure = gas_stiffness * (newDensity - rest_density);
    }
}