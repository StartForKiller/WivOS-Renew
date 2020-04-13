#pragma once

#include <boot/stivale.h>
#include <debugging/debugger.hpp>
#include <mm/pmm.hpp>
#include <sys/gdt.hpp>
#include <sys/pit.hpp>
#include <sys/pic.hpp>
#include <sys/idt.hpp>
#include <mm/vmm.hpp>
#include <acpi/acpi.hpp>

namespace WivOS {
    class Kernel
    {
    private:
        struct stivale_struct_t *stivale;

        Debugger debugger;
        Pmm pmm;
        GDT gdt;
        PIT pit;
        PIC pic;
        IDT idt;
        ACPI acpi;
    public:
        Kernel(struct stivale_struct_t *stivale);
        ~Kernel();
        void init();
    };
    
}