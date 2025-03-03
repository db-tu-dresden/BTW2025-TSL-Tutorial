#!/bin/bash
cmake -S . -B build_fp16 -DGENERATE_TSL=True -DArchId=sapphirerapids -DCMAKE_CXX_COMPILER=g++
cmake --build build_fp16 -j
intel-sde -spr -- bin/filter_agg