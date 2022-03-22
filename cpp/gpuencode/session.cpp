#include <stdexcept>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <cuda.h>

#include "helper.h"
#include "encode.h"
#include "session.h"
#include "inputbuffer.h"
#include "bitstreambuffer.h"
#include "event.h"
#include "api.h"
#include "logging_intern.h"

namespace encode {

GUID codecTo(Codec codec) {
    std::map<Codec, GUID> codecs {
        std::make_pair(Codec::H264, NV_ENC_CODEC_H264_GUID),
        std::make_pair(Codec::HEVC, NV_ENC_CODEC_HEVC_GUID),
    };
    return codecs[codec];
}

GUID presetTo(Preset preset) {
    std::map<Preset, GUID> presets {
        std::make_pair(Preset::P1, NV_ENC_PRESET_P1_GUID),
        std::make_pair(Preset::P2, NV_ENC_PRESET_P2_GUID),
        std::make_pair(Preset::P3, NV_ENC_PRESET_P3_GUID),
        std::make_pair(Preset::P4, NV_ENC_PRESET_P4_GUID),
        std::make_pair(Preset::P5, NV_ENC_PRESET_P5_GUID),
        std::make_pair(Preset::P6, NV_ENC_PRESET_P6_GUID),
        std::make_pair(Preset::P7, NV_ENC_PRESET_P7_GUID)
    };
    return presets[preset];
}

NV_ENC_TUNING_INFO tuningTo(Tuning tuning) {
    std::map<Tuning, NV_ENC_TUNING_INFO> tunings {
        std::make_pair(Tuning::HighQuality, NV_ENC_TUNING_INFO_HIGH_QUALITY),
        std::make_pair(Tuning::LowLatency, NV_ENC_TUNING_INFO_LOW_LATENCY),
        std::make_pair(Tuning::UltraLowLatency, NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY),
        std::make_pair(Tuning::Lossless, NV_ENC_TUNING_INFO_LOSSLESS)
    };
    return tunings[tuning];
}

NV_ENC_BUFFER_FORMAT bufferFormatTo(BufferFormat buffer) {
    std::map<BufferFormat, NV_ENC_BUFFER_FORMAT> buffers {
        std::make_pair(BufferFormat::ABGR, NV_ENC_BUFFER_FORMAT_ABGR),
        std::make_pair(BufferFormat::ABGR10, NV_ENC_BUFFER_FORMAT_ABGR10),
        std::make_pair(BufferFormat::NV12, NV_ENC_BUFFER_FORMAT_NV12)
    };
    return buffers[buffer];
}

void destroyEncoder(void *encoder) {
    if (encoder != nullptr) {
        auto r = API(nvEncDestroyEncoder(encoder));
        if (r != NV_ENC_SUCCESS) {
            _LOG_ERROR("Error destroying encoder " << r);
        }
    }
}

Session::Session(Config config, UpdateConfig updateConfig) : config(config), encoder(nullptr, destroyEncoder) {
    const GUID codec = codecTo(config.codec);
    const GUID preset = presetTo(config.preset);
    const NV_ENC_BUFFER_FORMAT bufferFormat = bufferFormatTo(config.format);
    const NV_ENC_TUNING_INFO tuning = tuningTo(config.tuning);
    
    CUcontext ctx;
    CS(cuCtxGetCurrent(&ctx), "Cuda current context");
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS openSessionExParams;
    zero(openSessionExParams);
    openSessionExParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
    openSessionExParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
    openSessionExParams.device = ctx;
    openSessionExParams.apiVersion = NVENCAPI_VERSION;

    {
        void *e = nullptr;
        ERR(API(nvEncOpenEncodeSessionEx(&openSessionExParams, &e)), "Error opening encode session");
        encoder.reset(e);
    }

    uint32_t encodePresetGUIDCount;
    ERR(API(nvEncGetEncodePresetCount(encoder.get(), codec, &encodePresetGUIDCount)), "Error getting preset count");

    std::vector<GUID> presetGUIDs(encodePresetGUIDCount);
    ERR(API(nvEncGetEncodePresetGUIDs(encoder.get(), codec, presetGUIDs.data(), encodePresetGUIDCount, &encodePresetGUIDCount)), "Error getting preset guids");

    if (std::find(presetGUIDs.begin(), presetGUIDs.end(), preset) == presetGUIDs.end()) {
        throw std::runtime_error("Error preset not supported");
    }

    NV_ENC_INITIALIZE_PARAMS params;
    zero(params);
    params.version = NV_ENC_INITIALIZE_PARAMS_VER;
    params.encodeGUID = codec;
    params.presetGUID = preset;
    params.encodeWidth = config.width;
    params.encodeHeight = config.height;
    params.darWidth = config.width;
    params.darHeight = config.height;
    params.frameRateNum = config.frameRateNum;
    params.frameRateDen =  config.frameRateDen;
    params.enableEncodeAsync = 1;
    params.enablePTD = 1;
    params.reportSliceOffsets = 0;
    params.enableSubFrameWrite = 0;
    params.enableExternalMEHints = 0;
    params.enableMEOnlyMode = 0;
    params.enableWeightedPrediction = 0;
    params.enableOutputInVidmem = 0;
    params.encodeConfig = nullptr;
    params.maxEncodeWidth = params.encodeWidth;
    params.maxEncodeHeight = params.encodeHeight;
    params.tuningInfo = tuning;
    params.bufferFormat = NV_ENC_BUFFER_FORMAT_UNDEFINED;

    updateConfig(encoder.get(), params);

    ERR(API(nvEncInitializeEncoder(encoder.get(), &params)), "Error initializing encoder");
}

Session::~Session() {

}

void Session::encode(uint64_t frameIndex, InputBuffer &inputBuffer, BitStreamBuffer &bitStreamBuffer, Event &event, uint32_t pitch) {
    NV_ENC_PIC_PARAMS params;
    zero(params);
    params.version = NV_ENC_PIC_PARAMS_VER;
    params.inputWidth = config.width;
    params.inputHeight = config.height;
    params.inputPitch = pitch;
    params.frameIdx = static_cast<uint32_t>(frameIndex);
    params.inputTimeStamp = frameIndex;
    params.inputDuration = (config.frameRateDen * 1000000) / config.frameRateNum; // micro seconds!
    params.inputBuffer = inputBuffer.getInputBuffer();
    params.outputBitstream = bitStreamBuffer.getBitStreamBuffer();
    params.completionEvent = event();
    params.bufferFmt = bufferFormatTo(config.format);
    params.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;

    ERR(API(nvEncEncodePicture(encoder.get(), &params)), "Error encoding picture");
}

void Session::flush(BitStreamBuffer &bitStreamBuffer, Event &event) {
    NV_ENC_PIC_PARAMS params;
    zero(params);
    params.version = NV_ENC_PIC_PARAMS_VER;
    params.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
    params.completionEvent = event();
    ERR(API(nvEncEncodePicture(encoder.get(), &params)), "Error flushing encoder");
}

}