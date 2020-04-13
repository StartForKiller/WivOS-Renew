#include <sys/gdt.hpp>

namespace WivOS {
    GDT::GDT(Debugger debugger, Pmm pmm) : debugger(debugger), pmm(pmm)
    {
    }
    
    GDT::~GDT()
    {
    }

    void GDT::init() {
        gdtEntries[0].limit = 0;
        gdtEntries[0].baseLow = 0;
        gdtEntries[0].baseMid = 0;
        gdtEntries[0].access = 0;
        gdtEntries[0].granularity = 0;
        gdtEntries[0].baseHigh = 0;

        gdtEntries[1].limit = 0;
        gdtEntries[1].baseLow = 0;
        gdtEntries[1].baseMid = 0;
        gdtEntries[1].access = 0b10011010;
        gdtEntries[1].granularity = 0b00100000;
        gdtEntries[1].baseHigh = 0;

        gdtEntries[2].limit = 0;
        gdtEntries[2].baseLow = 0;
        gdtEntries[2].baseMid = 0;
        gdtEntries[2].access = 0b10010010;
        gdtEntries[2].granularity = 0b00000000;
        gdtEntries[2].baseHigh = 0;

        gdtPointer.size = 8 * 2;
        gdtPointer.address = ((void *)gdtEntries);

        asm volatile("lgdt (%0);"
            : 
            : "r"(&gdtPointer));

        asm volatile("cli");

        asm volatile ( "pushq $0x8\n"
			"\rpushq $.L_reloadCs\n"
			"\rlretq\n"
			".L_reloadCs:" );
    }
};