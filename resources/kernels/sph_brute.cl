
__kernel void density_pressure_step(__global ParticleCL *output, int size, float poly6_constant)
{
    int global_x = (int) get_global_id(0);

    if (global_x < size) {
        float3 thisPosition = output[global_x].position;
        float density = 0.0f;

        // for all neighbors particles
        for (int i = 0; i < size; i++) {
            float3 distance = thisPosition - output[i].position;
            float radiusSquared = dot(distance, distance);

            if (radiusSquared <= particle_h2) {
                density += Wpoly6(radiusSquared, poly6_constant);
            }
        }

        density *= particle_mass;

        output[global_x].density = density;
        output[global_x].pressure = gas_stiffness * (density - rest_density);
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