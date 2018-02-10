#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <klib.h>
#include <panic.h>

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t rev;
    uint32_t rsdt_addr;
} __attribute__((packed)) rsdp_t;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t rev;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_rev;
    uint32_t creator_id;
    uint32_t creator_rev;
} __attribute__((packed)) acpi_sdt_t;

typedef struct {
    acpi_sdt_t sdt;
    uint32_t sdt_ptr[65536];
} __attribute__((packed)) rsdt_t;

typedef struct {
    acpi_sdt_t sdt;
    uint32_t local_controller_addr;
    uint32_t flags;
} __attribute__((packed)) madt_t;

rsdp_t *rsdp;
rsdt_t *rsdt;
madt_t *madt;

void init_acpi(void) {
    kprint(KPRN_INFO, "Initialising ACPI...");

    /* look for the "RSD PTR " signature from 0x80000 to 0xa0000 */
                                           /* 0xf0000 to 0x100000 */
    for (size_t i = 0x80000; i < 0x100000; i++) {
        if (i == 0xa0000 - 4) {
            /* skip video mem and mapped hardware */
            i = 0xeffff;
            continue;
        }
        if (!kstrncmp((char *)i, "RSD PTR ", 8)) {
            kprint(KPRN_INFO, "Found RSDP at %x", i);
            rsdp = (rsdp_t *)i;
            goto rsdp_found;
        }
    }
    panic("RSDP table not found");

rsdp_found:
    kprint(KPRN_INFO, "Found RSDT at %x", rsdp->rsdt_addr);
    rsdt = (rsdt_t *)rsdp->rsdt_addr;

    /* search for MADT table */
    for (size_t i = 0; i < rsdt->sdt.length; i++) {
        madt = (madt_t *)rsdt->sdt_ptr[i];
        if (!kstrncmp(madt->sdt.signature, "APIC", 4)) {
            kprint(KPRN_INFO, "Found MADT at %x", (size_t)madt);
            goto madt_found;
        }
    }
    panic("MADT table not found");

madt_found:
    kprint(KPRN_INFO, "Length: %x", madt->sdt.length);
    kprint(KPRN_INFO, "Rev.: %u", madt->sdt.rev);
    kprint(KPRN_INFO, "OEMID: %k", madt->sdt.oem_id, 6);
    kprint(KPRN_INFO, "OEM table ID: %k", madt->sdt.oem_table_id, 8);
    kprint(KPRN_INFO, "OEM rev.: %u", madt->sdt.oem_rev);

    return;

}
