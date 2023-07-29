#include "simulation.hpp"

#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/copy.h>

#include "helper_math.cuh"
#include <thrust/host_vector.h>

namespace gravitysim {

using namespace DirectX;

#define checkCudaErrors(val) check_cuda((val), #val, __FILE__, __LINE__)
void check_cuda(cudaError_t result, char const *const func, const char *const file, int const line) {
  if (result != 0) {
    std::cerr << "CUDA error = " << static_cast<uint32_t>(result) << " at " <<
      file << ":" << line << " '" << func << "' \n";
    cudaDeviceReset();
    exit(99);
  }
}

__host__ void Simulation::transfer_masses_to_gpu() {
  gpu_data.masses = masses;
}

__host__ void Simulation::transfer_kinematics_to_gpu() {
  size_t n = masses.size();
  gpu_data.positions.resize(n);
  gpu_data.vels.resize(n);
  gpu_data.accs.assign(n, make_float3(0));

  float3 *dev_ptr = thrust::raw_pointer_cast(gpu_data.positions.data());
  vec3f *host_ptr = thrust::raw_pointer_cast(positions.data());
  cudaMemcpy(dev_ptr, host_ptr, n * sizeof(vec3f), cudaMemcpyHostToDevice);

  dev_ptr = thrust::raw_pointer_cast(gpu_data.vels.data());
  host_ptr = thrust::raw_pointer_cast(vels.data());
  cudaMemcpy(dev_ptr, host_ptr, n * sizeof(vec3f), cudaMemcpyHostToDevice);
}

__host__ void Simulation::transfer_gpu_kinematics_to_cpu() {
  thrust::copy(gpu_data.positions.begin(), gpu_data.positions.end(), reinterpret_cast<float3 *>(positions.data()));
  thrust::copy(gpu_data.vels.begin(), gpu_data.vels.end(), reinterpret_cast<float3 *>(vels.data()));
}


__global__ void gpu_particle_particle(float *masses, float3 *positions, float3 *accs, size_t n, float G) {
  int i = blockIdx.x * blockDim.x + threadIdx.x; // thread id
  if (i >= n) return;
  float3 p1 = positions[i];
  
  // maybe use 2d thread but could have race condition
  for (int j = 0; j < n; j++) {
    if (i == j) continue;
    float3 p2 = positions[j];
    float3 diff = p2 - p1;
    
    float acc_magnitude = G * masses[j] / dot(diff, diff);
    
    float3 unit_diff = normalize(diff);
    float3 acc_delta = acc_magnitude * unit_diff;
    
    accs[i] += acc_delta;
  }
}

__global__ void gpu_step(float3 *positions, float3 *vels, float3 *accs, size_t n, float time_step) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= n) return;
    vels[i] += accs[i] * time_step;
    positions[i] += vels[i] * time_step;
}

__host__ void Simulation::calc_accs_gpu_particle_particle() {
  size_t n = gpu_data.masses.size();
  size_t block_size = 256;
  size_t num_blocks = (n + block_size - 1) / block_size;

  gpu_data.accs.assign(n, make_float3(0));

  gpu_particle_particle<<<block_size, num_blocks>>>(
      thrust::raw_pointer_cast(gpu_data.masses.data()),
      thrust::raw_pointer_cast(gpu_data.positions.data()),
      thrust::raw_pointer_cast(gpu_data.accs.data()),
      n,
      G
  );
  
  checkCudaErrors(cudaDeviceSynchronize());

  gpu_step<<<block_size, num_blocks>>>(
    thrust::raw_pointer_cast(gpu_data.positions.data()),
    thrust::raw_pointer_cast(gpu_data.vels.data()),
    thrust::raw_pointer_cast(gpu_data.accs.data()),
    n,
    time_step
  );
  checkCudaErrors(cudaDeviceSynchronize());
}


} // namespace gravitysim
