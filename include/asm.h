/*
 * Minuteman D17B/D37C Assembler
 * Header definitions
 *
 * The D17B was designed when Kennedy was president and the
 * Beatles were playing Hamburg. It's still operational.
 * Make of that what you will.
 *
 * Copyright 2025 - Apache 2.0 License
 */

#ifndef D17B_ASM_H
#define D17B_ASM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*
 * Case-insensitive string comparison
 * We roll our own because Windows _stricmp has... issues.
 * The original D17B programmers didn't have case sensitivity either.
 * They also didn't have lowercase. Or a proper keyboard, really.
 */
static inline int str_eq_nocase(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a >= 'a' && *a <= 'z' ? *a - 32 : *a;
        char cb = *b >= 'a' && *b <= 'z' ? *b - 32 : *b;
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == *b;  /* Both must be null */
}

/* Word size */
#define WORD_BITS   24
#define WORD_MASK   0xFFFFFF

/* Memory limits */
#define MAX_CHANNELS    64
#define MAX_SECTORS     128
#define MAX_SYMBOLS     1024
#define MAX_LINE_LEN    256
#define MAX_ERRORS      100

/* Instruction types */
typedef enum {
    ITYPE_MEMORY,       /* Uses channel,sector addressing */
    ITYPE_LOOP,         /* Uses rapid-access loop */
    ITYPE_SHIFT,        /* Shift with count */
    ITYPE_CONTROL,      /* Control/misc with subcode */
    ITYPE_IO,           /* I/O with subcode */
    ITYPE_IMPLIED       /* No operands */
} instr_type_t;

/* Instruction table entry */
typedef struct {
    const char *mnemonic;
    uint8_t     opcode;         /* Primary opcode (octal in docs) */
    uint8_t     subcode;        /* Secondary code for 40-class instructions */
    instr_type_t type;
    bool        d37c_only;      /* Only available on D37C */
} instr_t;

/* Symbol (label) entry */
typedef struct {
    char        name[32];
    uint8_t     channel;
    uint8_t     sector;
    bool        defined;
} symbol_t;

/* Assembler state */
typedef struct {
    /* Current location */
    uint8_t     cur_channel;
    uint8_t     cur_sector;

    /* Output buffer */
    uint32_t    output[MAX_CHANNELS][MAX_SECTORS];
    bool        used[MAX_CHANNELS][MAX_SECTORS];

    /* Symbol table */
    symbol_t    symbols[MAX_SYMBOLS];
    int         num_symbols;

    /* Error tracking */
    char        errors[MAX_ERRORS][MAX_LINE_LEN];
    int         num_errors;
    int         current_line;

    /* Options */
    bool        d37c_mode;      /* Enable D37C extensions */
    bool        verbose;
} asm_state_t;

/* Rapid-access loop channels (octal values from docs) */
#define CHAN_U_LOOP     060     /* U loop - 1 word */
#define CHAN_F_LOOP     052     /* F loop - 4 words */
#define CHAN_E_LOOP     056     /* E loop - 8 words */
#define CHAN_H_LOOP     054     /* H loop - 16 words */
#define CHAN_L_REG      064     /* L register */
#define CHAN_V_LOOP     070     /* V loop - 4 words (D37C) */
#define CHAN_R_LOOP     072     /* R loop - 4 words (D37C) */

/* Token types */
typedef enum {
    TOK_LABEL,      /* LABEL: */
    TOK_MNEMONIC,   /* CLA, STO, etc */
    TOK_NUMBER,     /* Decimal or octal number */
    TOK_LOOP,       /* U, F, E, H, L, V, R */
    TOK_COMMA,      /* , */
    TOK_EOL,        /* End of line */
    TOK_ERROR       /* Lexer error */
} token_type_t;

typedef struct {
    token_type_t type;
    char text[64];
    int value;          /* For numbers */
    uint8_t loop_chan;  /* For loop tokens */
} token_t;

/* Function prototypes */
void asm_init(asm_state_t *state);
int asm_pass1(asm_state_t *state, const char *filename);
int asm_pass2(asm_state_t *state, const char *filename);
int asm_write_output(asm_state_t *state, const char *filename);

/* Instruction lookup */
const instr_t *find_instruction(const char *mnemonic);

/* Symbol table */
symbol_t *find_symbol(asm_state_t *state, const char *name);
symbol_t *add_symbol(asm_state_t *state, const char *name);

/* Lexer */
int tokenize_line(const char *line, token_t *tokens, int max_tokens);
int read_line(FILE *fp, char *buf, int max_len);

/* Error reporting */
void asm_error(asm_state_t *state, const char *fmt, ...);

#endif /* D17B_ASM_H */
