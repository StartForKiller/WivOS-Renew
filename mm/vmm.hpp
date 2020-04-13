#pragma once

#include <stdint.h>
#include <stddef.h>
#include <boot/stivale.h>
#include <debugging/debugger.hpp>
#include <lib/lock.hpp>
#include <mm/pmm.hpp>

#define PT_PRESENT (1 << 0)
#define PT_ENTRIES 512

void *kmalloc(size_t size);
void *kcalloc(size_t bytes, size_t elem);
void *krealloc(void *old, size_t s);
void kfree(void *ptr);

namespace WivOS {
    class Vmm
    {
    private:
        Debugger debugger;
        Lock lock;
        Pmm pmm;
        size_t* pml4;

        size_t* finOrAllocPageTable(size_t* table, size_t index, size_t flags);
        size_t* findPageTable(size_t* table, size_t index);
        void cleanPageTable(size_t* table);

    public:
        Vmm(struct stivale_memmap_t *memmap, Debugger debugger, Pmm pmm);
        ~Vmm();

        void setActive();
        void mapPage(size_t pAddress, size_t vAddress, size_t flags);
        void mapPageHuge(size_t pAddress, size_t vAddress, size_t flags);
        void unmapPage(size_t vAddress);
        size_t getPhys(size_t vAddress);
    };
};