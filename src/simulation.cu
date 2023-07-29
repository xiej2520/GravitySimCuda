#include "simulation.hpp"

#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/copy.h>

#include "helper_math.cuh"
#include <thrust/host_vector.h>

namespace gravitysim {

using namespace DirectX;

void Simulation::transfer_masses_to_gpu() {
  printf("here\n");
  //gpu_data.masses.resize(masses.size());
  printf("here1\n");
  //thrust::copy(masses.begin(), masses.end(), gpu_data.masses.begin());
  thrust::host_vector<float> h = masses;
  printf("here2\n");
  thrust::device_vector<float> d = masses;
  printf("here3\n");
  //gpu_data.masses = masses;
  printf("here2\n");
}

void Simulation::transfer_kinematics_to_gpu() {
  size_t n = masses.size();
  gpu_data.positions.resize(n);
  gpu_data.vels.resize(n);
  gpu_data.accs.resize(n);
  thrust::copy(positions.begin(), positions.end(), reinterpret_cast<XMFLOAT3 *>(thrust::raw_pointer_cast(gpu_data.positions.data())));
  thrust::copy(vels.begin(), vels.end(), reinterpret_cast<XMFLOAT3 *>(thrust::raw_pointer_cast(gpu_data.vels.data())));
  thrust::copy(accs.begin(), accs.end(), reinterpret_cast<XMFLOAT3 *>(thrust::raw_pointer_cast(gpu_data.accs.data())));
}

void Simulation::transfer_gpu_kinematics_to_cpu() {
  thrust::copy(gpu_data.positions.begin(), gpu_data.positions.end(), reinterpret_cast<float3 *>(positions.data()));
  thrust::copy(gpu_data.vels.begin(), gpu_data.vels.end(), reinterpret_cast<float3 *>(vels.data()));
  thrust::copy(gpu_data.accs.begin(), gpu_data.accs.end(), reinterpret_cast<float3 *>(accs.data()));
}


__global__ void gpu_particle_particle(float *masses, float3 *positions, float3 *vels, float3 *accs, int n, float G) {
  int i = threadIdx.x + blockIdx.x + blockDim.x;
  if (i >= n) return;
  float3 p1 = positions[i];
  
  // maybe use 2d thread but could have race condition
  for (int j=0; j<n; j++) {
    if (i == j) continue;
    float3 p2 = positions[j];
    float3 diff = p2 - p1;
    
    float acc_magnitude = G * masses[j] / dot(diff, diff);
    
    float3 unit_diff = normalize(diff);
    float3 acc_delta = acc_magnitude * unit_diff;
    
    accs[i] += acc_delta;
  }
}

void x(std::vector<float> &m) {
  printf("1\n");
  thrust::host_vector<float> h = m;
  printf("2\n");
  thrust::device_vector<float> g = m;
  printf("3\n");
}

void Simulation::calc_accs_gpu_particle_particle() {
  x(masses);
  transfer_masses_to_gpu();
  printf("transfered masses\n");
  transfer_kinematics_to_gpu();
  printf("transfered kinematics\n");
  
  size_t n = gpu_data.masses.size();
  int block_size = 256;
  int num_blocks = (n + block_size - 1) / block_size;

  gpu_particle_particle<<<block_size, num_blocks>>>(thrust::raw_pointer_cast(gpu_data.masses.data()),
      thrust::raw_pointer_cast(gpu_data.positions.data()),
      thrust::raw_pointer_cast(gpu_data.vels.data()),
      thrust::raw_pointer_cast(gpu_data.accs.data()),
      n,
      G
  );
  printf("finished running\n");
  transfer_gpu_kinematics_to_cpu();
  printf("transfered to cpu\n");
}


} // namespace gravitysim
