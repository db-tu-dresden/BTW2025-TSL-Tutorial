# SIMD for Everyone - A tutorial to TSL

This repository contains supplementary material for the tutorial "SIMD for Everyone - A tutorial to TSL", presented at Workshop on Novel Data Management Ideas on Heterogeneous Hardware Architectures (NoDMC) @ 21st Conference on Database Systems for Business, Technology and Web (BTW 2025) in Bamberg, Germany. 

If you want to work "offline", just clone the repository and open it with VSCode (using the provided devcontainer):
```console
git clone --recurse-submodules https://github.com/db-tu-dresden/BTW2025-TSL-Tutorial.git
```

The setup is tested on:
- x86-64 Linux (Arch, Ubuntu)
- aarch64 Linux (Ubuntu)
- x86-64 Windows 11

If you face any hurdles setting up the environment, we highly recommend to use __Github Codespace__ (with the provided devcontainer).


To bootstrap the tutorial, the repository contains a [devcontainer](./.devcontainer/devcontainer.json) and an associated [dockerfile](./.devcontainer/Dockerfile). 
The image contains:

- Development tools (for building the examples)
  - clang, gcc (generate x86 binaries)
  - aarch64-linux-gnu-gcc, aarch64-linux-gnu-binutils (generate aarch64 binaries)
  - cmake, make, ninja
  - python (execute the TSL generator)
- Execution/Emulation environment
  - qemu (run/emulate aarch64 code on x86)
  - intel-sde (emulate latest x86 hardware on older x86 platforms)
- TSL
  - The TSL will be installed after container start up under /usr/include/tsl. Consequently, it can be directly included and used.
  - Additionally, the TSLgenerator is located under 3rdparty/tslgen.

For the course of the tutorial, we will provide an introduction in how to use and extend the **T**emplate **S**IMD **L**ibrary ([available on *Github*](https://github.com/db-tu-dresden/TSL)) for exploiting hardware provided SIMD capabilities in a hardware-agnostic way. 


## Filter-Aggregation: TSL in action

To provide an overview about how to use and extend the TSL, we use a filter aggregation kernel as a toy example.
Basically, the filter-aggregation iterates over two chunks of consecutive memory, loads data from one location, compares it with a specific value and accumulates the corresponding values from the other piece of memory. 
In order to exploit data-level parallelism, we will implement the algorithm using explicit SIMD programming with the help of the TSL (An overview of the supported instructions can be found [**here**](https://db-tu-dresden.github.io/TSL/)). 
The associated files are:

- [src/filter_aggregate.cpp](src/filter_aggregate.cpp) (This is the main file that initializes the data and calls the algorithm)
- [src/filter_aggregate_live.hpp](src/filter_aggregate_live.hpp) (This file contains the skeleton for the live-coding session)
- [src/filter_aggregate.hpp](src/filter_aggregate.hpp) (This file can be used as reference for the relevant implementations)
- [src/column.hpp](src/column.hpp) (This file provides a column datastructure to manage consecutive data)
- [src/simd_utils.hpp](src/simd_utils.hpp), src/utils.hpp (Contains some helper functions, especially to get access to the data level parallism degree provided by the underlying hardware)

The code can be build using either the VS-Code CMake integration or using a terminal:
```console
cmake -S . -B build && cmake --build build -j4
```
The resulting binaries will be located under ./bin/

## Adding std::float16_t: Extending TSL

In order to extend the TSL, we will add support for half-precision floating point numbers into the Filter-Aggregation kernel. 
As a showcase, we decided to extend the filtering part. 
In order to extend the TSL to support std::float16_t, we need to change the following files:
- [3rdparty/tslgen/generator/config/default_conf.yaml](3rdparty/tslgen/generator/config/default_conf.yaml) (add "std::float16_t" the the relevant data types)
- [3rdparty/tslgen/compare.yaml](3rdparty/tslgen/compare.yaml)
  -  add avx512 and scalar definitions for ```greater_than_or_equal```
- [3rdparty/tslgen/ls.yaml](3rdparty/tslgen/ls.yaml)
  - add avx512 and scalar definitions for ```loadu``` and ```set1```
- [3rdparty/tslgen/mask_ls.yaml](3rdparty/tslgen/mask_ls.yaml)
  - add avx512 and scalar definitions for ```to_integral```

An experimental implementation can be found [here](https://github.com/db-tu-dresden/TSL/tree/float16).

To generate the TSL and prepare the the build environment, just run
```console
cmake -S . -B build_fp16 -DGENERATE_TSL=True -DArchId=sapphirerapids -DCMAKE_CXX_COMPILER=g++
```
As clang-19 seems to lack support for std::float16_t, we use g++-14 here. 

Next, build the code using
```console
cmake --build build_fp16 -j4
```

As we assume the participants lack direct access to Intel Sapphire Rapids cores, we use _intel_sde_ to emulate the necessary hardware with support for avx512-fp16:
```console
intel-sde -spr -- bin/filter_agg
```

