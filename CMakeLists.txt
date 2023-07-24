cmake_minimum_required(VERSION 3.8)
project(GravitySimCuda LANGUAGES CXX CUDA)

enable_language(CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cu" "src/*.cpp" main.cu)

add_executable(gravity_sim_cuda ${SOURCES})

target_compile_features(gravity_sim_cuda PUBLIC cxx_std_20)
set_target_properties(gravity_sim_cuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_subdirectory(imgui)
add_subdirectory(DirectXMath)
add_subdirectory(${CMAKE_SOURCE_DIR}/DirectXTK ${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK)
target_link_libraries(gravity_sim_cuda PRIVATE imgui d3d12.lib DirectXTK)


# cmake -DCMAKE_CUDA_ARCHITECTURES=75 -B build
# cmake --build build