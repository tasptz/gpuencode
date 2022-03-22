#pragma once

#include <stdint.h>

#include "nocopymove.h"

namespace encode {

enum class Codec {
    H264,
    HEVC
};

enum class Preset {
  P1,
  P2,
  P3,
  P4,
  P5,
  P6,
  P7
};

enum class Tuning {
    HighQuality,
    LowLatency,
    UltraLowLatency,
    Lossless
};

enum class BufferFormat {
    ABGR,
    ABGR10,
    NV12 // full Y plane, interleaved Cr, Cy half planes
};

struct Config {
    Codec codec = Codec::HEVC;
    Preset preset = Preset::P5;
    Tuning tuning = Tuning::HighQuality;
    BufferFormat format = BufferFormat::ABGR;
    uint32_t width;
    uint32_t height;
    uint32_t frameRateNum;
    uint32_t frameRateDen = 1;
    uint32_t buffers = 15;
};

class Encoder {
public:
    Encoder(Config config, std::ostream &ostream);
    ~Encoder();
    NON_COPYABLE_NOR_MOVABLE(Encoder);

    /**
     * Data needs to be in format rgba (4 bytes per pixel)
     * For internal use: if ptr is nullptr, the encoder will be flushed.
     */
    void operator() (uint64_t frameIndex, const uint8_t *ptr);
private:
    struct EncoderImpl;
    EncoderImpl *impl = nullptr;
};

}