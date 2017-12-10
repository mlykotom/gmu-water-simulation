// extensiony pro atomicke instrukce jsou potrebne pouze pro zarizeni s podporou OpenCL 1.0 s verzi OpenCL >= 1.1 neni potreba
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__kernel void blelloch_scan(__global int *input, int array_size, __global int *result, volatile __local int *tmp_a)
{
    int global_x = (int)get_global_id(0);
    int global_w = (int)get_global_size(0);
    int local_x = (int)get_local_id(0);
    int local_w = (int)get_local_size(0);
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
typedef struct tag_ParticleCL
{
    ulong id;
    float3 position;
    float3 velocity;
    float3 acceleration;
    float density;
    float pressure;
} ParticleCL;

__kernel void update_grid_positions(__global ParticleCL *particles, __global int *positions, int particles_count, int3 grid_size, float3 halfCellSize, float h)
{
    int global_x = (int)get_global_id(0);

    volatile __global int* counterPtr = positions;
    barrier(CLK_GLOBAL_MEM_FENCE);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (global_x < particles_count)
    {
        //this division is really bad...
        float3 newGridPosition = (particles[global_x].position + halfCellSize) / h;

        int x = (int) floor(newGridPosition.x);
        int y = (int) floor(newGridPosition.y);
        int z = (int) floor(newGridPosition.z);

         printf("global index: %d, particle index: %d \n", global_x, particles[global_x].id);

        //printf("x: %d, y: %d, z: %d  \n", x, y, z);
       // printf("global index: %d \n", global_x);
       // printf("pos: %d, %d, %d \n", particles[global_x].position.x, particles[global_x].position.y, particles[global_x].position.z);

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

  
        //todo: nahradit atomic_inc
        //atomic_add(&positions[x + y * grid_size.y + z * grid_size.y * grid_size.z], 1);

        //A[depth][col][row]
        //(x*grid_size.y + y) * grid_size.z + z        
        //int index = (x*grid_size.y + y) * grid_size.z + z;

        int index = x + y * grid_size.y + z * grid_size.y * grid_size.z;

        bool b = ( (index < (grid_size.x * grid_size.y * grid_size.z)) && (index >= 0) );
        //if (index < (grid_size.x*grid_size.y*grid_size.z) && index >= 0)
        //{
        //    //atomic_add(&positions[0], 1);
        //    positions[0] = 1;
        //}

        //if (global_x % 2)
        //if (b)
        //if((global_x < (grid_size.x * grid_size.y * grid_size.z)) && (global_x >= 0))
        //{
        //    atomic_inc(&positions[global_x]);

        //}

        printf("index: %d, x: %d, y: %d, z: %d  \n", index,x,y,z);

        if ((index < (grid_size.x * grid_size.y * grid_size.z)) && (index >= 0))
        {
            atomic_inc(&positions[index]);

        }


        //atomic_add(&positions[0], 1);
       // positions[0] = grid_size.x * grid_size.y * grid_size.z;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

}