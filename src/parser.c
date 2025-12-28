/*
 * Minuteman D17B/D37C Assembler
 * Parser - two-pass assembly
 *
 * Copyright 2025 - Apache 2.0 License
 */

#include "asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 16

/*
 * Encode a memory instruction
 * Format: [opcode:6][channel:6][sector:12]
 */
static uint32_t encode_memory(uint8_t opcode, uint8_t channel, uint16_t sector) {
    uint32_t word = 0;
    word |= ((uint32_t)opcode & 077) << 18;     /* Bits 23-18: opcode */
    word |= ((uint32_t)channel & 077) << 12;    /* Bits 17-12: channel */
    word |= ((uint32_t)sector & 07777);         /* Bits 11-0: sector */
    return word & WORD_MASK;
}

/*
 * Encode a shift instruction
 * Format: [000:6][subcode:6][count:12]
 */
static uint32_t encode_shift(uint8_t subcode, uint16_t count) {
    uint32_t word = 0;
    word |= ((uint32_t)subcode & 077) << 12;    /* Bits 17-12: subcode */
    word |= ((uint32_t)count & 07777);          /* Bits 11-0: shift count */
    return word & WORD_MASK;
}

/*
 * Encode a control/IO instruction (40-class)
 * Format: [040:6][subcode:6][operand:12]
 */
static uint32_t encode_control(uint8_t subcode, uint16_t operand) {
    uint32_t word = 0;
    word |= ((uint32_t)040 & 077) << 18;        /* Bits 23-18: opcode 040 */
    word |= ((uint32_t)subcode & 077) << 12;    /* Bits 17-12: subcode */
    word |= ((uint32_t)operand & 07777);        /* Bits 11-0: operand */
    return word & WORD_MASK;
}

/*
 * Advance location counter
 */
static void advance_location(asm_state_t *state) {
    state->cur_sector++;
    if (state->cur_sector >= MAX_SECTORS) {
        state->cur_sector = 0;
        state->cur_channel++;
        if (state->cur_channel >= MAX_CHANNELS) {
            asm_error(state, "Memory overflow");
        }
    }
}

/*
 * Parse an operand for memory instructions
 * Returns: 0 on success, -1 on error
 * Sets channel and sector in output params
 */
static int parse_memory_operand(asm_state_t *state, token_t *tokens, int num_tokens,
                                 int start_idx, uint8_t *channel, uint16_t *sector,
                                 bool pass2) {
    if (start_idx >= num_tokens) {
        if (pass2) asm_error(state, "Missing operand");
        return -1;
    }

    token_t *tok = &tokens[start_idx];

    /* Loop reference: U, F,n, E,n, H,n, L */
    if (tok->type == TOK_LOOP) {
        *channel = tok->loop_chan;

        /* Check for index (F,n etc) */
        if (start_idx + 2 < num_tokens &&
            tokens[start_idx + 1].type == TOK_COMMA &&
            tokens[start_idx + 2].type == TOK_NUMBER) {
            *sector = tokens[start_idx + 2].value;
        } else {
            *sector = 0;  /* Default to sector 0 (U, L) */
        }
        return 0;
    }

    /* Symbol reference */
    if (tok->type == TOK_MNEMONIC) {
        symbol_t *sym = find_symbol(state, tok->text);
        if (sym == NULL) {
            if (pass2) {
                asm_error(state, "Undefined symbol '%s'", tok->text);
            }
            return -1;
        }
        *channel = sym->channel;
        *sector = sym->sector;
        return 0;
    }

    /* Direct address: channel,sector */
    if (tok->type == TOK_NUMBER) {
        *channel = tok->value;

        if (start_idx + 2 < num_tokens &&
            tokens[start_idx + 1].type == TOK_COMMA &&
            tokens[start_idx + 2].type == TOK_NUMBER) {
            *sector = tokens[start_idx + 2].value;
        } else {
            if (pass2) asm_error(state, "Expected channel,sector");
            return -1;
        }
        return 0;
    }

    if (pass2) asm_error(state, "Invalid operand");
    return -1;
}

/*
 * Process a single line
 * Returns: 0 on success, -1 on error
 */
