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

__host__ void Simulation::transfer_mus_to_gpu() {
  gpu_data.mus = mus;
}

__host__ void Simulation::transfer_kinematics_to_gpu() {
  gpu_data.positions.resize(num_bodies);
  gpu_data.vels.resize(num_bodies);
  gpu_data.accs.assign(num_bodies, make_float3(0));

  // the data has the same layout
  // I have not found a better method of doing this
  float3 *dev_ptr = thrust::raw_pointer_cast(gpu_data.positions.data());
  vec3f *host_ptr = thrust::raw_pointer_cast(positions.data());
  cudaMemcpy(dev_ptr, host_ptr, num_bodies * sizeof(vec3f), cudaMemcpyHostToDevice);

  dev_ptr = thrust::raw_pointer_cast(gpu_data.vels.data());
  host_ptr = thrust::raw_pointer_cast(vels.data());
  cudaMemcpy(dev_ptr, host_ptr, num_bodies * sizeof(vec3f), cudaMemcpyHostToDevice);
}

__host__ void Simulation::transfer_gpu_kinematics_to_cpu() {
  // data has the same layout
  // I have not found a better way of doing this
  thrust::copy(gpu_data.positions.begin(), gpu_data.positions.end(), reinterpret_cast<float3 *>(positions.data()));
  thrust::copy(gpu_data.vels.begin(), gpu_data.vels.end(), reinterpret_cast<float3 *>(vels.data()));
}

__host__ void Simulation::transfer_gpu_positions_to_cpu() {
  // data has the same layout
  // I have not found a better way of doing this
  thrust::copy(gpu_data.positions.begin(), gpu_data.positions.end(), reinterpret_cast<float3 *>(positions.data()));
}

__global__ void gpu_particle_particle(float *mus, float3 *positions, float3 *vels, float3 *accs, size_t n, float G, float time_step) {
  int i = blockIdx.x * blockDim.x + threadIdx.x; // thread id
  if (i >= n) return;
  float3 p1 = positions[i];
  
  // maybe use 2d thread but could have race conditions
  // parallelized calculation of acceleration between all bodies
  for (int j = 0; j < n; j++) {
    if (i == j) continue;
    float3 p2 = positions[j];
    float3 diff = p2 - p1;
    
    float acc_magnitude = mus[j] / dot(diff, diff);
    
    float3 unit_diff = normalize(diff);
    float3 acc_delta = acc_magnitude * unit_diff;
    
    accs[i] += acc_delta;
  }
  vels[i] += accs[i] * time_step;
}

__global__ void gpu_step(float3 *positions, float3 *vels, size_t n, float time_step) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= n) return;
  positions[i] += vels[i] * time_step;
}

__host__ void Simulation::calc_accs_gpu_particle_particle() {
  unsigned int block_size = 256;
  unsigned int num_blocks = (num_bodies + block_size - 1) / block_size;

  gpu_data.accs.assign(num_bodies, make_float3(0));

  gpu_particle_particle<<<block_size, num_blocks>>>(
      thrust::raw_pointer_cast(gpu_data.mus.data()),
      thrust::raw_pointer_cast(gpu_data.positions.data()),
      thrust::raw_pointer_cast(gpu_data.vels.data()),
      thrust::raw_pointer_cast(gpu_data.accs.data()),
      num_bodies,
      G,
      time_step
  );
  
  checkCudaErrors(cudaDeviceSynchronize());

  gpu_step<<<block_size, num_blocks>>>(
    thrust::raw_pointer_cast(gpu_data.positions.data()),
    thrust::raw_pointer_cast(gpu_data.vels.data()),
    num_bodies,
    time_step
  );
  checkCudaErrors(cudaDeviceSynchronize());
}


} // namespace gravitysim
