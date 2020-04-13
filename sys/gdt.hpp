#pragma once

#include <stdint.h>
#include <stddef.h>
#include <debugging/debugger.hpp>
#include <mm/pmm.hpp>

struct alignas(1) GDTEntry {
    uint16_t limit;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t access;
    uint8_t granularity;
    uint8_t baseHigh;
} __attribute__((packed));

struct GDTPointer {
    uint16_t size;
    void* address;
} __attribute__((packed));

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

namespace WivOS {
    class GDT
    {
    private:
        Debugger debugger;
        Pmm pmm;

        GDTEntry gdtEntries[3];
        GDTPointer gdtPointer;
    public:
        GDT(Debugger debugger, Pmm pmm);
        ~GDT();

        void init();
    };
    
};