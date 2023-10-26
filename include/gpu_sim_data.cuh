#pragma once

#include <cuda_runtime.h>
#include <thrust/device_vector.h>

// holds data for gpu simulation
struct GPUSimData {
  thrust::device_vector<float> mus;
  thrust::device_vector<float3> positions;
  thrust::device_vector<float3> vels;
  thrust::device_vector<float3> accs;
};