static int process_line(asm_state_t *state, const char *line, bool pass2) {
    token_t tokens[MAX_TOKENS];
    int num_tokens = tokenize_line(line, tokens, MAX_TOKENS);

    if (num_tokens < 0) {
        asm_error(state, "Tokenization error");
        return -1;
    }

    if (num_tokens == 0) {
        return 0;  /* Empty line or comment */
    }

    int idx = 0;

    /* Handle label */
    if (tokens[0].type == TOK_LABEL) {
        if (!pass2) {
            /* Pass 1: Record label */
            symbol_t *sym = add_symbol(state, tokens[0].text);
            if (sym == NULL) {
                asm_error(state, "Symbol table full");
                return -1;
            }
            sym->channel = state->cur_channel;
            sym->sector = state->cur_sector;
            sym->defined = true;
        }
        idx++;
    }

    /* Check for mnemonic */
    if (idx >= num_tokens) {
        return 0;  /* Label only line */
    }

    if (tokens[idx].type != TOK_MNEMONIC) {
        asm_error(state, "Expected instruction mnemonic");
        return -1;
    }

    /* Handle directives */
    if (str_eq_nocase(tokens[idx].text, "ORG")) {
        /* ORG channel,sector */
        if (idx + 3 < num_tokens &&
            tokens[idx + 1].type == TOK_NUMBER &&
            tokens[idx + 2].type == TOK_COMMA &&
            tokens[idx + 3].type == TOK_NUMBER) {
            state->cur_channel = tokens[idx + 1].value;
            state->cur_sector = tokens[idx + 3].value;
        } else {
            asm_error(state, "ORG requires channel,sector");
            return -1;
        }
        return 0;
    }

    if (str_eq_nocase(tokens[idx].text, "DATA") ||
        str_eq_nocase(tokens[idx].text, "OCT")) {
        /* DATA/OCT value */
        if (idx + 1 < num_tokens && tokens[idx + 1].type == TOK_NUMBER) {
            if (pass2) {
                state->output[state->cur_channel][state->cur_sector] =
                    tokens[idx + 1].value & WORD_MASK;
                state->used[state->cur_channel][state->cur_sector] = true;
            }
            advance_location(state);
        } else {
            asm_error(state, "DATA/OCT requires a value");
            return -1;
        }
        return 0;
    }

    if (str_eq_nocase(tokens[idx].text, "BSS")) {
        /* BSS count - reserve space */
        if (idx + 1 < num_tokens && tokens[idx + 1].type == TOK_NUMBER) {
            for (int i = 0; i < tokens[idx + 1].value; i++) {
                advance_location(state);
            }
        } else {
            asm_error(state, "BSS requires a count");
            return -1;
        }
        return 0;
    }

    if (str_eq_nocase(tokens[idx].text, "END")) {
        return 0;  /* End of assembly */
    }

    /* Look up instruction - mode-aware for TMI and other dual-encoding instructions */
    const instr_t *instr = find_instruction(tokens[idx].text, state->d37c_mode);
    if (instr == NULL) {
        asm_error(state, "Unknown instruction '%s'%s", tokens[idx].text,
                  state->d37c_mode ? "" : " (D37C-only instruction?)");
        return -1;
    }

    uint32_t word = 0;

    switch (instr->type) {
        case ITYPE_MEMORY: {
            uint8_t channel = 0;
            uint16_t sector = 0;
            if (parse_memory_operand(state, tokens, num_tokens, idx + 1,
                                     &channel, &sector, pass2) == 0 || !pass2) {
                word = encode_memory(instr->opcode, channel, sector);
            }
            break;
        }

        case ITYPE_SHIFT: {
            uint16_t count = 1;
            if (idx + 1 < num_tokens && tokens[idx + 1].type == TOK_NUMBER) {
                count = tokens[idx + 1].value;
            }
            word = encode_shift(instr->subcode, count);
            break;
        }

        case ITYPE_CONTROL:
        case ITYPE_IO: {
            uint16_t operand = 0;
            if (idx + 1 < num_tokens && tokens[idx + 1].type == TOK_NUMBER) {
                operand = tokens[idx + 1].value;
            }
            word = encode_control(instr->subcode, operand);
            break;
        }

        case ITYPE_IMPLIED: {
            word = encode_control(instr->subcode, 0);
            break;
        }

        case ITYPE_LOOP:
            /* Should be handled as memory with loop addressing */
            break;
    }

    if (pass2) {
        state->output[state->cur_channel][state->cur_sector] = word;
        state->used[state->cur_channel][state->cur_sector] = true;
    }

    advance_location(state);
    return 0;
}

/*
 * Pass 1: Collect labels and calculate addresses
 */
int asm_pass1(asm_state_t *state, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    char line[MAX_LINE_LEN];
    state->current_line = 0;
    state->cur_channel = 0;
    state->cur_sector = 0;

    while (read_line(fp, line, MAX_LINE_LEN) >= 0) {
        state->current_line++;
        process_line(state, line, false);
    }

    fclose(fp);
    return state->num_errors > 0 ? -1 : 0;
}

/*
 * Pass 2: Generate code
 */
int asm_pass2(asm_state_t *state, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    char line[MAX_LINE_LEN];
    state->current_line = 0;
    state->cur_channel = 0;
    state->cur_sector = 0;

    while (read_line(fp, line, MAX_LINE_LEN) >= 0) {
        state->current_line++;
        if (process_line(state, line, true) != 0) {
            /* Continue to find more errors */
        }
    }

    fclose(fp);
    return state->num_errors > 0 ? -1 : 0;
}
