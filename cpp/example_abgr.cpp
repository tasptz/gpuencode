#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <gpuencode/context.h>
#include <gpuencode/encode.h>

int main() {
    try {
        encode::CudaContext cudaContext;
        encode::Config config;
        config.codec = encode::Codec::HEVC;
        config.preset = encode::Preset::P1;
        config.tuning = encode::Tuning::HighQuality;
        config.format = encode::BufferFormat::ABGR;
        config.width = 640;
        config.height = 480;
        config.frameRateNum = 20;

        std::ofstream ofs("video.hevc", std::ofstream::out + std::ofstream::binary);
        {
            encode::Encoder encoder(config, ofs);
            std::vector<uint8_t> image(config.width * config.height * 4);
            for (uint32_t i = 0; i < config.frameRateNum * 10; ++i) {
                auto p = &image[0];
                for (unsigned int y = 0; y < config.height; ++y) {
                    for (unsigned int x = 0; x < config.width; ++x) {
                        *p++ = (x + i) % 256;
                        *p++ = (y + i * 2) % 256;
                        *p++ = (x + y + i * 3) % 256;
                        *p++ = 255;
                    }
                }
                std::cout << "." << std::flush;
                encoder(i, &image[0]);
            }
            std::cout << "\n";
        }
        ofs.close();
        std::cout << "done" << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}