#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <klib.h>
#include <panic.h>
#include <acpi.h>

rsdp_t *rsdp;
rsdt_t *rsdt;
madt_t *madt;

local_apic_t *local_apics[MAX_MADT];
size_t local_apic_ptr = 0;

io_apic_t *io_apics[MAX_MADT];
size_t io_apic_ptr = 0;

iso_t *isos[MAX_MADT];
size_t iso_ptr = 0;

nmi_t *nmis[MAX_MADT];
size_t nmi_ptr = 0;

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

    /* parse the MADT entries */
    for (uint8_t *madt_ptr = (uint8_t *)(&madt->madt_entries_begin);
        (size_t)madt_ptr < (size_t)madt + madt->sdt.length;
        madt_ptr += *(madt_ptr + 1)) {
        switch (*(madt_ptr)) {
            case 0:
                /* processor local APIC */
                kprint(KPRN_INFO, "Found local APIC #%u", local_apic_ptr);
                local_apics[local_apic_ptr++] = (local_apic_t *)madt_ptr;
                break;
            case 1:
                /* I/O APIC */
                kprint(KPRN_INFO, "Found I/O APIC #%u", io_apic_ptr);
                io_apics[io_apic_ptr++] = (io_apic_t *)madt_ptr;
                break;
            case 2:
                /* interrupt source override */
                kprint(KPRN_INFO, "Found ISO #%u", iso_ptr);
                isos[iso_ptr++] = (iso_t *)madt_ptr;
                break;
            case 4:
                /* NMI */
                kprint(KPRN_INFO, "Found NMI #%u", nmi_ptr);
                nmis[nmi_ptr++] = (nmi_t *)madt_ptr;
                break;
            default:
                break;
        }
    }

    return;

}
