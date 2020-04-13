#include <acpi/acpi.hpp>
#include <mm/vmm.hpp>
#include <lai/host.h>
#include <lai/core.h>
#include <acpispec/tables.h>
#include <lib/ports.hpp>

extern "C" {
int strncmp(const char *s1, const char *s2, size_t n);
};

WivOS::ACPI *ACPIInstance = nullptr;

namespace WivOS {
    ACPI::ACPI(Debugger debugger)
    {
        this->debugger = debugger;
    }
    
    ACPI::~ACPI()
    {
    }

    void ACPI::init()
    {
        debugger.printf((char*)"[ACPI] Initialicing...\n");

        for (size_t i = 0x80000 + MEM_PHYS_OFFSET; i < 0x100000  + MEM_PHYS_OFFSET; i += 16) {
            if (i == 0xa0000 + MEM_PHYS_OFFSET) {
                i = 0xe0000 - 16 + MEM_PHYS_OFFSET;
                continue;
            }
            if (!strncmp((char *)i, "RSD PTR ", 8)) {
                debugger.printf((char*)"[ACPI] Found RSDP at %x\n", i);
                rsdp = (struct rsdp_t *)i;
                goto rsdp_found;
            }
        }

        debugger.printf((char*)"[ACPI] Non-ACPI compliant system\n");
        while(1);

    rsdp_found:
        debugger.printf((char*)"[ACPI] ACPI available\n");
        debugger.printf((char*)"[ACPI] Revision: %u\n", (uint64_t)rsdp->rev);
        debugger.printf((char*)"[ACPI] Found RSDT at %x\n", ((size_t)rsdp->rsdt_addr + MEM_PHYS_OFFSET));
        rsdt = (struct rsdt_t *)((size_t)rsdp->rsdt_addr + MEM_PHYS_OFFSET);
    
        ACPIInstance = this;

        lai_set_acpi_revision(rsdp->rev);
        //lai_enable_tracing(1);
        lai_create_namespace();
    }

    void * ACPI::findSdt(const char *signature, int index) {
        struct sdt_t *ptr;
        int cnt = 0;

        for(size_t i = 0; i < (rsdt->sdt.length - sizeof(struct sdt_t)) / 4; i++) {
            ptr = (struct sdt_t *)((size_t)rsdt->sdt_ptr[i] + MEM_PHYS_OFFSET);
            if(!strncmp(ptr->signature, signature, 4)) {
                if(cnt++ == index) {
                    debugger.printf((char*)"[ACPI] Found \"%s\" at %x\n", signature, (size_t)ptr);
                    return (void *)ptr;
                }
            }
        }

        debugger.printf((char*)"[ACPI] \"%s\" not found\n", signature);
        return nullptr;
    }
};

extern WivOS::Debugger *kernelDebugger;

extern "C" {
    void laihost_log(int level, const char *str) {
        kernelDebugger->printf((char*)str);
        kernelDebugger->printf((char*)"\n");
    }

    void laihost_panic(const char *str) {
        kernelDebugger->printf((char*)str);
        kernelDebugger->printf((char*)"\n");
        while(1);
    }

    void *laihost_malloc(size_t size) {
        return kmalloc(size);
    }

    void *laihost_realloc(void *p, size_t size) {
        return krealloc(p, size);
    }

    void laihost_free(void *p) {
        return kfree(p);
    }

    void *laihost_scan(const char *signature, size_t index) {
        if (!strncmp(signature, "DSDT", 4)) {
            if (index > 0) {
                return NULL;
            }
            // Scan for the FADT
            acpi_fadt_t *fadt = (acpi_fadt_t *)ACPIInstance->findSdt("FACP", 0);
            void *dsdt = (char *)(size_t)fadt->dsdt + MEM_PHYS_OFFSET;
            return dsdt;
        } else {
            return ACPIInstance->findSdt(signature, index);
        }
    }

    uint8_t laihost_inb(uint16_t port) {
        return inb(port);
    }

    void laihost_outb(uint16_t port, uint8_t value) {
        outb(port, value);
    }

    uint16_t laihost_inw(uint16_t port) {
        return inw(port);
    }

    void laihost_outw(uint16_t port, uint16_t value) {
        outw(port, value);
    }

    uint32_t laihost_ind(uint16_t port) {
        return ind(port);
    }

    void laihost_outd(uint16_t port, uint32_t value) {
        outd(port, value);
    }

    void *laihost_map(size_t phys_addr, size_t count) {
        (void)count;
        size_t virt_addr = phys_addr + MEM_PHYS_OFFSET;
        return (void *)virt_addr;
    }
};