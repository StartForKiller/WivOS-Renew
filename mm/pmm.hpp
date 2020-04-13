#pragma once

#include <stdint.h>
#include <stddef.h>
#include <boot/stivale.h>
#include <debugging/debugger.hpp>
#include <lib/lock.hpp>

#define KERNEL_PHYS_OFFSET 0xffffffff80000000
#define MEM_PHYS_OFFSET 0xffff800000000000
#define PAGE_SIZE 0x1000

#define bit_test(var, offset) ({ \
    int __ret; \
    asm volatile ( \
        "bt %2, %1;" \
        : "=@ccc" (__ret) \
        : "r" ((uint32_t)(var)), "r" ((uint32_t)(offset)) \
    ); \
    __ret; \
})

#define test_bit(base, offset) ({ \
    int ret; \
    asm volatile ( \
        "bt %2, (%1);" \
        : "=@ccc" (ret) \
        : "r" (base), "r" (offset) \
        : "memory" \
    ); \
    ret; \
})

#define set_bit(base, offset) ({ \
    int ret; \
    asm volatile ( \
        "bts %2, (%1);" \
        : "=@ccc" (ret) \
        : "r" (base), "r" (offset) \
        : "memory" \
    ); \
    ret; \
})

#define reset_bit(base, offset) ({ \
    int ret; \
    asm volatile ( \
        "btr %2, (%1);" \
        : "=@ccc" (ret) \
        : "r" (base), "r" (offset) \
        : "memory" \
    ); \
    ret; \
})

namespace WivOS {
    class Pmm
    {
    private:
        Debugger debugger;

        size_t allocBase;
        size_t topPage;
        size_t* bitmap;
        Lock pmmLock;
        size_t endIndex;
        struct stivale_memmap_t *memmap;

        void* allocInternal(size_t count, size_t limit);

    public:
        Pmm(struct stivale_memmap_t *memmap, Debugger debugger);
        ~Pmm();

        void init();

        void* pmmAlloc(size_t count);
        void* pmmAllocNoZero(size_t count);
        void pmmFree(void* ptr, size_t count);
    };
    
    static size_t AlignUp(size_t value, size_t alignment);
    static size_t AlignDown(size_t value, size_t alignment);
};