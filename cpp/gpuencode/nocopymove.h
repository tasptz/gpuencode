#pragma once

#define NON_COPYABLE_NOR_MOVABLE(T) \
    T(const T &) = delete; \
    T(T &&) = delete; \
    T& operator=(const T &) = delete; \
    T& operator=(T &&) = delete;
