#include <string>

#include "encode.h"
#include "helper.h"
#include "session.h"
#include "api.h"
#include "event.h"
#include "logging_intern.h"

namespace encode {

Event::Event(Session &session) : session(session) {
    zero(params);
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (event == NULL) {
        throw std::runtime_error("Error creating event " + std::to_string(GetLastError()));
    }
    params.version = NV_ENC_EVENT_PARAMS_VER;
    params.completionEvent = event;
    if (API(nvEncRegisterAsyncEvent(session.getEncoder(), &params)) != NV_ENC_SUCCESS) {
        if (CloseHandle(event) == 0) {
            _LOG_ERROR("Error closing event handle");
        }
        throw std::runtime_error("Error registering async event");
    }
}

Event::~Event() {
    if (API(nvEncUnregisterAsyncEvent(session.getEncoder(), &params)) != NV_ENC_SUCCESS) {
        _LOG_ERROR("Error unregistering async event");
    }
    if (event) {
        if (CloseHandle(event) == 0) {
            _LOG_ERROR("Error closing event handle");
        }
    }
}

void Event::wait() {
    switch (WaitForSingleObject(event, 1000)) {
        case WAIT_OBJECT_0:
            return;
        case WAIT_TIMEOUT:
            throw Event::Timeout();
        default:
            throw std::runtime_error("Error while waiting");
    }
}

}