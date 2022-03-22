#include <string>
#include <memory>

#include "helper.h"
#include "encode.h"
#include "bitstreambuffer.h"
#include "session.h"
#include "api.h"
#include "logging_intern.h"

namespace encode {

BitStreamBuffer::BitStreamBuffer(Session &session) : session(session) {
    NV_ENC_CREATE_BITSTREAM_BUFFER buffer;
    zero(buffer);
    buffer.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;

    ERR(API(nvEncCreateBitstreamBuffer(session.getEncoder(), &buffer)), "Error creating bitstream buffer");

    bitstreamBuffer = buffer.bitstreamBuffer;
}

BitStreamBuffer::~BitStreamBuffer() {
    if (bitstreamBuffer) {
        if (API(nvEncDestroyBitstreamBuffer(session.getEncoder(), bitstreamBuffer)) != NV_ENC_SUCCESS) {
            _LOG_ERROR("Error destroying bit stream buffer");
        }
    }
}

BitStreamBuffer::Lock::Lock(Session &session, NV_ENC_OUTPUT_PTR bitstreamBuffer) : session(session), bitstreamBuffer(bitstreamBuffer) {
    NV_ENC_LOCK_BITSTREAM lock;
    zero(lock);
    lock.version = NV_ENC_LOCK_BITSTREAM_VER;
    lock.doNotWait = 1;
    lock.outputBitstream = bitstreamBuffer;
    ERR(API(nvEncLockBitstream(session.getEncoder(), &lock)), "Error locking bit stream");
    ptr = lock.bitstreamBufferPtr;
    size = lock.bitstreamSizeInBytes;
}

BitStreamBuffer::Lock::~Lock() {
    if (API(nvEncUnlockBitstream(session.getEncoder(), bitstreamBuffer)) != NV_ENC_SUCCESS) {
        _LOG_ERROR("Error unlocking bit stream");
    }
}

}