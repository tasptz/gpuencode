#pragma once

#include "nocopymove.h"

namespace encode {

class CudaContext {
public:
    CudaContext(int device=0);
    ~CudaContext();
    NON_COPYABLE_NOR_MOVABLE(CudaContext);

    int getDevice() { return device; }
private:
    int device = -1;
    struct CudaContextImpl;
    CudaContextImpl *impl = nullptr;
};

}