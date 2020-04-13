extern "C" {
    #include <boot/stivale.h>
    #include <debugging/debugger.hpp>
    #include <main.hpp>

    typedef void (*constructor)();
    extern "C" constructor start_ctors;
    extern "C" constructor end_ctors;
    extern "C" void callConstructors()
    {
        for(constructor* i = &start_ctors; i != &end_ctors; i++)
            (*i)();
    }

    void kmain(struct stivale_struct_t *stivale) {
        stivale = (struct stivale_struct_t*)((size_t)stivale + MEM_PHYS_OFFSET);
        WivOS::Kernel kernel(stivale);
        kernel.init();
        while(1);
    }
}

WivOS::Vmm *kernelPml4 = nullptr;
WivOS::Pmm *kernelPmm = nullptr;
WivOS::Debugger *kernelDebugger = nullptr;

namespace WivOS {
    Kernel::Kernel(struct stivale_struct_t *stivale) : pmm(&stivale->memmap, debugger), gdt(debugger, pmm), pit(debugger, 1000), pic(debugger), idt(debugger), acpi(debugger) {
        this->stivale = stivale;
    }
    Kernel::~Kernel() {}

    void Kernel::init() {
        kernelDebugger = &debugger;
        kernelPmm = &pmm;
        gdt.init();
        pit.init();
        pic.init();
        idt.init();

        Vmm aSpace(&stivale->memmap, debugger, pmm);
        kernelPml4 = &aSpace;
        kernelPml4->setActive();

        acpi.init();

        asm volatile("sti");

        this->debugger.printf((char*)"WivOS Initialization done, at least for now\n");

        while(1);
    }
};