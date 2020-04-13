#include <debugging/debugger.hpp>
#include <lib/ports.hpp>

namespace WivOS {

    Debugger::Debugger() {
        
    }

    Debugger::~Debugger() {
        
    }

    void Debugger::print(char c) {
        outb(0xE9, c);
    }

    void Debugger::print(char *s) {
        for (auto i = 0; s[i] != '\0'; i++) {
            print(s[i]);
        }
    }

    void Debugger::printInt(uint64_t x) {
        int i;
        char buf[21];

        buf[20] = 0;

        if(!x) {
            print('0');
            return;
        }

        for (i = 19; x; i--) {
            buf[i] = CONVERSION_TABLE[x % 10];
            x /= 10;
        }

        i++;
        print(&buf[i]);
    }

    void Debugger::printHex(uint64_t x) {
        int i;
        char buf[17];

        buf[16] = 0;

        if(!x) {
            print((char*)"0x0");
            return;
        }

        for (i = 15; x; i--) {
            buf[i] = CONVERSION_TABLE[x % 16];
            x /= 16;
        }

        i++;
        print((char*)"0x");
        print(&buf[i]);
    }

    void Debugger::vprint(char *format, va_list args) {
        for (auto i = 0; format[i]; i++) {
            if (format[i] != '%') {
                print(format[i]);
                continue;
            }

            if (format[++i]) {
                switch (format[i]) {
                    case 's':
                        {
                            char *str = va_arg(args, char *);
                            print(str);
                        }
                        break;
                    case 'x':
                        {
                            uint64_t value = va_arg(args, uint64_t);
                            printHex(value);
                        }
                        break;
                    case 'u':
                        {
                            uint64_t value = va_arg(args, uint64_t);
                            printInt(value);
                        }
                        break;
                    default:
                        print('%');
                        print(format[i]);
                }
            }
        }
    }

    void Debugger::printf(char *message, ...) {
        va_list args;
        va_start(args, message);
        vprint(message, args);
        va_end(args);
    }

};