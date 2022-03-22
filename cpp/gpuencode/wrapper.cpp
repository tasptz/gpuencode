#include <fstream>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "context.h"
#include "encode.h"

namespace py = pybind11;

namespace wrapper {

class CudaContext {
public:
    CudaContext(int device) : device(device) { }

    void enter() {
        cudaContext = std::make_unique<encode::CudaContext>(device);
    }

    void exit(py::args ) {
        cudaContext.reset();
    }

private:
    const int device;
    std::unique_ptr<encode::CudaContext> cudaContext;
};

class Encoder {
public:
    class Impl {
    public:
        Impl(encode::Config config, std::ostream &ostream) :
            encoder(config, ostream)
        { }

        void operator() (uint64_t frameIndex, py::array_t<uint8_t> array) {
            const uint8_t *ptr = array.data();
            // release the gil
            // the numpy array must remain untouched on the python side
            py::gil_scoped_release release;
            encoder(frameIndex, ptr);
        }
    private:
        encode::Encoder encoder;
    };

    Encoder(encode::Config config) : config(config)
    { }

    Impl &enter() {
        if (videoFilepath.empty())
            throw std::runtime_error("No video filepath set");
        ofs.open(videoFilepath, std::ofstream::out + std::ofstream::binary);
        encoder = std::make_unique<Impl>(config, ofs);
        return *encoder;
    }

    void exit(py::args ) {
        encoder.reset();
        ofs.close();
    }

    std::string videoFilepath;
private:
    encode::Config config;
    std::ofstream ofs;
    std::unique_ptr<Impl> encoder;
};

}

PYBIND11_MODULE(pygpuencode, m) {
    m.doc() = "GPU video encoding through Nvidia Encode";
    py::class_<wrapper::CudaContext>(m, "CudaContext")
        .def(py::init<int>(), py::arg("device")=0)
        .def("__enter__", &wrapper::CudaContext::enter)
        .def("__exit__", &wrapper::CudaContext::exit);
    py::enum_<encode::Codec>(m, "Codec")
        .value("H264", encode::Codec::H264)
        .value("HEVC", encode::Codec::HEVC);
    py::enum_<encode::Preset>(m, "Preset")
        .value("P1", encode::Preset::P1)
        .value("P2", encode::Preset::P2)
        .value("P3", encode::Preset::P3)
        .value("P4", encode::Preset::P4)
        .value("P5", encode::Preset::P5)
        .value("P6", encode::Preset::P6)
        .value("P7", encode::Preset::P7);
    py::enum_<encode::Tuning>(m, "Tuning")
        .value("HighQuality", encode::Tuning::HighQuality)
        .value("LowLatency", encode::Tuning::LowLatency)
        .value("UltraLowLatency", encode::Tuning::UltraLowLatency)
        .value("Lossless", encode::Tuning::Lossless);
    py::enum_<encode::BufferFormat>(m, "Format")
        .value("ABGR", encode::BufferFormat::ABGR)
        .value("ABGR10", encode::BufferFormat::ABGR10)
        .value("NV12", encode::BufferFormat::NV12);
    py::class_<encode::Config>(m, "SessionConfig")
        .def(py::init())
        .def_readwrite("codec", &encode::Config::codec)
        .def_readwrite("preset", &encode::Config::preset)
        .def_readwrite("tuning", &encode::Config::tuning)
        .def_readwrite("format", &encode::Config::format)
        .def_readwrite("width", &encode::Config::width)
        .def_readwrite("height", &encode::Config::height)
        .def_readwrite("framerateNum", &encode::Config::frameRateNum)
        .def_readwrite("frameRateDen", &encode::Config::frameRateDen)
        .def_readwrite("buffers", &encode::Config::frameRateNum);
    py::class_<wrapper::Encoder::Impl>(m, "EncoderImpl")
        .def("__call__", &wrapper::Encoder::Impl::operator ());
    py::class_<wrapper::Encoder>(m, "Encoder")
        .def(py::init<encode::Config>())
        .def("__enter__", &wrapper::Encoder::enter, py::return_value_policy::reference)
        .def("__exit__", &wrapper::Encoder::exit)
        .def_readwrite("video_filepath", &wrapper::Encoder::videoFilepath);
}