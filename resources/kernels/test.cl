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

__kernel void update_density_pressure(__global ParticleCL *output, __global ParticleCL *input, int size)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {

    }
}