
__kernel void density_pressure_step(__global ParticleCL *output, int size, float poly6_constant)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        __private ParticleCL thisParticle = output[global_x];
        thisParticle.density = 0.0f;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = thisParticle.position - output[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2) {
                thisParticle.density += Wpoly6(radiusSquared, poly6_constant);
            }
        }

        thisParticle.density *= particle_mass;
        thisParticle.pressure = gas_stiffness * (thisParticle.density - rest_density);

        output[global_x] = thisParticle;
    }
}

__kernel void GLOBAL_density_pressure_step(__global ParticleCL *output, int size, float poly6_constant)
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
        __private ParticleCL thisParticle = output[global_x];
        __private float3 f_pressure = 0.0f, f_viscosity = 0.0f;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = thisParticle.position - output[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2 && thisParticle.id != output[i].id) {
                float3 spikyGradient = WspikyGradient(distance, radiusSquared, spiky_constant);
                float viscosityLaplacian = WviscosityLaplacian(radiusSquared, viscosity_constant);

                f_pressure += thisParticle.pressure / pow(thisParticle.density, 2) + thisParticle.pressure / pow(thisParticle.density, 2) * spikyGradient;
                f_viscosity += (output[i].velocity - thisParticle.velocity) * viscosityLaplacian / thisParticle.density;
            }
        }

        f_pressure *= -particle_mass * thisParticle.density;
        f_viscosity *= viscosity * particle_mass;

        output[global_x].acceleration = (f_pressure + f_viscosity + gravity * thisParticle.density) / thisParticle.density;
    }
}