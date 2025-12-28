/*
 * Minuteman D17B/D37C Instruction Table
 *
 * Opcodes from:
 * - D17B Computer Programming Manual (Sep 1971)
 * - AFIT Thesis GE/EE/74-16 (Mar 1974)
 */

#include "asm.h"
#include <string.h>

/* Instruction table
 * Opcodes are in OCTAL as per original documentation
 */
static const instr_t instructions[] = {
    /* ========== ARITHMETIC ========== */
    /* D17B Arithmetic - available on both machines */
    { "CLA",  044, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Clear and Add */
    { "STO",  054, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Store Accumulator */
    { "ADD",  064, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Add */
    { "SAD",  060, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Split Add */
    { "SUB",  074, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Subtract */
    { "SSU",  070, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Split Subtract */
    { "MPY",  024, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Multiply */
    { "SMP",  020, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Split Multiply */
    { "MPM",  034, 0,    ITYPE_MEMORY,  MACH_D17B_ONLY },  /* Multiply Magnitude - D37C uses DIV */
    { "SMM",  030, 0,    ITYPE_MEMORY,  MACH_D17B_ONLY },  /* Split Multiply Magnitude */
    { "COM",  040, 046,  ITYPE_CONTROL, MACH_BOTH },  /* Complement */
    { "MIM",  040, 044,  ITYPE_CONTROL, MACH_BOTH },  /* Minus Magnitude */

    /* D37C Additional Arithmetic */
    { "DIV",  034, 0,    ITYPE_MEMORY,  MACH_D37C_ONLY },  /* Divide - replaces MPM */
    { "ORA",  040, 040,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* OR to Accumulator */
    { "AWC",  040, 050,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Add Without Carry */
    { "PMI",  040, 056,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Plus Magnitude */
    { "SPM",  040, 066,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Split Plus Magnitude */

    /* ========== SHIFT ========== */
    /* D17B Shifts */
    { "ALS",  000, 022,  ITYPE_SHIFT,   MACH_BOTH },  /* Accumulator Left Shift */
    { "ARS",  000, 032,  ITYPE_SHIFT,   MACH_BOTH },  /* Accumulator Right Shift */
    { "SAL",  000, 020,  ITYPE_SHIFT,   MACH_BOTH },  /* Split Accumulator Left */
    { "SAR",  000, 030,  ITYPE_SHIFT,   MACH_BOTH },  /* Split Accumulator Right */
    { "SLL",  000, 024,  ITYPE_SHIFT,   MACH_D17B_ONLY },  /* Split Left, Left Shift */
    { "SLR",  000, 034,  ITYPE_SHIFT,   MACH_D17B_ONLY },  /* Split Left, Right Shift */
    { "SRL",  000, 026,  ITYPE_SHIFT,   MACH_D17B_ONLY },  /* Split Right, Left - D37C uses ALC */
    { "SRR",  000, 036,  ITYPE_SHIFT,   MACH_D17B_ONLY },  /* Split Right, Right - D37C uses ARC */

    /* D37C Shifts - ALC/ARC replace SRL/SRR at same encoding */
    { "ALC",  000, 026,  ITYPE_SHIFT,   MACH_D37C_ONLY },  /* Accumulator Left Cycle */
    { "ARC",  000, 036,  ITYPE_SHIFT,   MACH_D37C_ONLY },  /* Accumulator Right Cycle */

    /* ========== CONTROL ========== */
    /* D17B Control */
    { "TRA",  050, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Transfer (jump) */
    /*
     * TMI: Different opcode on each machine!
     * D17B: opcode 010 (the D37C reused this for TZE)
     * D37C: opcode 030
     */
    { "TMI",  010, 0,    ITYPE_MEMORY,  MACH_D17B_ONLY },  /* Transfer on Minus - D17B */
    { "TMI",  030, 0,    ITYPE_MEMORY,  MACH_D37C_ONLY },  /* Transfer on Minus - D37C */
    { "HPR",  040, 022,  ITYPE_IMPLIED, MACH_BOTH },  /* Halt and Proceed */
    { "SCL",  004, 0,    ITYPE_MEMORY,  MACH_BOTH },  /* Split Compare and Limit */
    { "ANA",  040, 042,  ITYPE_CONTROL, MACH_BOTH },  /* AND to Accumulator */
    { "LPR",  040, 070,  ITYPE_CONTROL, MACH_BOTH },  /* Load Phase Register */
    { "EFC",  040, 062,  ITYPE_CONTROL, MACH_BOTH },  /* Enable Fine Countdown */
    { "HFC",  040, 060,  ITYPE_CONTROL, MACH_BOTH },  /* Halt Fine Countdown */
    { "RSD",  040, 020,  ITYPE_CONTROL, MACH_BOTH },  /* Reset Detector */

    /* D37C Additional Control */
    { "TZE",  010, 0,    ITYPE_MEMORY,  MACH_D37C_ONLY },  /* Transfer on Zero - reuses D17B TMI opcode */
    { "FCL",  014, 0,    ITYPE_MEMORY,  MACH_D37C_ONLY },  /* Full Compare and Limit */
    { "TSM",  040, 006,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Transfer Sector on Minus */
    { "TSZ",  040, 004,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Transfer Sector on Zero */
    { "GBP",  040, 064,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Generate Bit Pattern */
    { "GPT",  040, 060,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Generate Parity Bit */
    { "MAL",  040, 002,  ITYPE_CONTROL, MACH_D37C_ONLY },  /* Modify A and L */

    /* ========== I/O ========== */
    /* D17B I/O */
    { "DIA",  040, 052,  ITYPE_IO,      MACH_BOTH },  /* Discrete Input A */
    { "DIB",  040, 050,  ITYPE_IO,      MACH_BOTH },  /* Discrete Input B */
    { "DOA",  040, 026,  ITYPE_IO,      MACH_BOTH },  /* Discrete Output A */
    { "VOA",  040, 030,  ITYPE_IO,      MACH_D17B_ONLY },  /* Voltage Output A */
    { "VOB",  040, 032,  ITYPE_IO,      MACH_D17B_ONLY },  /* Voltage Output B */
    { "VOC",  040, 034,  ITYPE_IO,      MACH_D17B_ONLY },  /* Voltage Output C */
    { "BOA",  040, 010,  ITYPE_IO,      MACH_D17B_ONLY },  /* Binary Output A */
    { "BOB",  040, 012,  ITYPE_IO,      MACH_D17B_ONLY },  /* Binary Output B */
    { "BOC",  040, 002,  ITYPE_IO,      MACH_D17B_ONLY },  /* Binary Output C */
    { "COA",  000, 040,  ITYPE_IO,      MACH_BOTH },  /* Character Output A */

    /* D37C Additional I/O */
    { "DIC",  040, 020,  ITYPE_IO,      MACH_D37C_ONLY },  /* Discrete Input C */
    { "DOB",  040, 054,  ITYPE_IO,      MACH_D37C_ONLY },  /* Discrete Output B */
    { "VIA",  040, 010,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input A */
    { "VIB",  040, 012,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input B */
    { "VIC",  040, 014,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input C */
    { "VID",  040, 016,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input D */
    { "VIE",  040, 030,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input E */
    { "VIF",  040, 032,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input F */
    { "VIG",  040, 034,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input G */
    { "VIH",  040, 036,  ITYPE_IO,      MACH_D37C_ONLY },  /* Voltage Input H */
    { "ECI",  040, 062,  ITYPE_IO,      MACH_D37C_ONLY },  /* Enable Cable Input */
    { "ECO",  040, 062,  ITYPE_IO,      MACH_D37C_ONLY },  /* Enable Cable Output */
    { "EPP",  040, 062,  ITYPE_IO,      MACH_D37C_ONLY },  /* Enable Platform Power */
    { "DPP",  040, 062,  ITYPE_IO,      MACH_D37C_ONLY },  /* Disable Platform Power */
    { "RIC",  000, 024,  ITYPE_IO,      MACH_D37C_ONLY },  /* Radio Intercommunication */
    { "SRD",  000, 016,  ITYPE_IO,      MACH_D37C_ONLY },  /* Simulate Transient */

    /* End marker */
    { NULL, 0, 0, 0, MACH_BOTH }
};

/*
 * Find instruction by mnemonic, respecting machine mode
 *
 * In D17B mode: returns MACH_BOTH or MACH_D17B_ONLY entries
 * In D37C mode: returns MACH_BOTH or MACH_D37C_ONLY entries
 *
 * For instructions like TMI that have different encodings per machine,
 * this ensures the correct opcode is used.
 */
const instr_t *find_instruction(const char *mnemonic, bool d37c_mode) {
    for (int i = 0; instructions[i].mnemonic != NULL; i++) {
        if (str_eq_nocase(instructions[i].mnemonic, mnemonic)) {
            mach_compat_t compat = instructions[i].compat;

            /* Check if this entry is valid for the current mode */
            if (d37c_mode) {
                /* D37C mode: accept MACH_BOTH or MACH_D37C_ONLY */
                if (compat == MACH_BOTH || compat == MACH_D37C_ONLY) {
                    return &instructions[i];
                }
            } else {
                /* D17B mode: accept MACH_BOTH or MACH_D17B_ONLY */
                if (compat == MACH_BOTH || compat == MACH_D17B_ONLY) {
                    return &instructions[i];
                }
            }
            /* Name matches but wrong mode - keep searching for mode-specific version */
        }
    }
    return NULL;
}
