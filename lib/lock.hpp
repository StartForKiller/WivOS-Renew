#pragma once

#include <stdint.h>
#include <stddef.h>

namespace WivOS {
    class Lock
    {
    private:
        uint8_t locked;
    public:
        Lock();
        ~Lock();

        void acquire();
        bool tryToAcquire();

        void release();
    };
    
};