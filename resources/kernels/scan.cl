// extensiony pro atomicke instrukce jsou potrebne pouze pro zarizeni s podporou OpenCL 1.0 s verzi OpenCL >= 1.1 neni potreba
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__kernel void reduce(__global int *result, int array_size, int offset)
{
    int global_x = (int)get_global_id(0);
    int global_w = (int)get_global_size(0);
    int local_x = (int)get_local_id(0);
    int local_w = (int)get_local_size(0);
    int group_x = (int)get_group_id(0);
    //===========================================================================================  

    if (((global_x + 1) % (offset << 1) == 0) && (global_x < array_size) && (global_x - offset >= 0))
        result[global_x] = result[global_x] + result[global_x - offset];

    if (global_x == global_w - 1)
    {
        result[array_size - 1] = 0;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

}

__kernel void down_sweep(__global int *result, int array_size, int offset)
{
    int global_x = (int)get_global_id(0);
    int global_w = (int)get_global_size(0);
    //=========================================================================================== 

    if (((global_x + 1) % offset == 0) && (global_x < array_size))
    {
        int half_index = global_x - (offset >> 1);

        int tmp = result[global_x];
        result[global_x] += result[half_index];
        result[half_index] = tmp;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

}


__kernel void blelloch_scan(__global int *result, int array_size)
{
    int global_x = (int)get_global_id(0);
    int global_w = (int)get_global_size(0);
    //===========================================================================================  

    //Reduce
    for (int i = 1; i < global_w; i <<= 1)
    {
        if (((global_x + 1) % (i << 1) == 0) && (global_x < array_size) && (global_x - i >= 0))
        {
            result[global_x] = result[global_x] + result[global_x - i];
        }

        barrier(CLK_GLOBAL_MEM_FENCE);
    }



    ////down sweep
    result[array_size - 1] = 0;
    barrier(CLK_GLOBAL_MEM_FENCE);
    for (int i = array_size; i > 1; i >>= 1)
    {
        int half_index = global_x - ( i >> 1);
        if (((global_x + 1) % i == 0) && (global_x < array_size) )
        {
            int tmp = result[global_x];
            result[global_x] += result[half_index];
            result[half_index] = tmp;
        }

        barrier(CLK_GLOBAL_MEM_FENCE);

    }

}


//TODO: move to another file
//#pragma pack(push ,16) 
typedef struct  __attribute__((aligned(16))) tag_ParticleCL
{
    float3 position;
    float3 velocity;
    float3 acceleration;
    float density;
    float pressure;
    uint id;
    uint cell_id;
} ParticleCL;
//#pragma pack(pop) 


__constant float particle_mass = 0.02f; // kg
__constant float particle_h = 0.0457f;                //0.25 //0.02 //0.045
__constant float particle_h2 = 0.00208849f;                //0.25 //0.02 //0.045
__constant float viscosity = 3.5f;           // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
__constant float gas_stiffness = 3.0f;       //20.0 // 461.5  // Nm/kg is gas constant of water vapor
__constant float rest_density = 998.29f;     // kg/m^3 is rest density of water particle

__kernel void update_grid_positions(__global ParticleCL *particles, __global int *positions, int particles_count, int3 grid_size, float3 halfCellSize)
{
    int global_x = (int)get_global_id(0);

    if (global_x < particles_count)
    {
        //this division is really bad...
        float3 newGridPosition = (particles[global_x].position + halfCellSize) / particle_h;

        int x = (int) floor(newGridPosition.x);
        int y = (int) floor(newGridPosition.y);
        int z = (int) floor(newGridPosition.z);

        if (x < 0) {
            x = 0;
        }
        else if (x >= grid_size.x) {
            x = grid_size.x - 1;
        }

        if (y < 0) {
            y = 0;
        }
        else if (y >= grid_size.y) {
            y = grid_size.y - 1;
        }

        if (z < 0) {
            z = 0;
        }
        else if (z >= grid_size.z) {
            z = grid_size.z - 1;
        }

        int cell_index = x + y * grid_size.x + z * grid_size.x * grid_size.y;

        if ((cell_index < (grid_size.x * grid_size.y * grid_size.z)) && (cell_index >= 0))
        {
            atomic_inc(&positions[cell_index]);
            particles[global_x].cell_id = cell_index;
        }

    }

    barrier(CLK_GLOBAL_MEM_FENCE);
}

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

__kernel void density_pressure_step(__global ParticleCL *particles, __global int *scan_array, __global int *sorted_indices, int size, int3 grid_size, float poly6_constant)
{
    int global_x = (int)get_global_id(0);

    if (global_x < size) {
        particles[global_x].density = 0.0;

        int z = particles[global_x].cell_id / grid_size.z;
        int zRest = particles[global_x].cell_id - z * grid_size.z;
        int y = zRest / grid_size.y;
        int yRest = zRest - y * grid_size.y;
        int x = yRest;

        // for all neighbors particles
        for (int offsetX = -1; offsetX <= 1; offsetX++) 
        {
            if (x + offsetX < 0) continue;
            if (x + offsetX >= grid_size.x) break;

            for (int offsetY = -1; offsetY <= 1; offsetY++) 
            {
                if (y + offsetY < 0) continue;
                if (y + offsetY >= grid_size.y) break;

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
                {
                    if (z + offsetZ < 0) continue;
                    if (z + offsetZ >= grid_size.z) break;


                    int gridIndex = particles[global_x].cell_id + offsetX + offsetY* grid_size.x + +offsetZ* grid_size.x*grid_size.y;

                    int particlesIndexFrom = scan_array[gridIndex];
                    int particlesIndexTo = scan_array[gridIndex + 1] - scan_array[gridIndex];

                    //sorted_indices[particlesIndexFrom];
                    //sorted_indices[particlesIndexTo];


                //    auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                    for (int i = particlesIndexFrom; i < particlesIndexTo; ++i)
                    {

                        float3 distance = particles[global_x].position - particles[sorted_indices[i]].position;
                        float radiusSquared = dot(distance, distance);

                        if (radiusSquared <= particle_h2)
                        {
                            particles[global_x].density += Wpoly6(radiusSquared, poly6_constant);
                        }
                    }
                }
            }
        }

        particles[global_x].density *= particle_mass;
        particles[global_x].pressure = gas_stiffness * (particles[global_x].density - rest_density);
    }
}


__kernel void forces_step(__global ParticleCL *particles, __global int *scan_array, __global int *sorted_indices, int size, int3 grid_size, 
                            float3 gravity, float spiky_constant, float viscosity_constant)
{
    int global_x = (int)get_global_id(0);

    if (global_x < size)
    {
        __private ParticleCL thisParticle = particles[global_x];
        __private float3 f_pressure = 0.0f, f_viscosity = 0.0f;

        int z = particles[global_x].cell_id / grid_size.z;
        int zRest = particles[global_x].cell_id - z * grid_size.z;
        int y = zRest / grid_size.y;
        int yRest = zRest - y * grid_size.y;
        int x = yRest;

        // for all neighbors particles
        for (int offsetX = -1; offsetX <= 1; offsetX++)
        {
            if (x + offsetX < 0) continue;
            if (x + offsetX >= grid_size.x) break;

            for (int offsetY = -1; offsetY <= 1; offsetY++)
            {
                if (y + offsetY < 0) continue;
                if (y + offsetY >= grid_size.y) break;

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
                {
                    if (z + offsetZ < 0) continue;
                    if (z + offsetZ >= grid_size.z) break;


                    int gridIndex = particles[global_x].cell_id + offsetX + offsetY* grid_size.x + +offsetZ* grid_size.x*grid_size.y;

                    int particlesIndexFrom = scan_array[gridIndex];
                    int particlesIndexTo = scan_array[gridIndex + 1] - scan_array[gridIndex];

                    //sorted_indices[particlesIndexFrom];
                    //sorted_indices[particlesIndexTo];


                    //    auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                    for (int i = particlesIndexFrom; i < particlesIndexTo; ++i)
                    {

                        float3 distance = thisParticle.position - particles[sorted_indices[i]].position;
                        float radiusSquared = dot(distance, distance);

                        if (radiusSquared <= particle_h2 && thisParticle.id != particles[sorted_indices[i]].id)
                        {
                            float3 spikyGradient = WspikyGradient(distance, radiusSquared, spiky_constant);
                            float viscosityLaplacian = WviscosityLaplacian(radiusSquared, viscosity_constant);

                            f_pressure += thisParticle.pressure / pow(thisParticle.density, 2) + thisParticle.pressure / pow(thisParticle.density, 2) * spikyGradient;
                            f_viscosity += (particles[sorted_indices[i]].velocity - thisParticle.velocity) * viscosityLaplacian / thisParticle.density;
                        }
                    }
                }
            }
        }


        f_pressure *= -particle_mass * thisParticle.density;
        f_viscosity *= viscosity * particle_mass;

        particles[global_x].acceleration = (f_pressure + f_viscosity + gravity * thisParticle.density) / thisParticle.density;
    }
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