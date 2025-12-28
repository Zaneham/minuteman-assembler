/*
 * Minuteman D17B/D37C Assembler
 * Lexer - tokenizes assembly source lines
 *
 * Copyright 2025 - Apache 2.0 License
 */

#include "asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Skip whitespace
 */
static const char *skip_ws(const char *p) {
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return p;
}

/*
 * Parse a number (decimal or octal)
 * Octal numbers end with 'O' or 'o'
 * Numbers starting with 0 are NOT automatically octal (unlike C)
 */
static int parse_number(const char *str, int *value) {
    char buf[32];
    int i = 0;
    bool octal = false;

    while (str[i] && (isdigit(str[i]) || toupper(str[i]) == 'O')) {
        buf[i] = str[i];
        i++;
        if (i >= 31) return -1;
    }
    buf[i] = '\0';

    if (i == 0) return -1;

    /* Check for octal suffix */
    if (toupper(buf[i-1]) == 'O') {
        octal = true;
        buf[i-1] = '\0';
    }

    if (octal) {
        *value = (int)strtol(buf, NULL, 8);
    } else {
        *value = (int)strtol(buf, NULL, 10);
    }

    return i;
}

/*
 * Get loop channel for a loop designator
 */
static int get_loop_channel(char loop) {
    switch (toupper(loop)) {
        case 'U': return CHAN_U_LOOP;
        case 'F': return CHAN_F_LOOP;
        case 'E': return CHAN_E_LOOP;
        case 'H': return CHAN_H_LOOP;
        case 'L': return CHAN_L_REG;
        case 'V': return CHAN_V_LOOP;
        case 'R': return CHAN_R_LOOP;
        default:  return -1;
    }
}

/*
 * Check if a string is a loop designator
 */
static bool is_loop(const char *str) {
    if (strlen(str) != 1) return false;
    return get_loop_channel(str[0]) >= 0;
}

/*
 * Tokenize a single line
 * Returns array of tokens (caller should provide buffer)
 * Returns number of tokens, or -1 on error
 */
int tokenize_line(const char *line, token_t *tokens, int max_tokens) {
    const char *p = line;
    int num_tokens = 0;

    p = skip_ws(p);

    /* Empty line or comment */
    if (*p == '\0' || *p == ';' || *p == '\n' || *p == '\r') {
        tokens[0].type = TOK_EOL;
        return 0;
    }

    while (*p && *p != ';' && *p != '\n' && *p != '\r') {
        if (num_tokens >= max_tokens) return -1;

        p = skip_ws(p);
        if (*p == '\0' || *p == ';' || *p == '\n' || *p == '\r') break;

        token_t *tok = &tokens[num_tokens];

        /* Comma */
        if (*p == ',') {
            tok->type = TOK_COMMA;
            tok->text[0] = ',';
            tok->text[1] = '\0';
            p++;
            num_tokens++;
            continue;
        }

        /* Number */
        if (isdigit(*p)) {
            int len = parse_number(p, &tok->value);
            if (len < 0) {
                tok->type = TOK_ERROR;
                return -1;
            }
            tok->type = TOK_NUMBER;
            strncpy(tok->text, p, len);
            tok->text[len] = '\0';
            p += len;
            num_tokens++;
            continue;
        }

        /* Identifier (label, mnemonic, or loop) */
        if (isalpha(*p) || *p == '_') {
            int i = 0;
            while (isalnum(p[i]) || p[i] == '_') {
                if (i >= 63) break;
                tok->text[i] = p[i];
                i++;
            }
            tok->text[i] = '\0';
            p += i;

            /* Check for label (ends with colon) */
            if (*p == ':') {
                tok->type = TOK_LABEL;
                p++;
            } else if (is_loop(tok->text)) {
                tok->type = TOK_LOOP;
                tok->loop_chan = get_loop_channel(tok->text[0]);
            } else {
                tok->type = TOK_MNEMONIC;
            }
            num_tokens++;
            continue;
        }

        /* Unknown character */
        tok->type = TOK_ERROR;
        snprintf(tok->text, sizeof(tok->text), "Unexpected character '%c'", *p);
        return -1;
    }

    return num_tokens;
}

/*
 * Read a line from file, stripping newline
 */
int read_line(FILE *fp, char *buf, int max_len) {
    if (fgets(buf, max_len, fp) == NULL) {
        return -1;
    }

    /* Strip trailing newline/carriage return */
    int len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }

    return len;
}
