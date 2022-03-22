cmake_minimum_required(VERSION 3.10)
project(GpuEncode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(CUDAToolkit REQUIRED)
set(cuda_libs CUDA::cudart CUDA::cuda_driver)

add_subdirectory(ext)

option(GPUENCODE_STATIC "Build static" 1)
option(WITH_EXAMPLES "Build examples" 0)
option(BUILD_PYTHON "Build with python wrapper" 0)

add_subdirectory(cpp)

if(NOT BUILD_PYTHON)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_NAME "gpuencode")
    set(CPACK_PACKAGE_VERSION_MAJOR 1)
    set(CPACK_PACKAGE_VERSION_MINOR 0)
    set(CPACK_PACKAGE_VERSION_PATCH 0)
    include(CPack)
endif()
