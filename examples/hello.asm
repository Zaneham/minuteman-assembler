; hello.asm - A simple test program for the D17B/D37C assembler
;
; This program does absolutely nothing useful, which is frankly
; preferable when dealing with nuclear missile guidance computers.
;
; It demonstrates:
; - Labels
; - Memory instructions
; - Rapid-access loop operations
; - Control flow
; - The general syntax of D17B assembly
;

        ORG     0,0             ; Start at channel 0, sector 0

; ========== MAIN PROGRAM ==========
START:
        CLA     DATA1           ; Clear accumulator, add DATA1
        ADD     DATA2           ; Add DATA2 to accumulator
        STO     RESULT          ; Store result

        CLA     U               ; Load from U-loop (fast!)
        ADD     F,0             ; Add F-loop word 0
        STO     H,15            ; Store to H-loop word 15

        TMI     NEGATIVE        ; If negative, branch
        TRA     DONE            ; Otherwise, we're done

NEGATIVE:
        COM                     ; Complement the accumulator
        TRA     DONE            ; Carry on

DONE:
        HPR                     ; Halt and proceed
                                ; (Waits for operator intervention)
        TRA     START           ; Loop forever
                                ; The missile would appreciate this

; ========== DATA SECTION ==========
        ORG     1,0             ; Put data in channel 1

DATA1:  OCT     000123          ; Some data (octal)
DATA2:  OCT     000456          ; More data
RESULT: BSS     1               ; Reserve one word for result

        END
