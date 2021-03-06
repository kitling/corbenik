#include <common.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>

#define FIRM_INTERNAL_CODE
#include <firm/internal.h>

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

void firmlaunch(firm_h* firm) {
    // Get entrypoints
    uint32_t  entry11 = firm->a11Entry;
    void_call entry9  = (void_call)firm->a9Entry;

    // Copy sections from FIRMs to their destination.
    for (firm_section_h *section = firm->section; section < firm->section + 4 && section->address != 0; section++) {
        memmove((void *)section->address, (void *)((uint8_t*)firm + section->offset), section->size);
    }

    free(firm); // Really, no point in this. Why not, though.

    fflush(stderr); // Flush logs if need be before unmount.

    fumount(); // Unmount SD.

    deinitScreens(); // Turn off display

    *a11_entry = (uint32_t)entry11; // Start kernel11

    entry9(); // Start process9
}

