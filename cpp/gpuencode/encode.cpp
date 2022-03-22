#include <mutex>
#include <thread>
#include <memory>
#include <queue>
#include <atomic>

#include "encode.h"
#include "helper.h"
#include "bitstreambuffer.h"
#include "inputbuffer.h"
#include "event.h"
#include "session.h"
#include "logging_intern.h"

namespace encode {

struct Encoder::EncoderImpl {
    struct Tuple {
        std::unique_ptr<BitStreamBuffer> bitStreamBuffer;
        std::unique_ptr<InputBuffer> inputBuffer;
        std::unique_ptr<Event> event;
        bool stop = false;
    };

    EncoderImpl(Config config, std::ostream &ostream) :
        bufferFormat(config.format),
        session(config),
        ostream(ostream),
        outputThread(&EncoderImpl::outputLoop, this)
        {

        std::unique_lock<std::mutex> lockInput(inputMutex);
        for (uint32_t i = 0; i < config.buffers; ++i) {
            Tuple t;
            t.bitStreamBuffer = std::make_unique<BitStreamBuffer>(session);
            t.inputBuffer = std::make_unique<InputBuffer>(session);
            t.event = std::make_unique<Event>(session);
            input.push(std::move(t));
        }
    }

    ~EncoderImpl() {
        if (mustFlush) {
            this->operator ()(0, nullptr);
        }

        {
            std::unique_lock<std::mutex> lockInput(inputMutex);
            std::unique_lock<std::mutex> lockOutput(outputMutex);
            output.push(std::move(input.front()));
            input.pop();
            // mark it for stopping
            output.back().stop = true;
            outputCond.notify_one();
        }
        outputThread.join();
    }

    void EncoderImpl::operator() (uint64_t frameIndex, const uint8_t *ptr) {
        Tuple t;
        {
            std::unique_lock<std::mutex> l(inputMutex);
            inputCond.wait(l, [this]() { return this->input.empty() == false; });
            t = std::move(input.front());
            input.pop();
        }

        if (ptr != nullptr) {
            uint32_t pitch = 0;
            {
                auto lock = t.inputBuffer->lock();
                pitch = lock->getPitch();
                if (bufferFormat == BufferFormat::ABGR || bufferFormat == BufferFormat::ABGR10) {
                    if (pitch == session.getConfig().width * 4) {
                        std::copy(
                            ptr,
                            ptr + session.getConfig().width * session.getConfig().height * 4,
                            reinterpret_cast<uint8_t*>(lock->getPointer()));
                    }
                    else {
                        auto *dst = reinterpret_cast<uint8_t*>(lock->getPointer());
                        for (unsigned int r = 0; r < session.getConfig().height; ++r) {
                            std::copy(
                                ptr,
                                ptr + session.getConfig().width * 4,
                                dst);
                            ptr += session.getConfig().width * 4;
                            dst += pitch;
                        }
                    }
                }
                else if (bufferFormat == BufferFormat::NV12) {
                    if (pitch == session.getConfig().width) {
                        std::copy(
                            ptr,
                            ptr + (session.getConfig().width * session.getConfig().height * 3) / 2,
                            reinterpret_cast<uint8_t*>(lock->getPointer())
                        );
                    }
                    else {
                        auto *dst = reinterpret_cast<uint8_t*>(lock->getPointer());
                        for (unsigned int r = 0; r < (session.getConfig().height * 3) / 2; ++r) {
                            std::copy(
                                ptr,
                                ptr + session.getConfig().width,
                                dst);
                            ptr += session.getConfig().width;
                            dst += pitch;
                        }
                    }
                }
            }
            session.encode(frameIndex, *t.inputBuffer, *t.bitStreamBuffer, *t.event, pitch);
        }
        else {
            // lock it just to be sure it is not in use
            auto lock = t.inputBuffer->lock();
            session.flush(*t.bitStreamBuffer, *t.event);
        }

        {
            std::unique_lock<std::mutex> l(outputMutex);
            output.push(std::move(t));
            outputCond.notify_one();
        }
    }

    void EncoderImpl::outputLoop() {
        try {
            const std::thread::id threadId = std::this_thread::get_id();
            while (true) {
                Tuple t;
                {
                    std::unique_lock<std::mutex> l(outputMutex);
                    outputCond.wait(l, [this]() { return this->output.empty() == false; });
                    if (output.front().stop) {
                        break;
                    }
                    t = std::move(output.front());
                    output.pop();
                }

                bool gotFrame = false;
                for (int i = 0; i < 10; ++i) {
                    try {
                        t.event->wait();
                        auto lock = t.bitStreamBuffer->lock();
                        ostream.write(
                            reinterpret_cast<const char*>(lock->getPointer()),
                            lock->getSize()
                        );
                        mustFlush = true;
                        gotFrame = true;
                        break;
                    }
                    catch (Event::Timeout &) {
                        _LOG_ERROR(threadId << " Timeout occurred (" << i << ")");
                    }
                }

                if (gotFrame == false) {
                    std::ostringstream oss;
                    oss << threadId << " Error in retrieving encoded frames";
                    throw std::runtime_error(oss.str());
                }

                {
                    std::unique_lock<std::mutex> l(inputMutex);
                    input.push(std::move(t));
                    inputCond.notify_one();
                }
            }
        }
        catch (std::exception &e) {
            _LOG_ERROR("Error in output thread: " << e.what());
        }
        _LOG_INFO("Output loop done");
    }

    const BufferFormat bufferFormat;

    std::atomic_bool mustFlush = false;
    Session session;
    std::ostream &ostream;

    std::queue<Tuple> input;
    std::condition_variable inputCond;
    std::mutex inputMutex;

    std::queue<Tuple> output;
    std::condition_variable outputCond;
    std::mutex outputMutex;

    std::thread outputThread;
};

Encoder::Encoder(Config config, std::ostream &ostream) {
    impl = new EncoderImpl(config, ostream);
}

Encoder::~Encoder() {
    delete impl;
}

void Encoder::operator() (uint64_t frameIndex, const uint8_t *ptr) {
    impl->operator ()(frameIndex, ptr);
}

}