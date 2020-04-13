#include <mm/vmm.hpp>

namespace WivOS {
    Vmm::Vmm(struct stivale_memmap_t *memmap, Debugger debugger, Pmm pmm) : pmm(pmm)
    {
        this->debugger = debugger;
        this->pml4 = (size_t*)((size_t)pmm.pmmAlloc(1) + MEM_PHYS_OFFSET);
        this->lock.release();

        for(size_t i = 0; i < 0x100000000; i+= 0x200000) {
            this->mapPageHuge(i, MEM_PHYS_OFFSET + i, 0x03);
        }

        for(size_t i = 0; i < 0x80000000; i+= 0x200000) {
            this->mapPageHuge(i, KERNEL_PHYS_OFFSET + i, 0x03);
        }

        for(auto i = 0; i < memmap->entries; i++) {
            auto base = memmap->address[i].base;
            auto size = memmap->address[i].size;

            size_t alignedBase = base - (base % PAGE_SIZE);
            size_t alignedSize = (size / PAGE_SIZE) * PAGE_SIZE;

            if (size % PAGE_SIZE) {
                alignedSize += PAGE_SIZE;
            }

            if (base % PAGE_SIZE) {
                alignedSize += PAGE_SIZE;
            }

            for (uint64_t j = 0; j * PAGE_SIZE < alignedSize; j++) {
                size_t addr = alignedBase + j * PAGE_SIZE;

                if (addr < 0x100000000) {
                    continue;
                }

                this->mapPage(addr, MEM_PHYS_OFFSET + addr, 0x03);
            }
        }
    }
    
    Vmm::~Vmm()
    {
    }

    size_t* Vmm::finOrAllocPageTable(size_t* table, size_t index, size_t flags) {
        auto ret = findPageTable(table, index);

        if(ret == nullptr) {
            ret = (size_t*)((size_t)this->pmm.pmmAlloc(1) + MEM_PHYS_OFFSET);
            table[index] = ((size_t)ret - MEM_PHYS_OFFSET) | flags;
        }

        return ret;
    }

    size_t* Vmm::findPageTable(size_t* table, size_t index) {
        if(table[index] & PT_PRESENT) {
            return (size_t*)((table[index] & ~((size_t)0xFFF)) + MEM_PHYS_OFFSET);
        } else {
            return nullptr;
        }
    }

    void Vmm::cleanPageTable(size_t* table) {
        for(size_t i = 0; ; i++) {
            if (i == PT_ENTRIES) {
                this->pmm.pmmFree((void*)((size_t)table - MEM_PHYS_OFFSET), 1);
            } else if (table[i] & PT_PRESENT) {
                return;
            }
        }
    }

    void Vmm::setActive() {
        this->lock.acquire();
        asm volatile("mov %%rax, %%cr3" : : "a"((size_t)(this->pml4) - MEM_PHYS_OFFSET));
        this->lock.release();
    }

    void Vmm::mapPage(size_t pAddress, size_t vAddress, size_t flags) {
        this->lock.acquire();

        auto pml4Entry = (vAddress & ((size_t)0x1FF << 39)) >> 39;
        auto pml3Entry = (vAddress & ((size_t)0x1FF << 30)) >> 30;
        auto pml2Entry = (vAddress & ((size_t)0x1FF << 21)) >> 21;
        auto pml1Entry = (vAddress & ((size_t)0x1FF << 12)) >> 12;

        size_t* pml3 = finOrAllocPageTable(this->pml4, pml4Entry, 0b111);
        size_t* pml2 = finOrAllocPageTable(pml3, pml3Entry, 0b111);
        size_t* pml1 = finOrAllocPageTable(pml2, pml2Entry, 0b111);

        pml1[pml1Entry] = pAddress | flags;

        this->lock.release();
    }

    void Vmm::mapPageHuge(size_t pAddress, size_t vAddress, size_t flags) {
        this->lock.acquire();

        auto pml4Entry = (vAddress & ((size_t)0x1FF << 39)) >> 39;
        auto pml3Entry = (vAddress & ((size_t)0x1FF << 30)) >> 30;
        auto pml2Entry = (vAddress & ((size_t)0x1FF << 21)) >> 21;
        //auto pml1Entry = (vAddress & ((size_t)0x1FF << 12)) >> 12;

        size_t* pml3 = finOrAllocPageTable(this->pml4, pml4Entry, 0b111);
        size_t* pml2 = finOrAllocPageTable(pml3, pml3Entry, 0b111);

        pml2[pml2Entry] = pAddress | flags | (1ull<<7);

        this->lock.release();
    }

    void Vmm::unmapPage(size_t vAddress) {
        this->lock.acquire();

        auto pml4Entry = (vAddress & ((size_t)0x1FF << 39)) >> 39;
        auto pml3Entry = (vAddress & ((size_t)0x1FF << 30)) >> 30;
        auto pml2Entry = (vAddress & ((size_t)0x1FF << 21)) >> 21;
        auto pml1Entry = (vAddress & ((size_t)0x1FF << 12)) >> 12;

        size_t* pml3 = findPageTable(this->pml4, pml4Entry);
        if(pml3 == nullptr) return;
        size_t* pml2 = findPageTable(pml3, pml3Entry);
        if(pml2 == nullptr) return;
        size_t* pml1 = findPageTable(pml2, pml2Entry);
        if(pml1 == nullptr) return;

        pml1[pml1Entry] = 0;

        cleanPageTable(pml3);
        cleanPageTable(pml2);
        cleanPageTable(pml1);

        this->lock.release();
    }

    size_t Vmm::getPhys(size_t vAddress) {
        this->lock.acquire();

        auto pml4Entry = (vAddress & ((size_t)0x1FF << 39)) >> 39;
        auto pml3Entry = (vAddress & ((size_t)0x1FF << 30)) >> 30;
        auto pml2Entry = (vAddress & ((size_t)0x1FF << 21)) >> 21;
        auto pml1Entry = (vAddress & ((size_t)0x1FF << 12)) >> 12;

        size_t* pml3 = findPageTable(this->pml4, pml4Entry);
        if(pml3 == nullptr) return 0;
        size_t* pml2 = findPageTable(pml3, pml3Entry);
        if(pml2 == nullptr) return 0;
        size_t* pml1 = findPageTable(pml2, pml2Entry);
        if(pml1 == nullptr) return 0;

        this->lock.release();

        return pml1[pml1Entry] & ~(0xFFF | (1ull << 63));
    }
};