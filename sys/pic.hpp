#pragma once

#include <stdint.h>
#include <stddef.h>
#include <debugging/debugger.hpp>

namespace WivOS {
    class PIC
    {
    private:
        Debugger debugger;
        
    public:
        PIC(Debugger debugger);
        ~PIC();

        void init();
    };
    
};