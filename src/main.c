/*
 * Minuteman D17B/D37C Assembler
 * Main entry point
 *
 * Copyright 2025 - Apache 2.0 License
 */

#include "asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void print_usage(const char *prog) {
    fprintf(stderr, "Minuteman D17B/D37C Assembler\n\n");
    fprintf(stderr, "Usage: %s [options] input.asm\n\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o FILE    Output file (default: a.out, use - for stdout)\n");
    fprintf(stderr, "  -37        Enable D37C mode (default: D17B)\n");
    fprintf(stderr, "  -v         Verbose output\n");
    fprintf(stderr, "  -h         Show this help\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s program.asm -o program.bin\n", prog);
    fprintf(stderr, "  %s -37 program.asm -o - | d17b -r -\n", prog);
}

/*
 * Error reporting
 */
void asm_error(asm_state_t *state, const char *fmt, ...) {
    if (state->num_errors >= MAX_ERRORS) return;

    va_list args;
    va_start(args, fmt);

    char *buf = state->errors[state->num_errors];
    int len = snprintf(buf, MAX_LINE_LEN, "Line %d: ", state->current_line);
    vsnprintf(buf + len, MAX_LINE_LEN - len, fmt, args);

    va_end(args);
    state->num_errors++;
}

/*
 * Initialize assembler state
 */
void asm_init(asm_state_t *state) {
    memset(state, 0, sizeof(asm_state_t));
    state->cur_channel = 0;
    state->cur_sector = 0;
}

/*
 * Print all accumulated errors
 */
static void print_errors(asm_state_t *state) {
    for (int i = 0; i < state->num_errors; i++) {
        fprintf(stderr, "Error: %s\n", state->errors[i]);
    }
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = "a.out";
    asm_state_t state;

    asm_init(&state);

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-37") == 0) {
            state.d37c_mode = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            state.verbose = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            if (input_file != NULL) {
                fprintf(stderr, "Error: Multiple input files not supported\n");
                return 1;
            }
            input_file = argv[i];
        }
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    if (state.verbose) {
        printf("Assembling: %s\n", input_file);
        printf("Mode: %s\n", state.d37c_mode ? "D37C" : "D17B");
        printf("Output: %s\n", output_file);
    }

    /* Pass 1: Collect labels */
    if (state.verbose) {
        printf("Pass 1: Collecting labels...\n");
    }

    if (asm_pass1(&state, input_file) != 0) {
        print_errors(&state);
        return 1;
    }

    if (state.verbose) {
        printf("  Found %d symbols\n", state.num_symbols);
    }

    /* Pass 2: Generate code */
    if (state.verbose) {
        printf("Pass 2: Generating code...\n");
    }

    if (asm_pass2(&state, input_file) != 0) {
        print_errors(&state);
        return 1;
    }

    /* Check for errors */
    if (state.num_errors > 0) {
        print_errors(&state);
        fprintf(stderr, "Assembly failed with %d error(s)\n", state.num_errors);
        return 1;
    }

    /* Write output */
    if (asm_write_output(&state, output_file) != 0) {
        fprintf(stderr, "Error: Failed to write output file\n");
        return 1;
    }

    if (state.verbose) {
        /* Count words used */
        int words = 0;
        for (int c = 0; c < MAX_CHANNELS; c++) {
            for (int s = 0; s < MAX_SECTORS; s++) {
                if (state.used[c][s]) words++;
            }
        }
        printf("Assembly complete: %d words\n", words);
    }

    return 0;
}
