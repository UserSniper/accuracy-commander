/* Commander X16 PRG linker script.
 *
 * BASIC memory regions are freely overwritten, but any KERNAL regions are left alone.
 *
 * Produces a PRG file with a SYS command to start the program.
 */

__basic_zp_start = 0x0024;
__basic_zp_end = 0x0080;

MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x96FF
}

INCLUDE commodore.ld

SECTIONS {
    /* put any sections you might want here. */
}

/* Set initial soft stack address (It grows down) */
__stack = 0x9eff;

OUTPUT_FORMAT {
    /* Tells the LOAD command where to place the file's contents. */
    SHORT(0x0801)
    TRIM(ram)
}