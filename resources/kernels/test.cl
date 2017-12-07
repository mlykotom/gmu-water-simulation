#ifdef __JETBRAINS_IDE__ // so that IDE doesn't complain and provides some help
#define __kernel
#define __global
#define __local
#define __constant
#endif

#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

__constant float3 dt = float3(0.01, 0.01, 0.01);

typedef struct tag_ParticleCL
{
    float3 position;
    float3 velocity;
    float3 acceleration;
} ParticleCL;

__kernel void test(__global ParticleCL *output, int size)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        float3 newPosition = output[global_x].position + (output[global_x].velocity * dt) + output[global_x].acceleration * dt * dt;

        output[global_x].velocity = (newPosition - output[global_x].position) / dt;
        output[global_x].position = newPosition;

        if (global_x == 0) {
//            printf("%.4f\n", output[global_x].velocity);
        }
    }
}