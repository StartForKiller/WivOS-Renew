#include <sys/pit.hpp>
#include <lib/ports.hpp>

namespace WivOS {
    PIT::PIT(Debugger debugger, uint16_t pitFrequency) : debugger(debugger), pitFrequency(pitFrequency)
    {
    }
    
    PIT::~PIT()
    {
    }

    void PIT::init() {
        uint32_t divisor = 1193180 / pitFrequency;

        outb(0x43, 0x36);

        uint8_t l = (uint8_t)(divisor & 0xFF);
        uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

        outb(0x40, l);
        outb(0x40, h);
    }
};