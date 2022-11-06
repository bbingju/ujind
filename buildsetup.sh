#!/bin/bash

cmake -H. -Bbuild_sdk -DCMAKE_BUILD_TYPE=build_sdk -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_TOOLCHAIN_FILE=./buildroot.cmake
