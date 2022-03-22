#pragma once

#include <functional>
#include <memory>

#include "encode.h"

namespace encode {

NV_ENC_BUFFER_FORMAT bufferFormatTo(BufferFormat buffer);

class InputBuffer;
class BitStreamBuffer;
class Event;

typedef std::function<void (void *, NV_ENC_INITIALIZE_PARAMS &)> UpdateConfig;

class Session {
public:
    Session(Config config, UpdateConfig updateConfig=[](void *, NV_ENC_INITIALIZE_PARAMS &) { });
    ~Session();
    NON_COPYABLE_NOR_MOVABLE(Session);

    Config getConfig() { return config; }
    void* getEncoder() { return encoder.get(); }

    void encode(uint64_t frameIndex, InputBuffer &inputBuffer, BitStreamBuffer &bitStreamBuffer, Event &event, uint32_t pitch);
    /// Flush the encoder after last frame
    void flush(BitStreamBuffer &bitStreamBuffer, Event &event);
private:
    Config config;
    std::unique_ptr<void, void (*)(void *)> encoder;
};

}