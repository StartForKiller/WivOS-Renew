#pragma once

#define outb(port, val) ({ \
    asm volatile ( \
        "outb %0, %1;" \
        : \
        : "a" ((uint8_t)val), "Nd" ((uint16_t)port) \
    ); \
})

#define inb(port) ({ \
    uint8_t ret; \
    asm volatile ( \
        "inb %1, %0;" \
        : "=a" (ret) \
        : "Nd" ((uint16_t)port) \
    ); \
    ret; \
})

#define outw(port, val) ({ \
    asm volatile ( \
        "outw %0, %1;" \
        : \
        : "a" ((uint16_t)val), "Nd" ((uint16_t)port) \
    ); \
})

#define inw(port) ({ \
    uint16_t ret; \
    asm volatile ( \
        "inw %1, %0;" \
        : "=a" (ret) \
        : "Nd" ((uint16_t)port) \
    ); \
    ret; \
})

#define outd(port, val) ({ \
    asm volatile ( \
        "outl %0, %1;" \
        : \
        : "a" ((uint32_t)val), "Nd" ((uint16_t)port) \
    ); \
})

#define ind(port) ({ \
    uint32_t ret; \
    asm volatile ( \
        "inl %1, %0;" \
        : "=a" (ret) \
        : "Nd" ((uint16_t)port) \
    ); \
    ret; \
})
