// extensiony pro atomicke instrukce jsou potrebne pouze pro zarizeni s podporou OpenCL 1.0 s verzi OpenCL >= 1.1 neni potreba
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__kernel void blelloch_scan(__global int *input, int array_size, __global int *result, volatile __local int *tmp_a)
{
    int global_x = (int)get_global_id(0);
    int local_x = (int)get_local_id(0);
    int local_w = (int)get_local_size(0);
//===========================================================================================  


//    result[0] = 1000;

    atomic_add(result, 10);

    //tmp_a[local_x] = global_x >= array_size ? 0 : input[global_x];
    //barrier(CLK_LOCAL_MEM_FENCE);

    ////reduce
    //for (int i = 1; i < local_w; i <<= 1) {        

    //    if ((local_x % (i << 1) == 0) && (global_x + i < array_size))
    //        tmp_a[local_x] += tmp_a[local_x + i];

    //    barrier(CLK_LOCAL_MEM_FENCE);
    //}

    //barrier(CLK_LOCAL_MEM_FENCE);

    //if (global_x == 0)
    //{
    //    *result = tmp_a[local_x];
    //   // atomic_add(result,10);
    //}
}