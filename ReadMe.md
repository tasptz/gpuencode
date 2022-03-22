# GPU Encode Library
The purpose of this library is to provide simple access to the [NVIDIA Video Codec SDK](https://developer.nvidia.com/nvidia-video-codec-sdk). The SDK allows to encode high quality videostreams in realtime. Of special interest is the ability to encode videos with **H.265 (HEVC) 4K Lossless and 10 bits per channel**.

**Remark**: It utilizes the [asynchronous mode](https://docs.nvidia.com/video-technologies/video-codec-sdk/nvenc-video-encoder-api-prog-guide/#asynchronous-mode) only available on Windows!

## Install
 * install [NVIDIA driver](https://www.nvidia.com/download/index.aspx) and [CUDA](https://developer.nvidia.com/cuda-zone)
 * extract [NVIDIA Video Codec SDK](https://developer.nvidia.com/nvidia-video-codec-sdk) files to CUDA installation directory or make them available elsewhere
 * `pip install -r requirements.txt`

For the c++ library run
 * run `build.bat`
 * package `build/gpuencode-*.zip` is created

For the python wrapper
 * run `python setup.py bdist_wheel`
 * wheel `./dist/gpuencode-*.whl` is created
 * install with `pip install dist/gpuencode-*.whl`

## Usage
The library provides a simple C++ and python interface, each illustrated by one example.
 * for c++ `cpp/example_abgr.cpp` and `cpp/example_concurent.cpp` (see `example_abgr.exe` and `example_concurrent.exe` inside the `build` folder)
 * for python `python/example.py`

## Supported video cards
https://developer.nvidia.com/video-encode-decode-gpu-support-matrix
