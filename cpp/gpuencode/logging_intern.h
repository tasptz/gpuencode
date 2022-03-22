#pragma once

#include <sstream>

#include "logging.h"

#define _LOG(x) [&]() -> std::string {\
    std::ostringstream oss;\
    oss << x;\
    return oss.str();\
}()

#define _LOG_TRACE(x) LOG_TRACE(_LOG(x))
#define _LOG_DEBUG(x) LOG_DEBUG(_LOG(x))
#define _LOG_INFO(x) LOG_INFO(_LOG(x))
#define _LOG_WARN(x) LOG_WARN(_LOG(x))
#define _LOG_ERROR(x) LOG_ERROR(_LOG(x))
#define _LOG_CRITICAL(x) LOG_CRITICAL(_LOG(x))
