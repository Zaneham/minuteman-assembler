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
    /* D17B Arithmetic */
    { "CLA",  044, 0,    ITYPE_MEMORY,  false },  /* Clear and Add */
    { "STO",  054, 0,    ITYPE_MEMORY,  false },  /* Store Accumulator */
    { "ADD",  064, 0,    ITYPE_MEMORY,  false },  /* Add */
    { "SAD",  060, 0,    ITYPE_MEMORY,  false },  /* Split Add */
    { "SUB",  074, 0,    ITYPE_MEMORY,  false },  /* Subtract */
    { "SSU",  070, 0,    ITYPE_MEMORY,  false },  /* Split Subtract */
    { "MPY",  024, 0,    ITYPE_MEMORY,  false },  /* Multiply */
    { "SMP",  020, 0,    ITYPE_MEMORY,  false },  /* Split Multiply */
    { "MPM",  034, 0,    ITYPE_MEMORY,  false },  /* Multiply Magnitude */
    { "SMM",  030, 0,    ITYPE_MEMORY,  false },  /* Split Multiply Magnitude */
    { "COM",  040, 046,  ITYPE_CONTROL, false },  /* Complement */
    { "MIM",  040, 044,  ITYPE_CONTROL, false },  /* Minus Magnitude */

    /* D37C Additional Arithmetic */
    { "DIV",  034, 0,    ITYPE_MEMORY,  true  },  /* Divide (hardware!) */
    { "ORA",  040, 040,  ITYPE_CONTROL, true  },  /* OR to Accumulator */
    { "AWC",  040, 050,  ITYPE_CONTROL, true  },  /* Add Without Carry */
    { "PMI",  040, 056,  ITYPE_CONTROL, true  },  /* Plus Magnitude */
    { "SPM",  040, 066,  ITYPE_CONTROL, true  },  /* Split Plus Magnitude */

    /* ========== SHIFT ========== */
    /* D17B Shifts */
    { "ALS",  000, 022,  ITYPE_SHIFT,   false },  /* Accumulator Left Shift */
    { "ARS",  000, 032,  ITYPE_SHIFT,   false },  /* Accumulator Right Shift */
    { "SAL",  000, 020,  ITYPE_SHIFT,   false },  /* Split Accumulator Left */
    { "SAR",  000, 030,  ITYPE_SHIFT,   false },  /* Split Accumulator Right */
    { "SLL",  000, 024,  ITYPE_SHIFT,   false },  /* Split Left, Left Shift */
    { "SLR",  000, 034,  ITYPE_SHIFT,   false },  /* Split Left, Right Shift */
    { "SRL",  000, 026,  ITYPE_SHIFT,   false },  /* Split Right, Left Shift */
    { "SRR",  000, 036,  ITYPE_SHIFT,   false },  /* Split Right, Right Shift */

    /* D37C Additional Shifts */
    { "ALC",  000, 026,  ITYPE_SHIFT,   true  },  /* Accumulator Left Cycle */
    { "ARC",  000, 036,  ITYPE_SHIFT,   true  },  /* Accumulator Right Cycle */

    /* ========== CONTROL ========== */
    /* D17B Control */
    { "TRA",  050, 0,    ITYPE_MEMORY,  false },  /* Transfer (jump) */
    { "TMI",  010, 0,    ITYPE_MEMORY,  false },  /* Transfer on Minus */
    { "HPR",  040, 022,  ITYPE_IMPLIED, false },  /* Halt and Proceed */
    { "SCL",  004, 0,    ITYPE_MEMORY,  false },  /* Split Compare and Limit */
    { "ANA",  040, 042,  ITYPE_CONTROL, false },  /* AND to Accumulator */
    { "LPR",  040, 070,  ITYPE_CONTROL, false },  /* Load Phase Register */
    { "EFC",  040, 062,  ITYPE_CONTROL, false },  /* Enable Fine Countdown */
    { "HFC",  040, 060,  ITYPE_CONTROL, false },  /* Halt Fine Countdown */
    { "RSD",  040, 020,  ITYPE_CONTROL, false },  /* Reset Detector */

    /* D37C Additional Control */
    { "TZE",  010, 0,    ITYPE_MEMORY,  true  },  /* Transfer on Zero */
    { "FCL",  014, 0,    ITYPE_MEMORY,  true  },  /* Full Compare and Limit */
    { "TSM",  040, 006,  ITYPE_CONTROL, true  },  /* Transfer Sector on Minus */
    { "TSZ",  040, 004,  ITYPE_CONTROL, true  },  /* Transfer Sector on Zero */
    { "GBP",  040, 064,  ITYPE_CONTROL, true  },  /* Generate Bit Pattern */
    { "GPT",  040, 060,  ITYPE_CONTROL, true  },  /* Generate Parity Bit */
    { "MAL",  040, 002,  ITYPE_CONTROL, true  },  /* Modify A and L */

    /* ========== I/O ========== */
    /* D17B I/O */
    { "DIA",  040, 052,  ITYPE_IO,      false },  /* Discrete Input A */
    { "DIB",  040, 050,  ITYPE_IO,      false },  /* Discrete Input B */
    { "DOA",  040, 026,  ITYPE_IO,      false },  /* Discrete Output A */
    { "VOA",  040, 030,  ITYPE_IO,      false },  /* Voltage Output A */
    { "VOB",  040, 032,  ITYPE_IO,      false },  /* Voltage Output B */
    { "VOC",  040, 034,  ITYPE_IO,      false },  /* Voltage Output C */
    { "BOA",  040, 010,  ITYPE_IO,      false },  /* Binary Output A */
    { "BOB",  040, 012,  ITYPE_IO,      false },  /* Binary Output B */
    { "BOC",  040, 002,  ITYPE_IO,      false },  /* Binary Output C */
    { "COA",  000, 040,  ITYPE_IO,      false },  /* Character Output A */

    /* D37C Additional I/O */
    { "DIC",  040, 020,  ITYPE_IO,      true  },  /* Discrete Input C */
    { "DOB",  040, 054,  ITYPE_IO,      true  },  /* Discrete Output B */
    { "VIA",  040, 010,  ITYPE_IO,      true  },  /* Voltage Input A */
    { "VIB",  040, 012,  ITYPE_IO,      true  },  /* Voltage Input B */
    { "VIC",  040, 014,  ITYPE_IO,      true  },  /* Voltage Input C */
    { "VID",  040, 016,  ITYPE_IO,      true  },  /* Voltage Input D */
    { "VIE",  040, 030,  ITYPE_IO,      true  },  /* Voltage Input E */
    { "VIF",  040, 032,  ITYPE_IO,      true  },  /* Voltage Input F */
    { "VIG",  040, 034,  ITYPE_IO,      true  },  /* Voltage Input G */
    { "VIH",  040, 036,  ITYPE_IO,      true  },  /* Voltage Input H */
    { "ECI",  040, 062,  ITYPE_IO,      true  },  /* Enable Cable Input */
    { "ECO",  040, 062,  ITYPE_IO,      true  },  /* Enable Cable Output */
    { "EPP",  040, 062,  ITYPE_IO,      true  },  /* Enable Platform Power */
    { "DPP",  040, 062,  ITYPE_IO,      true  },  /* Disable Platform Power */
    { "RIC",  000, 024,  ITYPE_IO,      true  },  /* Radio Intercommunication */
    { "SRD",  000, 016,  ITYPE_IO,      true  },  /* Simulate Transient */

    /* End marker */
    { NULL, 0, 0, 0, false }
};

/*
 * Find instruction by mnemonic
 */
const instr_t *find_instruction(const char *mnemonic) {
    for (int i = 0; instructions[i].mnemonic != NULL; i++) {
        if (str_eq_nocase(instructions[i].mnemonic, mnemonic)) {
            return &instructions[i];
        }
    }
    return NULL;
}
