#pragma once

#include <memory>
#include <nvEncodeAPI.h>

#include "nocopymove.h"

namespace encode {

class Session;

class InputBuffer {
public:
    class Lock {
    public:
        Lock(Session &session, NV_ENC_INPUT_PTR inputBuffer);
        ~Lock();
        NON_COPYABLE_NOR_MOVABLE(Lock);

        void *getPointer() { return ptr; }
        uint32_t getPitch() { return pitch; }
    private:
        Session &session;
        NV_ENC_INPUT_PTR inputBuffer = nullptr;
        void *ptr = nullptr;
        uint32_t pitch = 0;
    };
    InputBuffer(Session &session);
    ~InputBuffer();
    NON_COPYABLE_NOR_MOVABLE(InputBuffer);

    NV_ENC_INPUT_PTR getInputBuffer() { return inputBuffer; }
    std::unique_ptr<Lock> lock() { return std::make_unique<Lock>(session, inputBuffer); }
private:
    Session &session;
    NV_ENC_INPUT_PTR inputBuffer = nullptr;
};

}