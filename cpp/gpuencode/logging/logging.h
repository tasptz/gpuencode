#pragma once

#include <iostream>

#define LOG_TRACE(x) std::cout << "Trace\t" << x << std::endl
#define LOG_DEBUG(x) std::cout << "Debug\t" << x << std::endl
#define LOG_INFO(x) std::cout << "Info\t" << x << std::endl
#define LOG_WARN(x) std::cerr << "Warning\t" << x << std::endl
#define LOG_ERROR(x) std::cerr << "Error\t" << x << std::endl
#define LOG_CRITICAL(x) std::cerr << "Critical\t" << x << std::endl
