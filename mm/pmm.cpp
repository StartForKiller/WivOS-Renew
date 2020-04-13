#include <mm/pmm.hpp>

extern "C" void* kernelTop;

namespace WivOS {
    Pmm::Pmm(struct stivale_memmap_t *memmap, Debugger debugger)
    {
        this->debugger = debugger;
        this->memmap = memmap;
        this->endIndex = 0;
        this->init();
    }
    
    Pmm::~Pmm()
    {
    }

    void Pmm::init()
    {
        allocBase = ((size_t)&kernelTop) - KERNEL_PHYS_OFFSET;

        debugger.printf((char*)"Initialicing PMM, alloc base: %x\n", allocBase);

        for (size_t i = 0; i < memmap->entries; i++)
        {
            if(memmap->address[i].type != USABLE) {
                continue;
            }

            debugger.printf((char*)"base/size before alignment: %x %x\n", memmap->address[i].base, memmap->address[i].size);

            auto base = AlignUp(memmap->address[i].base, PAGE_SIZE);
            auto size = memmap->address[i].size - (base - memmap->address[i].base);
            size = AlignDown(size, PAGE_SIZE);
            auto top = base + size;

            if(base < allocBase) {
                if(top > allocBase) {
                    size -= allocBase - base;
                    base = allocBase;
                } else {
                    memmap->address[i].type = RESERVED;
                    debugger.printf((char*)"unusable memory area\n");
                    continue;
                }
            }

            debugger.printf((char*)"base/size after alignment: %x %x\n", base, size);

            memmap->address[i].base = base;
            memmap->address[i].size = size;

            if(top > topPage) {
                topPage = top;
            }
        }
        
        debugger.printf((char*)"PMM: Top page addr: %x\n", topPage);

        size_t bitmapSize = (topPage / PAGE_SIZE) / 8;

        debugger.printf((char*)"PMM: So %x bytes.\n", bitmapSize);

        for (size_t i = 0; i < memmap->entries; i++)
        {
            if(memmap->address[i].type != USABLE) {
                continue;
            }

            if(memmap->address[i].size >= bitmapSize) {
                bitmap = (size_t*)(memmap->address[i].base + MEM_PHYS_OFFSET);

                memmap->address[i].size -= bitmapSize;
                memmap->address[i].base += bitmapSize;

                for (size_t j = 0; j < (bitmapSize / sizeof(size_t)); j++)
                {
                    bitmap[j] = ~((size_t)0);
                }

                break;
            }
        }

        for (size_t i = 0; i < memmap->entries; i++)
        {
            if(memmap->address[i].type != USABLE) {
                continue;
            }

            for (size_t j = 0; j < memmap->address[i].size; j += PAGE_SIZE) {
                size_t page = (memmap->address[i].base + j) / PAGE_SIZE;
                reset_bit(bitmap, page);
            }
        }
    }

    void* Pmm::allocInternal(size_t count, size_t limit) {
        size_t p = 0;
        while (endIndex < limit) {
            if(!test_bit((size_t)bitmap, endIndex++)) {
                if(++p == count) {
                    size_t page = endIndex - count;
                    for (size_t i = 0; i < endIndex; i++)
                    {
                        set_bit(bitmap, i);
                    }
                    return (void*)(page * PAGE_SIZE);
                }
            } else {
                p = 0;
            }
        }
        return nullptr;
    }

    void* Pmm::pmmAllocNoZero(size_t count) {
        pmmLock.acquire();

        size_t e = endIndex;
        void* ret = allocInternal(count, topPage / PAGE_SIZE);
        if (ret == nullptr) {
            endIndex = 0;
            ret = allocInternal(count, e);
        }

        pmmLock.release();
        return ret;
    }

    void* Pmm::pmmAlloc(size_t count) {
        auto ret = (uint64_t*)((size_t)pmmAllocNoZero(count) + MEM_PHYS_OFFSET);

        for (size_t i = 0; i < (count / sizeof(uint64_t)); i++)
        {
            ret[i] = 0;
        }
        
        return (void*)((size_t)(ret) - MEM_PHYS_OFFSET);
    }

    void Pmm::pmmFree(void* ptr, size_t count) {
        pmmLock.acquire();

        size_t page = (size_t)ptr / PAGE_SIZE;
        for (size_t i = 0; i < (page + count); i++)
        {
            reset_bit(bitmap, i);
        }

        pmmLock.release();
    }

    static size_t AlignUp(size_t value, size_t alignment) {
        if ((value & (alignment - 1)) != 0) {
            value &= ~(alignment - 1);
            value += alignment;
        }
        return value;
    }

    static size_t AlignDown(size_t value, size_t alignment) {
        if ((value & (alignment - 1)) != 0) {
            value &= ~(alignment - 1);
        }
        return value;
    }
};