/*
 * Minuteman D17B/D37C Assembler
 * Symbol Table - keeps track of where things are
 *
 * In the original system, programmers had to remember where on the
 * spinning disc they'd put things. We're not barbarians, so we use labels.
 *
 * Copyright 2025 - Apache 2.0 License
 */

#include "asm.h"
#include <string.h>

/*
 * Find a symbol by name
 *
 * Returns NULL if not found, which is either an error or an opportunity
 * for personal growth, depending on your perspective.
 */
symbol_t *find_symbol(asm_state_t *state, const char *name) {
    for (int i = 0; i < state->num_symbols; i++) {
        if (str_eq_nocase(state->symbols[i].name, name)) {
            return &state->symbols[i];
        }
    }
    return NULL;
}

/*
 * Add a new symbol to the table
 *
 * If the symbol already exists, returns the existing one.
 * If the table is full, returns NULL and you should probably
 * reconsider your life choices vis-à-vis nuclear missile software.
 */
symbol_t *add_symbol(asm_state_t *state, const char *name) {
    /* Check if already exists */
    symbol_t *existing = find_symbol(state, name);
    if (existing != NULL) {
        return existing;
    }

    /* Check for space */
    if (state->num_symbols >= MAX_SYMBOLS) {
        /*
         * 1024 symbols ought to be enough for anybody.
         * The original programmers had to work with a few dozen.
         * They also had to think about disc latency. Count your blessings.
         */
        return NULL;
    }

    /* Add new symbol */
    symbol_t *sym = &state->symbols[state->num_symbols++];
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
    sym->channel = 0;
    sym->sector = 0;
    sym->defined = false;

    return sym;
}
