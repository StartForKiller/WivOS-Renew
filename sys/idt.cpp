#include <sys/idt.hpp>
#include <sys/gdt.hpp>
#include <lib/ports.hpp>

static WivOS::IDT *IDTIntance = nullptr;

extern "C" void exception_handler(int exception, idt_regs *regs, size_t error_code) {
    WivOS::IDT::handlerHandle(regs, error_code, exception);
}

extern "C" {
    void exc_div0_handler(void);
    void exc_debug_handler(void);
    void exc_nmi_handler(void);
    void exc_breakpoint_handler(void);
    void exc_overflow_handler(void);
    void exc_bound_range_handler(void);
    void exc_inv_opcode_handler(void);
    void exc_no_dev_handler(void);
    void exc_double_fault_handler(void);
    void exc_inv_tss_handler(void);
    void exc_no_segment_handler(void);
    void exc_ss_fault_handler(void);
    void exc_gpf_handler(void);
    void exc_page_fault_handler(void);
    void exc_x87_fp_handler(void);
    void exc_alignment_check_handler(void);
    void exc_machine_check_handler(void);
    void exc_simd_fp_handler(void);
    void exc_virt_handler(void);
    void exc_security_handler(void);
}

namespace WivOS {
    IDT::IDT(Debugger debugger) : debugger(debugger)
    {
    }
    
    IDT::~IDT()
    {
    }

    void IDT::init() {
        IDTIntance = this;
        asm volatile("cli");

        idtPointer.size = (16*256);
        idtPointer.offset = ((void*)idtEntries);

        addInterrupt(0x00, &exc_div0_handler, 0);
        addInterrupt(0x01, &exc_debug_handler, 0);
        addInterrupt(0x02, &exc_nmi_handler, 0);
        addInterrupt(0x03, &exc_breakpoint_handler, 0);
        addInterrupt(0x04, &exc_overflow_handler, 0);
        addInterrupt(0x05, &exc_bound_range_handler, 0);
        addInterrupt(0x06, &exc_inv_opcode_handler, 0);
        addInterrupt(0x07, &exc_no_dev_handler, 0);
        addInterrupt(0x08, &exc_double_fault_handler, 0);
        addInterrupt(0x0a, &exc_inv_tss_handler, 0);
        addInterrupt(0x0b, &exc_no_segment_handler, 0);
        addInterrupt(0x0c, &exc_ss_fault_handler, 0);
        addInterrupt(0x0d, &exc_gpf_handler, 0);
        addInterrupt(0x0e, &exc_page_fault_handler, 0);
        addInterrupt(0x10, &exc_x87_fp_handler, 0);
        addInterrupt(0x11, &exc_alignment_check_handler, 0);
        addInterrupt(0x12, &exc_machine_check_handler, 0);
        addInterrupt(0x13, &exc_simd_fp_handler, 0);
        addInterrupt(0x14, &exc_virt_handler, 0);
        addInterrupt(0x1e, &exc_security_handler, 0);

        addInterrupt(0x20, &pitHandler, 0);
        
        __asm__ volatile("lidt (%0)" : : "r" (&idtPointer));
    }

    void IDT::addInterrupt(uint32_t index, idtHandler handler, uint8_t ist) {
        auto addr = (size_t)handler;

        idtEntries[index].offsetLow = (uint16_t)addr;
        idtEntries[index].selector = CODE_SEGMENT;
        idtEntries[index].ist = ist;
        idtEntries[index].flags = 0x8E;
        idtEntries[index].offsetMiddle = (uint16_t)(addr >> 16);
        idtEntries[index].offsetHigh = (uint32_t)(addr >> 32);
        idtEntries[index].reserved = 0;
    }

    void IDT::handlerHandle(idt_regs *regs, size_t error_code, int exception) {
        IDTIntance->debugger.printf((char*)"Hola:( %x %x %x ", regs->rip, error_code, exception);
        while(1);
    }

    void IDT::pitHandler() {
        IDTIntance->debugger.printf((char*)"Tick");
    }

};