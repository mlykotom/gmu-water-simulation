
__kernel void increment_local_scans(__global int *result, __global int *sums, int array_size)
{
    int global_x = (int)((get_global_id(0) + 1) << 1) - 1;
    int global_w = (int)get_global_size(0);
    int local_x = (int)((get_local_id(0) + 1) << 1) - 1;
    int local_w = (int)(get_local_size(0) << 1);
    int group_x = (int)get_group_id(0);
    int g_num = (int)get_num_groups(0);
    //=========================================================================================== 

    if (global_x < array_size && global_x -1 >= 0)
    {
        result[global_x] += sums[group_x];
        result[global_x-1] += sums[group_x];
    }
}

__kernel void scan_local(__global int *result, __global int * sums, int array_size, volatile __local int *tmp)
{
    int global_x = (int)((get_global_id(0) + 1) << 1) -1;
    int global_w = (int)get_global_size(0);
    int local_x = (int)((get_local_id(0)+1) << 1) -1;
    int local_w = (int) (get_local_size(0) << 1);
    int group_x = (int)get_group_id(0);
    int g_num = (int)get_num_groups(0);
    //===========================================================================================  

    //if (group_x * local_w >= array_size)
    //    return;

    tmp[local_x] = global_x >= array_size ? 0 : result[global_x];
    tmp[local_x-1] = (global_x-1) >= array_size ? 0 : result[global_x-1];

    barrier(CLK_LOCAL_MEM_FENCE);

    //reduce
    for (int i = 1; i < local_w; i <<= 1)
    {
        if (((local_x + 1) % (i << 1) == 0) && (local_x < local_w) && (local_x - i >= 0))
        {
            tmp[local_x] = tmp[local_x] + tmp[local_x - i];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (local_x == local_w - 1)
    {
        sums[group_x] = tmp[local_x];
        tmp[local_x] = 0;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    //down sweep
    for (int i = local_w; i > 1; i >>= 1)
    {
        int half_index = local_x - (i >> 1);
        if (((local_x + 1) % i == 0) && (local_x < local_w))
        {
            int val = tmp[local_x];
            tmp[local_x] += tmp[half_index];
            tmp[half_index] = val;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    result[global_x] = tmp[local_x];
    result[global_x-1] = tmp[local_x-1];

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

__kernel void update_grid_positions(__global ParticleCL *particles, __global int *positions, int particles_count, int3 grid_size, float3 half_box_size)
{
    int global_x = (int)get_global_id(0);

    if (global_x < particles_count)
    {
        //this division is really bad...
        float3 newGridPosition = (particles[global_x].position + half_box_size) / particle_h;

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
            particles[global_x].grid_position.xyz = (int3)(x, y, z);

        }

    }

    barrier(CLK_GLOBAL_MEM_FENCE);
}


__kernel void density_pressure_step(__global ParticleCL *particles, __global int *scan_array, __global int *sorted_indices, int size, int3 grid_size, float poly6_constant)
{
    int global_x = (int)get_global_id(0);

    if (global_x < size) 
    {

        int gird_array_size = grid_size.x * grid_size.y *grid_size.z;

        __private ParticleCL tmp_particle = particles[global_x];

        tmp_particle.density = 0.0;

        int x = tmp_particle.grid_position.x;
        int y = tmp_particle.grid_position.y;
        int z = tmp_particle.grid_position.z;

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


                    int gridIndex = x + offsetX + ( (offsetY + y)* grid_size.x) + ((offsetZ+z)* grid_size.x*grid_size.y);
                    int particlesIndexFrom = scan_array[gridIndex];
                    int particlesIndexTo = (gridIndex + 1) < gird_array_size ? (scan_array[gridIndex + 1] ) : (size);

                    for (int i = particlesIndexFrom; i < particlesIndexTo; ++i)
                    {

                        float3 distance = tmp_particle.position - particles[sorted_indices[i]].position;
                        float radiusSquared = dot(distance, distance);

                        if (radiusSquared <= particle_h2)
                        {
                            tmp_particle.density += Wpoly6(radiusSquared, poly6_constant);
                        }
                    }
                }
            }
        }

        tmp_particle.density *= particle_mass;
        tmp_particle.pressure = gas_stiffness * (tmp_particle.density - rest_density);
        particles[global_x] = tmp_particle;
    }


  //  barrier(CLK_GLOBAL_MEM_FENCE);

}


__kernel void forces_step(__global ParticleCL *particles, __global int *scan_array, __global int *sorted_indices, int size, int3 grid_size, 
                            float3 gravity, float spiky_constant, float viscosity_constant)
{
    int global_x = (int)get_global_id(0);

    if (global_x < size)
    {
        __private ParticleCL thisParticle = particles[global_x];
        __private float3 f_pressure = 0.0f, f_viscosity = 0.0f;

        int gird_array_size = grid_size.x * grid_size.y *grid_size.z;

        int x = particles[global_x].grid_position.x;
        int y = particles[global_x].grid_position.y;
        int z = particles[global_x].grid_position.z;

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


                    int gridIndex = x + offsetX + ((offsetY + y)* grid_size.x) + ((offsetZ + z)* grid_size.x*grid_size.y);
                    int particlesIndexFrom = scan_array[gridIndex];
                    int particlesIndexTo = (gridIndex + 1) < gird_array_size ? (scan_array[gridIndex + 1]) : (size);

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

    //barrier(CLK_GLOBAL_MEM_FENCE);

}