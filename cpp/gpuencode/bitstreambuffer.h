#pragma once

#include <memory>
#include <nvEncodeAPI.h>

namespace encode {

class Session;

class BitStreamBuffer {
public:
    class Lock {
    public:
        Lock(Session &session, NV_ENC_OUTPUT_PTR bitstreamBuffer);
        ~Lock();
        NON_COPYABLE_NOR_MOVABLE(Lock);

        void *getPointer() { return ptr; }
        uint32_t getSize() { return size; }
    private:
        Session &session;
        NV_ENC_OUTPUT_PTR bitstreamBuffer = nullptr;
        void *ptr = nullptr;
        uint32_t size = 0;
    };

    BitStreamBuffer(Session &session);
    ~BitStreamBuffer();
    NON_COPYABLE_NOR_MOVABLE(BitStreamBuffer);

    NV_ENC_OUTPUT_PTR getBitStreamBuffer() { return bitstreamBuffer; }
    std::unique_ptr<Lock> lock() { return std::make_unique<Lock>(session, bitstreamBuffer); }
private:
    Session &session;
    NV_ENC_OUTPUT_PTR bitstreamBuffer = nullptr;
    void *ptr = nullptr;
    uint32_t size = 0;
};

}