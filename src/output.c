/*
 * Minuteman D17B/D37C Assembler
 * Output - writes binary in original tape format
 *
 * The original format was punched paper tape fed into a tape reader
 * connected to a ground support computer which then programmed the
 * missile's disc memory. We're skipping a few steps.
 *
 * Output format: 24-bit words packed as 3 bytes, big-endian.
 * This matches how the words would have been serialised to tape.
 * Whether you then punch it onto paper tape is your business.
 *
 * Copyright 2025 - Apache 2.0 License
 */

#include "asm.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

/*
 * Write a 24-bit word as 3 bytes, big-endian
 *
 * The D17B used big-endian because it was 1962 and nobody had
 * yet agreed that little-endian was obviously correct. Or wrong.
 * The debate continues.
 */
static int write_word(FILE *fp, uint32_t word) {
    uint8_t bytes[3];
    bytes[0] = (word >> 16) & 0xFF;  /* High byte first, as God intended */
    bytes[1] = (word >> 8) & 0xFF;   /* Middle byte, doing its best */
    bytes[2] = word & 0xFF;          /* Low byte, bringing up the rear */

    if (fwrite(bytes, 1, 3, fp) != 3) {
        return -1;
    }
    return 0;
}

/*
 * Write assembled output to file
 *
 * Writes all used memory locations in channel/sector order.
 * Empty channels are skipped because even nuclear missiles
 * shouldn't waste storage.
 *
 * Use "-" as filename to write to stdout, which is useful for
 * piping directly to an emulator. Or a missile. We don't judge.
 */
int asm_write_output(asm_state_t *state, const char *filename) {
    FILE *fp;
    bool use_stdout = (strcmp(filename, "-") == 0);

    if (use_stdout) {
        fp = stdout;
#ifdef _WIN32
        /*
         * Windows insists on "helping" with text mode.
         * We must firmly decline this assistance.
         */
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    } else {
        fp = fopen(filename, "wb");
        if (fp == NULL) {
            return -1;
        }
    }

    /*
     * Write header: magic number and metadata
     *
     * Format:
     *   Bytes 0-2:  Magic "D17" (or "D37" for D37C mode)
     *   Byte 3:     Version (1)
     *   Bytes 4-5:  Number of words (big-endian)
     *   Bytes 6-7:  Starting channel,sector (big-endian)
     *
     * The original tapes didn't have headers. They just started
     * with data and you had to know what you were doing. We are
     * marginally more civilised.
     */

    /* Count words and find start */
    int word_count = 0;
    int start_chan = -1, start_sect = -1;

    for (int c = 0; c < MAX_CHANNELS; c++) {
        for (int s = 0; s < MAX_SECTORS; s++) {
            if (state->used[c][s]) {
                if (start_chan < 0) {
                    start_chan = c;
                    start_sect = s;
                }
                word_count++;
            }
        }
    }

    if (word_count == 0) {
        /*
         * An empty program. Perhaps the user is making a statement
         * about the futility of nuclear deterrence. Or they forgot
         * to write any code. Either way, not our problem.
         */
        if (!use_stdout) fclose(fp);
        return 0;
    }

    /* Write header */
    fprintf(fp, state->d37c_mode ? "D37" : "D17");
    fputc(1, fp);  /* Version */
    fputc((word_count >> 8) & 0xFF, fp);
    fputc(word_count & 0xFF, fp);
    fputc(start_chan & 0xFF, fp);
    fputc(start_sect & 0xFF, fp);

    /* Write all used words in order */
    for (int c = 0; c < MAX_CHANNELS; c++) {
        for (int s = 0; s < MAX_SECTORS; s++) {
            if (state->used[c][s]) {
                if (write_word(fp, state->output[c][s]) != 0) {
                    if (!use_stdout) fclose(fp);
                    return -1;
                }
            }
        }
    }

    if (!use_stdout) {
        fclose(fp);
    }

    return 0;
}
