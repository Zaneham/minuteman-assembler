# Minuteman Guidance Computer Assembler

An assembler for the D17B/D37C guidance computers. Because apparently the emulator wasn't enough and now we're writing toolchains for nuclear missiles.

## What Is This?

This is a two-pass assembler for the Autonetics D17B and D37C guidance computers. It takes assembly source files and produces binary output in the original tape format.

The D17B and D37C are the guidance computers for the LGM-30 Minuteman intercontinental ballistic missile. They've been keeping 400 nuclear weapons pointed at things since 1962. The missiles are still operational. The computers are still the same architecture. We are building development tools for them in 2025.

This is either historically important preservation work or we've completely lost the plot. Possibly both.

## Who's This For?

**Hobbyists** — You enjoy retrocomputing. You've done 6502, Z80, maybe some PDP-11. You thought "what's the most obscure thing I could possibly program?" and somehow ended up here. Welcome. We don't judge.

**Academics** — You're researching historical computing, cold war technology, or the remarkable longevity of safety-critical systems. The D17B predates structured programming, high-level languages in embedded systems, and the general consensus that maybe nuclear weapons shouldn't run on computers with rotating disc memory. There's a thesis in there somewhere.

**ICBM Maintainers** — So you're somewhere between middle-of-nowhere Montana and shit-I'm-lost Wyoming? And you're chucking hex dumps into a 60 year old computer? That's rough, dude. We don't know if this will actually help, but here's an assembler anyway. Good luck out there.

**The Morbidly Curious** — You just wanted to see what 1960s missile code looked like. Fair enough. Have a look at the examples folder.

## Features

- **Full D17B instruction set** (39 instructions) — Minuteman I, 1962
- **Full D37C instruction set** (57 instructions) — Minuteman II/III, 1965-present
- **Two-pass assembly** — Labels can be used before they're defined, like civilised people
- **Rapid-access loop addressing** — U, F, E, H, L loops (and V, R for D37C)
- **Channel/sector direct addressing** — Because memory is on a spinning disc
- **Case-insensitive** — The original programmers didn't have lowercase anyway
- **Binary output** — In the original tape format, ready for your emulator (or missile)

## Usage

```bash
# Assemble a program
./d17b-asm program.asm -o program.bin

# D37C mode (enables additional instructions)
./d17b-asm -37 program.asm -o program.bin

# Verbose output
./d17b-asm -v program.asm -o program.bin

# Pipe to emulator (when it supports stdin)
./d17b-asm program.asm -o - | d17b -r -
```

## Syntax

```asm
; Comments start with semicolon
; Labels end with colon

        ORG     0,0             ; Set location to channel 0, sector 0

START:  CLA     DATA            ; Clear accumulator and Add from DATA
        ADD     F,2             ; Add from F-loop, word 2
        MPY     H,15            ; Multiply from H-loop, word 15
        STO     U               ; Store to U-loop (fast memory!)
        TMI     NEGATIVE        ; Transfer on Minus
        TRA     START           ; Unconditional transfer (loop forever)

NEGATIVE:
        COM                     ; Complement accumulator
        TRA     START           ; Back to start

DATA:   OCT     000123          ; Octal constant
RESULT: BSS     1               ; Reserve one word

        END
```

### Memory Addressing

The D17B uses a rotating magnetic disc for memory. Addresses are specified as channel and sector:

| Syntax | Meaning |
|--------|---------|
| `52,17` | Channel 52 (octal), Sector 17 (octal) |
| `U` | U-loop — 1 word of rapid-access memory |
| `F,n` | F-loop word n (0-3) — 4 words |
| `E,n` | E-loop word n (0-7) — 8 words |
| `H,n` | H-loop word n (0-15) — 16 words |
| `L` | L register |
| `LABEL` | Whatever address the label points to |

The rapid-access loops are small amounts of memory that don't require waiting for the disc to rotate. They're fast. Everything else requires you to wait. Programming for the D17B means thinking about where on the disc your data physically is.

This is called "minimum latency coding" and it's exactly as fun as it sounds.

### Directives

| Directive | Description |
|-----------|-------------|
| `ORG c,s` | Set location to channel c, sector s |
| `OCT n` | Emit octal constant |
| `DATA n` | Emit constant (same as OCT) |
| `BSS n` | Reserve n words of space |
| `END` | End of source file |

### Supported Instructions

**D17B Mode (39 instructions):**
- **Arithmetic:** CLA, STO, ADD, SAD, SUB, SSU, MPY, SMP, MPM, SMM, COM, MIM
- **Shift:** ALS, ARS, SAL, SAR, SLL, SLR, SRL, SRR
- **Control:** TRA, TMI, HPR, SCL, ANA, LPR, EFC, HFC, RSD
- **I/O:** DIA, DIB, DOA, VOA, VOB, VOC, BOA, BOB, BOC, COA

**D37C Mode (57 instructions):**
- Everything above, plus:
- **Arithmetic:** DIV (hardware division!), ORA, AWC, PMI, SPM
- **Shift:** ALC, ARC
- **Control:** TZE, FCL, TSM, TSZ, GBP, GPT, MAL
- **I/O:** DIC, DOB, VIA-VIH, ECI, ECO, EPP, DPP, RIC, SRD

The D37C added hardware division. The D17B had to do it in software. Imagine explaining to the missile that it needs to wait while you divide.

## Building

```bash
make
```

If that doesn't work, you'll need a C compiler. We recommend anything from after 1970. The irony of building this with modern tools is not lost on us.

## Output Format

The assembler outputs binary in the original tape format:

- 8-byte header: magic ("D17" or "D37"), version, word count, start address
- 24-bit words packed as 3 bytes each, big-endian

Whether you then punch this onto paper tape and feed it into a ground support computer is entirely your business.

## Accuracy

This assembler was built from documentation in the public domain:

- **D17B Computer Programming Manual** (Sep 1971)
- **AFIT Thesis GE/EE/74-16** (Mar 1974)

The instruction encoding is believed to be accurate. It has not been tested against actual missile hardware, because we don't have any, and if we did, we probably shouldn't be programming it.

## Related Projects

If writing assembly for nuclear missile guidance computers has left you wanting more questionable life choices, might we suggest:

- **[minuteman-emu](https://github.com/Zaneham/minuteman-emu)** — The emulator this assembler targets. Because what good is an assembler if you can't run the code? Well, you *could* run it on an actual missile, but we'd rather you didn't.

- **[JOVIAL J73 LSP](https://github.com/Zaneham/jovial-lsp)** — Language server for the programming language that flies F-15s, B-52s, and AWACS. Same Cold War energy, slightly less existential dread.

- **[CMS-2 LSP](https://github.com/Zaneham/cms2-lsp)** — Language server for the US Navy's tactical language. Powers Aegis cruisers, which exist partly to shoot down things the Minuteman might be sending. It's a whole ecosystem, really.

- **[CORAL 66 LSP](https://github.com/Zaneham/coral66-lsp)** — The British Ministry of Defence's real-time language. Developed at the Royal Radar Establishment, Malvern, presumably between tea breaks. Features Crown Copyright and a stiff upper lip.

- **[HAL/S LSP](https://github.com/Zaneham/hals-lsp)** — NASA's Space Shuttle language. For when you'd rather go to space than through it at Mach 23.

## Licence

Copyright 2025 Zane Hambly

Apache 2.0 — See [LICENSE](LICENSE) for details.

---

*"Writing an assembler for nuclear missile guidance computers is a completely normal hobby."*
— Nobody, ever
