#pragma once

#include <stdint.h>
#include <stddef.h>
#include <debugging/debugger.hpp>

namespace WivOS {
    class PIT
    {
    private:
        Debugger debugger;
        uint16_t pitFrequency;
        
    public:
        PIT(Debugger debugger, uint16_t pitFrequency);
        ~PIT();

        void init();
    };
    
};