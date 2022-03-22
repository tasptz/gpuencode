#include <stdexcept>
#include <vector>

#include "helper.h"
#include "context.h"
#include "logging_intern.h"

namespace encode {

struct CudaContext::CudaContextImpl {
    CUcontext ctx = nullptr;
};

CudaContext::CudaContext(int device) : device(device) {
    impl = new CudaContext::CudaContextImpl();

    CS(cuInit(0), "Cuda init");

    int deviceCount;
    CS(cuDeviceGetCount(&deviceCount), "Cuda device count");

    if (deviceCount < (device + 1))
        throw std::runtime_error("Too few devices: " + std::to_string(deviceCount));

    CUdevice cuDevice;
    CS(cuDeviceGet(&cuDevice, device), "Cuda get device");

    int major, minor;
    CS(cuDeviceGetAttribute(&major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, cuDevice), "Cuda get version major");
    CS(cuDeviceGetAttribute(&minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, cuDevice), "Cuda get version minor");

    if (((major << 4) + minor) < 0x30) {
        throw std::runtime_error("Incompatible device");
    }

    std::vector<char> name(1024);
    CS(cuDeviceGetName(name.data(), static_cast<int>(name.size()), cuDevice), "Cuda get device name");
    _LOG_INFO("Device " << name.data() << " (compute capability " << major << "." << minor << ")");

    CS(cuCtxCreate(&(impl->ctx), 0, cuDevice), "Cuda context");
}

CudaContext::~CudaContext() {
    auto ctx = impl->ctx;
    delete impl;
    if (ctx) {
        if (cuCtxDestroy(ctx) != CUDA_SUCCESS) {
            _LOG_ERROR("Error destroying cuda context");
        }
    }
}

}