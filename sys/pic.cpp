#include <sys/pic.hpp>
#include <lib/ports.hpp>

#define PIC_EOI 0x20

#define MASTERPIC_COMMAND  0x20
#define MASTERPIC_DATA     0x21
#define SLAVEPIC_COMMAND   0xA0
#define SLAVEPIC_DATA      0xA1

namespace WivOS {
    PIC::PIC(Debugger debugger) : debugger(debugger)
    {
    }
    
    PIC::~PIC()
    {
    }

    void PIC::init() {
        auto masterMask = inb(MASTERPIC_DATA);
        auto slaveMask = inb(SLAVEPIC_DATA);

        outb(MASTERPIC_COMMAND, 0x11);
        outb(SLAVEPIC_COMMAND, 0x11);

        outb(MASTERPIC_DATA, 0xa0);
        outb(SLAVEPIC_DATA, 0xa8);

        outb(MASTERPIC_DATA, 4);
        outb(SLAVEPIC_DATA, 2);

        outb(MASTERPIC_DATA, 1);
        outb(SLAVEPIC_DATA, 1);

        outb(MASTERPIC_DATA, 0xff);
        outb(SLAVEPIC_DATA, 0xff);
    }
};