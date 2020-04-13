#pragma once

#include <stdint.h>
#include <stddef.h>
#include <debugging/debugger.hpp>

struct IDTDescriptor {
    uint16_t offsetLow;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offsetMiddle;
    uint32_t offsetHigh;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPointer {
    uint16_t size;
    void* offset;
} __attribute__((packed));

struct idt_regs {
    size_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx, rax, rip, cs, rflags, rsp, ss;
};

typedef void (*idtHandler)();

namespace WivOS {
    class IDT
    {
    private:
        Debugger debugger;
        IDTDescriptor idtEntries[256];
        IDTPointer idtPointer;
        uint8_t currentInterrupt;

        void addInterrupt(uint32_t index, idtHandler handler, uint8_t ist);

    public:
        IDT(Debugger debugger);
        ~IDT();

        void init();

        static void defaultInterruptHandler();
        static void pitHandler();
        static void handlerHandle(idt_regs *regs, size_t error_code, int exception);

        void registerHandler();
    };
    
};