#pragma once

#include <windows.h>
#include <nvEncodeAPI.h>

namespace encode {

class Session;

class Event {
public:
    struct Timeout {
    };
    Event(Session &session);
    ~Event();

    HANDLE operator() () { return event; }
    void wait();
private:
    Session &session;
    NV_ENC_EVENT_PARAMS params;
    HANDLE event = NULL;
};

}