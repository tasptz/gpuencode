#include <memory>

#include "encode.h"
#include "helper.h"
#include "inputbuffer.h"
#include "session.h"
#include "api.h"
#include "logging_intern.h"

namespace encode {

InputBuffer::InputBuffer(Session &session) : session(session) {
    auto config = session.getConfig();

    NV_ENC_CREATE_INPUT_BUFFER buffer;
    zero(buffer);
    buffer.version = NV_ENC_CREATE_INPUT_BUFFER_VER;
    buffer.width = config.width;
    buffer.height = config.height;
    buffer.bufferFmt = bufferFormatTo(config.format);
    ERR(API(nvEncCreateInputBuffer(session.getEncoder(), &buffer)), "Error creating input buffer");
    inputBuffer = buffer.inputBuffer;
}

InputBuffer::~InputBuffer() {
    if (inputBuffer) {
        if (API(nvEncDestroyInputBuffer(session.getEncoder(), inputBuffer)) != NV_ENC_SUCCESS) {
            _LOG_ERROR("Error destroying input buffer");
        }
    }
}

InputBuffer::Lock::Lock(Session &session, NV_ENC_INPUT_PTR inputBuffer) : session(session), inputBuffer(inputBuffer) {
    NV_ENC_LOCK_INPUT_BUFFER lock;
    zero(lock);
    lock.version = NV_ENC_LOCK_INPUT_BUFFER_VER;
    lock.inputBuffer = inputBuffer;
    ERR(API(nvEncLockInputBuffer(session.getEncoder(), &lock)), "Error locking input buffer");
    ptr = lock.bufferDataPtr;
    pitch = lock.pitch;
}

InputBuffer::Lock::~Lock() {
    if (API(nvEncUnlockInputBuffer(session.getEncoder(), inputBuffer)) != NV_ENC_SUCCESS) {
        _LOG_ERROR("Error unlocking input buffer");
    }
}

}