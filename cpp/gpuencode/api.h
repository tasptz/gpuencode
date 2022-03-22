#pragma once

#include <memory>
#include <mutex>

// temporary object exists until complete expression where it was created is done
#define API(z) encode::LockedApi()->z

namespace encode {

class Api {
public:
    Api();
    ~Api();
    NON_COPYABLE_NOR_MOVABLE(Api);
    NV_ENCODE_API_FUNCTION_LIST* operator->() { return &api; }
private:
    std::unique_ptr<void, void (*)(void *)> library;
    NV_ENCODE_API_FUNCTION_LIST api;
};

class LockedApi {
    public:
        LockedApi();
        ~LockedApi();
        NON_COPYABLE_NOR_MOVABLE(LockedApi);
        NV_ENCODE_API_FUNCTION_LIST* operator->();
    private:
        static std::mutex apiMutex;
        std::unique_lock<std::mutex> apiLock;
};

}
