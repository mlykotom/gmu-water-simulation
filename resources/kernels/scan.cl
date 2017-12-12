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



__constant float mass = 0.02f; // kg
__constant float h = 0.0457f;                //0.25 //0.02 //0.045
__constant float viscosity = 3.5f;           // 5.0 // 0.00089 // Ns/m^2 or Pa*s viscosity of water
__constant float gas_stiffness = 3.0f;       //20.0 // 461.5  // Nm/kg is gas constant of water vapor
__constant float rest_density = 998.29f;     // kg/m^3 is rest density of water particle

__kernel void update_grid_positions(__global ParticleCL *particles, __global int *positions, int particles_count, int3 grid_size, float3 halfCellSize)
{
    int global_x = (int)get_global_id(0);

    //volatile __global int* counterPtr = positions;
    //barrier(CLK_GLOBAL_MEM_FENCE);
    //barrier(CLK_LOCAL_MEM_FENCE);

    if (global_x < particles_count)
    {
        //this division is really bad...
        float3 newGridPosition = (particles[global_x].position + halfCellSize) / h;

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

                                             // TODO static things
float Wpoly6(double radiusSquared)
{
    double coefficient = 315.0 / (64.0 * M_PI * pow(h, 9)); // TODO static / one-time
    double hSquared = h * h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

float3 WspikyGradient(float3 diffPosition, float radiusSquared)
{
    float coefficient = -45.0 / (M_PI * pow(h, 6));  // TODO static / one-time
    float radius = sqrt(radiusSquared);
    float powCounted = pow(h - radius, 2);

    return coefficient * powCounted * diffPosition / radius;
}

float WviscosityLaplacian(double radiusSquared)
{
    double coefficient = 45.0 / (M_PI * pow(h, 6)); // TODO static / one-time
    double radius = sqrt(radiusSquared);

    return coefficient * (h - radius);
}

__kernel void density_pressure_step(__global ParticleCL *particles, int *scan, int *sorted_indices, int size, int3 grid_size)
{
    int global_x = (int)get_global_id(0);

    if (global_x < size) {
        particles[global_x].density = 0.0;

        // for all neighbors particles
        for (int offsetX = -1; offsetX <= 1; offsetX++) {
            if (x + offsetX < 0) continue;
            if (x + offsetX >= grid_size.x) break;

            for (int offsetY = -1; offsetY <= 1; offsetY++) {
                if (y + offsetY < 0) continue;
                if (y + offsetY >= grid_size.y) break;

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                    if (z + offsetZ < 0) continue;
                    if (z + offsetZ >= grid_size.z) break;


                    int gridIndex = particles[global_x].cell_id + offsetX + offsetY* grixSize.x + +offsetZ* grixSize.x*grixSize.y;

                    int particlesIndexFrom = scan[gridIndex];
                    int particlesIndexto = scan[gridIndex + 1] - scan[gridIndex];

                    //sorted_indices[particlesIndexFrom];
                    //sorted_indices[particlesIndexTo];


                //    auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                    for (int i = particlesIndexFrom; i < particlesIndexTo; ++i)
                    {

                        float3 distance = particles[global_x].position - particles[sorted_indices[i]].position;
                        float radiusSquared = dot(distance, distance);

                        if (radiusSquared <= h * h) {
                            particles[global_x].density += Wpoly6(radiusSquared);
                        }
                    }
        }

        particles[global_x].density *= mass;
        particles[global_x].pressure = gas_stiffness * (particles[global_x].density - rest_density);
    }
}