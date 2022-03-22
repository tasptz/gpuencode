#include <stdexcept>
#include <string>

#include "helper.h"
#include "encode.h"
#include "api.h"
#include "logging_intern.h"

typedef NVENCSTATUS (NVENCAPI *MYVERSION)(uint32_t*);
typedef NVENCSTATUS (NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

namespace encode {

void freeLibray(void *l) {
    if (l != nullptr) {
        FreeLibrary(reinterpret_cast<HMODULE>(l));
    }
}

Api::Api() : library(nullptr, freeLibray) {
    library.reset(LoadLibrary("nvEncodeAPI64.dll"));
    if (library == nullptr) {
        throw std::runtime_error("Error loading library");
    }

    MYVERSION nvEncodeAPIGetMaxSupportedVersion = reinterpret_cast<MYVERSION>(GetProcAddress(reinterpret_cast<HMODULE>(library.get()), "NvEncodeAPIGetMaxSupportedVersion"));
    if (nvEncodeAPIGetMaxSupportedVersion == nullptr) {
        throw std::runtime_error("Proc address error");
    }
    uint32_t version = 0;
    ERR(nvEncodeAPIGetMaxSupportedVersion(&version), "Error getting version ");

    uint32_t majorVersion = version >> 4;
    uint32_t minorVersion = (version << 28) >> 28;
    if (majorVersion != NVENCAPI_MAJOR_VERSION || (majorVersion == NVENCAPI_MAJOR_VERSION && minorVersion < NVENCAPI_MINOR_VERSION)) {
        std::ostringstream oss;
        oss << "Wrong version " << majorVersion << "." << minorVersion << " <> " << NVENCAPI_MAJOR_VERSION << "." << NVENCAPI_MINOR_VERSION;
        throw std::runtime_error(oss.str());
    }
    else {
        _LOG_INFO("Nvidia encode version " << majorVersion << "." << minorVersion);
    }
    MYPROC nvEncodeAPICreateInstance = reinterpret_cast<MYPROC>(GetProcAddress(reinterpret_cast<HMODULE>(library.get()), "NvEncodeAPICreateInstance"));
    if (nvEncodeAPICreateInstance == nullptr) {
        throw std::runtime_error("Proc address error");
    }

    zero(api);
    api.version = NV_ENCODE_API_FUNCTION_LIST_VER;
    ERR(nvEncodeAPICreateInstance(&api), "Error creating encode api instance ");
}

Api::~Api() {

}

LockedApi::LockedApi() : apiLock(apiMutex) {

}

LockedApi::~LockedApi() {

}

NV_ENCODE_API_FUNCTION_LIST* LockedApi::operator-> () {
    static Api api;
    return api.operator->();
}

std::mutex LockedApi::apiMutex;

}