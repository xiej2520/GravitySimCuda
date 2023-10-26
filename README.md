# Gravity Simulation

## Build and run

Requirements

* CUDA Toolkit 12.3
* CMake >= v3.8
* MSVC

```Shell
cmake -S . -B build -DCMAKE_CUDA_ARCHITECTURES=75
cmake --build build --config Release
```

Executable will be built in `./build/Release/gravity_sim_cuda.exe`
