#pragma once

#include <cuda_runtime.h>
#include <thrust/device_vector.h>

struct GPUSimData {
  thrust::device_vector<float> masses;
  thrust::device_vector<float3> positions;
  thrust::device_vector<float3> vels;
  thrust::device_vector<float3> accs;
};
