// extensiony pro atomicke instrukce jsou potrebne pouze pro zarizeni s podporou OpenCL 1.0 s verzi OpenCL >= 1.1 neni potreba
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__constant float particle_mass = 0.02f; // kg
__constant float particle_h = 0.0457f;                //0.25 //0.02 //0.045
__constant float particle_h2 = 0.00208849f;                //0.25 //0.02 //0.045
__constant float viscosity = 3.5f;           // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
__constant float gas_stiffness = 3.0f;       //20.0 // 461.5  // Nm/kg is gas constant of water vapor
__constant float rest_density = 998.29f;     // kg/m^3 is rest density of water particle


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


/**
* Verlet integration
* http://archive.gamedev.net/archive/reference/programming/features/verlet/
* @param output
* @param size
* @param dt
*/
__kernel void integration_step(__global ParticleCL *output, int size, float dt)
{
int global_x = (int)get_global_id(0);

if (global_x < size) {
__private ParticleCL tmp_particle = output[global_x];
__private float3 newPosition = tmp_particle.position + (tmp_particle.velocity * dt) + (tmp_particle.acceleration * dt * dt);

tmp_particle.velocity = (newPosition - tmp_particle.position) / dt;
tmp_particle.position = newPosition;

output[global_x] = tmp_particle;
}
}