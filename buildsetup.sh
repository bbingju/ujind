#!/bin/bash

cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_TOOLCHAIN_FILE=./buildroot.cmake