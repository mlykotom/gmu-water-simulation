//#if defined(__JETBRAINS_IDE__) && !defined(__kernel) // so that IDE doesn't complain and provides some help
//#define __kernel
//#define __global
//#define __local
//#define __constant
//#define __private
//#endif

__kernel void density_pressure_step(__global ParticleCL *particles, int size, float poly6_constant)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        float3 thisPosition = particles[global_x].position;
        float density = 0.0f;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = thisPosition - particles[i].position;
            float radiusSquared = dot(distance, distance);
            if (radiusSquared <= particle_h2) {
                density += Wpoly6(radiusSquared, poly6_constant);
            }
        }

        density *= particle_mass;

        particles[global_x].density = density;
        particles[global_x].pressure = gas_stiffness * (density - rest_density);
    }
}

__kernel void forces_step(__global ParticleCL *particles, int size, float3 gravity, float spiky_constant, float viscosity_constant)
{
    int global_x = (int) get_global_id(0);
    if (global_x < size) {
        __private float3 f_pressure = 0.0f, f_viscosity = 0.0f;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = particles[global_x].position - particles[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2 && particles[global_x].id != particles[i].id) {
                float3 spikyGradient = WspikyGradient(distance, radiusSquared, spiky_constant);
                float viscosityLaplacian = WviscosityLaplacian(radiusSquared, viscosity_constant);

                f_pressure += ((particles[global_x].pressure / pow(particles[global_x].density, 2)) + (particles[i].pressure / pow(particles[i].density, 2))) * spikyGradient;
                f_viscosity += (particles[i].velocity - particles[global_x].velocity) * viscosityLaplacian / particles[i].density;
            }
        }

        f_pressure *= -particle_mass * particles[global_x].density;
        f_viscosity *= viscosity * particle_mass;

        particles[global_x].acceleration = (f_pressure + f_viscosity + gravity * particles[global_x].density) / particles[global_x].density;
    }
}