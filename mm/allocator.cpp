#include <stdint.h>
#include <stddef.h>
#include <debugging/debugger.hpp>
#include <lib/lock.hpp>
#include <mm/vmm.hpp>

extern "C" {
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
};

extern WivOS::Vmm *kernelPml4;
extern WivOS::Pmm *kernelPmm;

#define vm_heap_base 0x200000000

uint8_t alloc_lock;

static size_t alloc_top = vm_heap_base;

void acquire_alloc() {
    while(true) {
        if(__sync_bool_compare_and_swap(&(alloc_lock), 0, 1)) {
            return;
        }

        asm volatile("nop;\n rep;");
    }
}

void release_alloc() {
    __sync_bool_compare_and_swap(&(alloc_lock), 1, 0);
}

extern WivOS::Debugger *kernelDebugger;

void *kmalloc(size_t size) {
    acquire_alloc();

    size = ((size + 7) / 8) * 8;

    size += 16;
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    void *out = (void *)alloc_top;

    for(size_t i = 0; i < pages; i++) {
        void *p = kernelPmm->pmmAlloc(1);

        if(!p) {
            release_alloc();
            return nullptr;
        }
        kernelPml4->mapPage((size_t)p, alloc_top, 0x03);
        alloc_top += PAGE_SIZE;
    }

    alloc_top += PAGE_SIZE;

    out = (void *)((uintptr_t)out + (pages * PAGE_SIZE - size));

    ((uint64_t *)out)[0] = size - 16;
    ((uint64_t *)out)[1] = pages;

    release_alloc();

    return (void *)((uintptr_t)out + 16);
}

void *kcalloc(size_t bytes, size_t elem) {
	void *out = kmalloc(bytes * elem);
	memset(out, 0, bytes * elem);
	return out;
}

void *krealloc(void *old, size_t s) {
	void *newp = kmalloc(s);
	if (old) {
		acquire_alloc();
		uint64_t size = *(uint64_t *)((uintptr_t)old - 16);
		release_alloc();
		memcpy(newp, old, size);
		kfree(old);
	}
	return newp;
}

void kfree(void *ptr) {
	acquire_alloc();
	size_t size = *(uint64_t *)((uintptr_t)ptr - 16);
	size_t req_pages = *(uint64_t *)((uintptr_t)ptr - 8);
	void *start = (void *)((uintptr_t)ptr & (~(PAGE_SIZE - 1)));

	size += 16;
	size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	for (size_t i = 0; i < pages; i++) {
		void *curr = (void *)((uintptr_t)start + i * PAGE_SIZE);
		void *p = (void *)kernelPml4->getPhys((size_t)curr);
		kernelPml4->unmapPage((size_t)curr);
		kernelPmm->pmmFree(p, 1);
	}
    release_alloc();
}

void* operator new(size_t size){
	return kmalloc(size);
}

void* operator new[](size_t size){
	return kmalloc(size);
}

void operator delete(void *p){
	kfree(p);
}

void operator delete[](void *p){
	kfree(p);
}

void operator delete(void *p, long unsigned int size){
	(void)(size);
	kfree(p);
}

void operator delete[](void *p, long unsigned int size){
	(void)(size);
	kfree(p);
}