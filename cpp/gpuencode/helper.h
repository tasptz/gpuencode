#pragma once

#include <nvEncodeAPI.h>
#include <cuda.h>
#include <sstream>

#define THROW(x, m) do { \
    std::ostringstream oss; \
    oss << __FILE__ << ":" << __LINE__ << " " << m << " (" << static_cast<int>(x) << ")"; \
    throw std::runtime_error(oss.str()); \
} while (false)

#define CS(x, m) do { if (x != CUDA_SUCCESS) THROW(x, m); } while (false)
#define ERR(x, m) do { if (x != NV_ENC_SUCCESS) THROW(x, m); } while (false)

template <typename T>
void zero(T& t) {
    memset(&t, 0, sizeof(t));
}
