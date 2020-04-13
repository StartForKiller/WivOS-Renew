section .stivalehdr

align 4

stivale_header:
    .stack: dq stackTop
    .videomode: dw 0
    .fbwidth: dw 0
    .fbheight: dw 0
    .fbbpp: dw 0

section .text

extern callConstructors
extern kmain

global loader
loader:
    call callConstructors
    push rax
    call kmain

section .bss

align 16
stack:
    resb 16384
stackTop: