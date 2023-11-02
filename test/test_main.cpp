#include <gtest/gtest.h>

#include "simulation.hpp"

TEST(Hello, BasicAssertions) {
  EXPECT_STRNE("hello", "world");
  EXPECT_EQ(7 * 6, 42);
}

TEST(GravitySim, set_COM_frame) {
  std::vector<float> masses = {2e6, 1e6, 4e5};
  std::vector<DirectX::XMFLOAT3> positions = {{0, 0, 0}, {10, 10, 10}, {0, 100, -12}};
  std::vector<DirectX::XMFLOAT3> vels = {{0, 0, 0}, {3, 3, 3}, {-4, -6, 9}};
  gravitysim::Simulation simulation(masses, positions, vels, 0.01f);
  
  simulation.set_COM_frame();
  
  const auto &v = simulation.get_vels();
  
  DirectX::XMFLOAT3 total_momentum{};
  for (int i=0; i<masses.size(); i++) {
    total_momentum.x += masses[i] * v[i].x;
    total_momentum.y += masses[i] * v[i].y;
    total_momentum.z += masses[i] * v[i].z;
  }
  EXPECT_NEAR(total_momentum.x, 0.0f, 10.0f);
  EXPECT_NEAR(total_momentum.y, 0.0f, 10.0f);
  EXPECT_NEAR(total_momentum.z, 0.0f, 10.0f);
  printf("Total momentum: %g %g %g\n", total_momentum.x, total_momentum.y, total_momentum.z);
}

TEST(GravitySim, ConserveTE2Bodies) {
  std::vector<float> masses = {2e1, 1e1};
  std::vector<DirectX::XMFLOAT3> positions = {{0, 0, 0}, {10, 10, 10}};
  std::vector<DirectX::XMFLOAT3> vels = {{0, 0, 0}, {3, 3, 3}};
  gravitysim::Simulation simulation(masses, positions, vels, 1e-5);
  
  simulation.set_G(1e1); // avoid precision issues
  simulation.set_COM_frame();
  
  float KE = simulation.get_KE();
  float PE = simulation.get_PE();
  float TE = KE + PE;
  printf("Initial KE: %.7f, Initial PE: %.7f, TE: %.7f\n", KE, PE, TE);
  for (int i=0; i<10; i++) {
    for (int j=0; j<1; j++) {
      simulation.step();
    }
    KE = simulation.get_KE();
    PE = simulation.get_PE();
    // probably too large
    EXPECT_NEAR(TE, KE + PE, 1e-1);
    //printf("KE: %.7f, PE: %.7f, TE: %.7f\n", KE, PE, KE + PE);
    //for (auto &p : simulation.get_positions()) {
    //  printf("%g %g %g\n", p.x, p.y, p.z);
    //}
  }
}

TEST(GravitySim, CPU_and_GPU_same) {
  std::vector<float> masses = {2e1, 1e1};
  std::vector<DirectX::XMFLOAT3> positions = {{0, 0, 0}, {10, 10, 10}};
  std::vector<DirectX::XMFLOAT3> vels = {{0, 0, 0}, {3, 3, 3}};
  gravitysim::Simulation sim_cpu(masses, positions, vels, 1e-5);
  
  gravitysim::Simulation sim_gpu(masses, positions, vels, 1e-5);
  
  
  sim_cpu.set_G(1e1); // avoid precision issues
  sim_cpu.set_COM_frame();
  sim_gpu.set_G(1e1); // avoid precision issues
  sim_gpu.set_COM_frame();
  
  for (int i=0; i<1000; i++) {
    for (int j=0; j<10; j++) {
      sim_cpu.step();
      sim_gpu.step();
    }
    auto cpuKE = sim_cpu.get_KE();
    auto cpuPE = sim_cpu.get_PE();
    auto gpuKE = sim_gpu.get_KE();
    auto gpuPE = sim_gpu.get_PE();
    //printf("CPU: KE=%g, PE=%g; GPU: KE=%g, PE=%g\n", cpuKE, cpuPE, gpuKE, gpuPE);
    EXPECT_EQ(cpuKE, gpuKE);
    EXPECT_EQ(cpuPE, gpuPE);

    auto cpu_pos = sim_cpu.get_positions();
    auto cpu_vels = sim_cpu.get_vels();
    auto gpu_pos = sim_gpu.get_positions();
    auto gpu_vels = sim_gpu.get_vels();
    for (int i=0; i<cpu_pos.size(); i++) {
      EXPECT_EQ(cpu_pos[i].x, gpu_pos[i].x);
      EXPECT_EQ(cpu_pos[i].y, gpu_pos[i].y);
      EXPECT_EQ(cpu_pos[i].z, gpu_pos[i].z);
      EXPECT_EQ(cpu_vels[i].x, gpu_vels[i].x);
      EXPECT_EQ(cpu_vels[i].y, gpu_vels[i].y);
      EXPECT_EQ(cpu_vels[i].z, gpu_vels[i].z);
    }
  }
}
