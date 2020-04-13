#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

namespace WivOS {
    class Debugger
    {
    private:
        const char *CONVERSION_TABLE = ((const char*)"0123456789ABCDEF");

        void print(char c);
        void print(char *s);
        void printInt(uint64_t x);
        void printHex(uint64_t x);
        void vprint(char *format, va_list args);
    public:
        Debugger();
        ~Debugger();
        void printf(char *message, ...);
    };
};