#include <lib/lock.hpp>

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

namespace WivOS {
    Lock::Lock()
    {
        this->locked = 0;
    }
    
    Lock::~Lock()
    {
    }

    void Lock::acquire() {
        while(true) {
            if(__sync_bool_compare_and_swap(&(this->locked), 0, 1)) {
                return;
            }

            asm volatile("nop;\n rep;");
        }
    }

    bool Lock::tryToAcquire() {
        return __sync_bool_compare_and_swap(&(this->locked), 0, 1);
    }

    void Lock::release() {
        __sync_bool_compare_and_swap(&(this->locked), 1, 0);
    }
};