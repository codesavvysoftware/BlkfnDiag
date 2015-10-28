
;*************************************************************************
;*
;* FILE:                ApexDiagRAMassy.s
;*
;* FULL DESC:           This file contains functions that perform Apex RAM
;*                      diagnostics during runtime.  The are written in
;*                      assembly to force variables into registers (rather
;*                      than RAM), and to increase performance.
;*
;*                      Functions included in this file are:
;*
;*                      dgnRAMError                     -Calls HW_HandleException if error observed
;*                      ApexDiagRamDataRunTestAsm       -RAM data pattern tests
;*                      ApexDiagRamAddressRunTestAsm    -RAM address line independence tests
;*
;* EDIT HISTORY:
;* [0] WBC      07-APR-2005 Initial version.
;* [1] SP       30-AUG-2013 RAM data pattern test modified to check shadow area; ECC error's
;*                          scrubbing added.
;* [2] PG       14-OCT-2013 Fault injection points added.
;* - mgrad      28-NOV-2013 File name changed during MISRA fixes.
;* - pszramo    13-DEC-2013 Removed code used if HOST_AZUSA was defined.
;*                          Renamed some fault injection points.
;* - mstasia    16-DEC-2013 Removed External RAM support, algorithm corrections
;*
;* @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
;*
;*************************************************************************

                INCLUDE ApexParameters.i
                IMPORT  HW_HandleException
                IMPORT  HI_DisableIrq
                IMPORT  HI_EnableIrq
                IMPORT  HI_LockCodeSpace            ; Needed for scrubbing code space
                IMPORT  HI_UnlockCodeSpace          ; Needed for scrubbing code space
                IMPORT  DgnSafeRam

                ;Start Fault Injection Point 7
                ;Allow fault injection flag
                ;IMPORT  InjectFaultFlag
                ;End Fault Injection Point 7

                ;Cache Enable/Disable operations
                IMPORT DisableDataCache
                IMPORT FlushDataCache
                IMPORT EnableDataCache

                EXPORT  ApexDiagRamDataRunTestAsm
                EXPORT  ApexDiagRamAddressRunTestAsm
                EXPORT  dgn_GloIntBusLockRegAddr


        AREA DGN_RAMTEST, CODE, READONLY

;*********************Constants that must be stored in code space ***************


dgn_GloIntBusLockRegAddr
         DCD     DGN_ADDR_INT_BUS_LOCK  ;Global int/bus lock register address
                                        ;(Stored here so that it can
                                        ;be retrieved in one load
                                        ;instruction and is not located
                                        ;in RAM).

dgn_SramEccError
         DCD     DGN_SRAM_ECC_ERROR     ;SramEccError register address
                                        ;(Stored here so that it can
                                        ;be retrieved in one load
                                        ;instruction and is not located
                                        ;in RAM).

dgn_RAMFinalPattern
        DCD     0x966c699c              ; Data pattern

dgn_IntDataRAMBegin
        DCD     DGN_INT_DATA_RAM_START  ;Internal data RAM start address

dgn_AddrPattern
        DCD     0xaa5555aa              ;RAM address independence test pattern

dgn_IntDataRAMSiz \
        EQU     (0*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal DgnSafeRam.intDataramSize)
dgn_IntShrRAMSiz \
        EQU     (2*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal DgnSafeRam.intShramSize)
dgn_ExtShrRAMSiz \
        EQU     (4*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal DgnSafeRam.extShramSize)
dgn_DataLoc \
        EQU     (6*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal DgnSafeRam.curDataTestAddr)
dgn_DataPrimeLoc \
        EQU     (7*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal DgnSafeRam.curDataTestAddrPrime)
dgn_Addr1Loc \
        EQU     (8*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal .curAddrTestAddr1)
dgn_Addr1PrimeLoc \
        EQU     (9*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal .curAddrTestAddr1Prime)
dgn_Addr2Loc \
        EQU     (10*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal .curAddrTestAddr2)
dgn_Addr2PrimeLoc \
        EQU     (11*4)                   ;Offset from start of DgnSafeRam
                                        ;(must equal .curAddrTestAddr2Prime)
dgn_AddrIterationsLoc \
        EQU     (14*4)                  ;Offset from start of DgnSafeRam
                                        ;(must equal .addrIterations)
dgn_AddrIterationsPrimeLoc \
        EQU     (15*4)                  ;Offset from start of DgnSafeRam
                                        ;(must equal .addrIterationsPrime)



;***********************************************************************
;*
;* FUNCTION NAME:       dgnRAMError
;*
;* FULL DESC:           This function handles the calling of HW_HandleException
;*                      if an error is observed.  Since the HW_HandleException routine
;*                      cannot display file and line numbers for assembly
;*                      files, we fake it out by feeding it with a pointer
;*                      to a text string that contains the file name (in a1),
;*                      and a 'line number' (in a2).  We'll put a different
;*                      'line number' in for each type of error to allow them
;*                      to be differentiated from each other.
;*
;* EDIT HISTORY:
;* [0] 07-APR-2005 WBC  Initial version
;*
;*      Copyright (c) 2005 Rockwell Automation Technologies, Inc. All rights reserved.
;*
;***********************************************************************/

dgnRAMError
                                        ;RAM Error handler
        LDR     a1, =dgnramtstFile
        B       HW_HandleException

;***********************************************************************
;*
;* FUNCTION NAME:       dgn_ScrubEccError
;*
;* FULL DESC:           This function handles scrubbing ECC errors that might
;*                      by detected by ECC circuit.
;*
;*                      Sequence of operation:
;*                      - read SRAM ECC error register
;*                      - check PER bit for signalled ECC errors
;*                      - if no error signalled we're done - exit
;*                      - if error found, unlock code space to allow it to be scrubed
;*                      - calculate error address by adding offset to starting memory address
;*                      - read and write value to scrub it
;*                      - write SRAM ECC error register to wipe out previous error
;*                      - read error location again
;*                      - if there are no errors we're done
;*                      - if there was an ECC error and at the same address we have a permanent
;*                        error - fault with line "50"
;*                      - otherwise repeat steps from scrubbing with new address
;*
;* EDIT HISTORY:
;* [0] 19-AUG-2013 SP  Initial version
;*
;*      Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;*
;*      Registers used:         r0 (a1) = SRAM ECC register address
;*                              r1 (a2) = SRAM ECC register content / ECC error address
;*                              r2 (a3) = ECC error address
;*                              r3 (a4) = Scratch
;*                              r4 (v1) = not used
;*                              r5 (v2) = not used
;*                              r6 (v3) = not used
;*                              r7 (v4) = not used
;*                              r8 (v5) = not used
;*                              r9 (v6) = not used
;*                              r10 (v7) = not used
;*
;***********************************************************************/
dgnScrubEccError PROC

        STMFD   sp!, {r0, r1, r2, r3, lr}
                                                  ; Check if ECC error occurred
        LDR     r0, dgn_SramEccError              ; Get ECC error register
        LDR     r1, [r0]                          ; Get register content

        ;Start Fault Injection Point 5
        ;MOV     r1, #0
        ;ORR     r1, r1, #DGN_ECC_ERR_MASK_SINGLE
        ;If above lines of code are uncommented then APEX will fault during data scrubbing
        ;End Fault Injection Point 5

        TST     r1, #DGN_ECC_ERR_MASK_SINGLE      ; Check per bit (which signals ECC error occurred)
        BEQ     dgn_ScrubEccErrorExit             ; No error, we're done

        STMFD   sp!, {r0, r1}                     ; Prevent r0 and r1 from being clobbered
        BLX     HI_UnlockCodeSpace                ; Unlock code space for scrubbing
        LDMIA   sp!, {r0, r1}                     ; Restore r0 and r1

        AND     r2, r1, #DGN_ECC_ERR_MASK_OFFSET  ; Get offset
        ADD     r2, r2, #DGN_RAM_SPACE_START      ; Add to start memory address to get actual address

dgn_Scrub
        LDR     r3, [r2]                          ; Read...
        STR     r3, [r2]                          ; ... and write value to scrub it
                                                  ; We don't care about write buffer being enabled at
                                                  ; this point - data read will cause write buffer to
                                                  ; be flushed.

        MOV     r3, #0                            ; Prepare value to reset ECC register
        STR     r3, [r0]                          ; Write ECC register to clean the error
        LDR     r3, [r2]                          ; Read scrubbed value to check it was corrected
                                                  ;
        LDR     r1, [r0]                          ; Get ECC register content

        ;Start Fault Injection Point 6
        ;MOV     r1, #0
        ;ORR     r1, r1, #DGN_ECC_ERR_MASK_SINGLE
        ;If above lines of code are uncommented then APEX will fault during data scrubbing
        ;End Fault Injection Point 6

        TST     r1, #DGN_ECC_ERR_MASK_SINGLE      ; Check per bit
        BEQ     dgn_ScrubbingDone                 ; Error corrected, we're done

        AND     r1, r1, #DGN_ECC_ERR_MASK_OFFSET  ; Get offset
        ADD     r1, r1, #DGN_RAM_SPACE_START      ; Add to start memory address to get actual address
        CMP     r1, r2                            ; Check if error occurred at the same place
        MOVNE   r2, r1                            ; If not save new error address
        BNE     dgn_Scrub                         ; And scrub again

        MOV     a2, #50                           ; Scrubbing did not work, we have a permanent error, "LINE 50"
        B       dgnRAMError

dgn_ScrubbingDone
                                                  ; At this point we don't really care about data in rX registers,
                                                  ;     so no saving before the call
        BLX     HI_LockCodeSpace                  ; Lock back the code space

dgn_ScrubEccErrorExit

        LDMIA   sp!, {r0, r1, r2, r3, lr}
        BX      lr

        ENDP

;***********************************************************************
;*
;* FUNCTION NAME:       dgnCheckShadowArea
;*
;* FULL DESC:           This function checks validates shadow area.
;*
;*                      Sequence of operation:
;*                      - read shadow area
;*                      - read SRAM ECC error register
;*                      - check PER bit for signalled ECC errors
;*                      - if no error signalled we're done - exit
;*                      - if error found, check if it was in shadow area
;*                      - if so test area causes shadow area to change - fault
;*                      - if ECC error address is different call scrubbing function
;*
;* EDIT HISTORY:
;* [0] 19-APR-2013 SP  Initial version
;*
;*      Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;*
;*      Registers used:         r0 (a1) = Scratch/ SRAM ECC register address
;*                              r1 (a2) = SRAM ECC register content / ECC error address
;*                              r2 (a3) = (I) Shadow RAM Address (word boundary)
;*                              r3 (a4) = not used
;*                              r4 (v1) = not used
;*                              r5 (v2) = not used
;*                              r6 (v3) = not used
;*                              r7 (v4) = not used
;*                              r8 (v5) = not used
;*                              r9 (v6) = not used
;*                              r10 (v7) = not used
;*
;***********************************************************************/
dgnCheckShadowArea PROC

        STMFD   sp!, {r0, r1, lr}

        LDR     r0, [r2]                          ; Read shadow area
        LDR     r0, dgn_SramEccError              ; Get ECC error register
        LDR     r1, [r0]                          ; Get register content

        ;Start Fault Injection Point 2
        ;STMFD   sp!, {r6, r7}
        ;LDR        r6, =InjectFaultFlag
        ;LDR        r7, [r6]
        ;CMP        r7, #1
        ;BNE        WithoutFaultInjection
        ;Actual fault injection
        ;MOV     r1, r2
        ;SUB     r1, r1, #DGN_RAM_SPACE_START
        ;ORR     r1, r1, #DGN_ECC_ERR_MASK_SINGLE
;WithoutFaultInjection
        ;LDMFD  sp!, {r6, r7}
        ;If above lines of code are uncommented then APEX will fault during shadow
        ;area check
        ;End Fault Injection Point 2

        TST     r1, #DGN_ECC_ERR_MASK_SINGLE      ; Check per bit
        BEQ     dgn_ShadowOk                      ; No error, we're done

        AND     r1, r1, #DGN_ECC_ERR_MASK_OFFSET  ; Get offset
        ADD     r1, r1, #DGN_RAM_SPACE_START      ; Add to start memory address to get actual address
        CMP     r1, r2                            ; if error in shadow area...
        MOVEQ   a2, #51                           ; Error in shadow ram detected, "LINE 51"
        BEQ     dgnRAMError

        BL      dgnScrubEccError

dgn_ShadowOk

        LDMIA   sp!, {r0, r1, lr}
        BX      lr

        ENDP

;***********************************************************************
;*
;* FUNCTION NAME:       ApexDiagRamDataRunTestAsm
;*
;* FULL DESC:           RAM data verification function.
;*
;*                      For details see ICE2_ESW_Platform Diagnostics_HLDD,
;*                      section 7.27 Apex2 RAM Data Pattern Test with Shadow Memory.
;*
;*                      Here's just a brief description:
;*
;*                      The 0x99c966c6 test pattern, along with the three other
;*                      variations derived by rotating this pattern by 4 bits
;*                      comprises a set of four unique data patterns.  This pattern
;*                      set is designed to ensure that all of the upper 16 bits of
;*                      a 32-bit memory word are independent of each other, and all of
;*                      the lower 16 bits are independent of each other.
;*
;*                      To illustrate this, we have the four variations of the rotating
;*                      0x99c966c6 pattern:
;*
;*                      Pattern 1 = 0x99c966c6
;*                      Pattern 2 = 0x9c966c69
;*                      Pattern 3 = 0xc966c699
;*                      Pattern 4 = 0x966c699c
;*
;*                      In order to make sure that each bit in each location
;*                      can be set to a one or a zero, an inverted pattern is
;*                      written on the second pass.
;*
;*                      As the data pattern tests are occurring in the four-word
;*                      test area, the shadow area will be observed to see if bits
;*                      change. Since ECC is used for values stored in the memory,
;*                      it is used to assist in the detection of altered bits.
;*                      Then shadow area is checked to detect cross-coupling between
;*                      memory words.
;*
;*                      We start by writing four patterns to test area and reading it
;*                      back. For each location, we'll then rotate though all four patterns,
;*                      followed by shadow area check, followed by successive upper and
;*                      lower 16-bit inversions and shadow area check again. Then we'll
;*                      move to next test location.
;*
;*                      Sequence of operations is:
;*                      - Disable interrupts
;*                      - Disable cache
;*                      - Save original data from first four RAM locations into registers
;*                      - Scrub any existing ECC error
;*                      - Read (and if necessary scrub) shadow area
;*                      - Write a pattern into first location
;*                      - Rotate pattern 4 bits
;*                      - Write pattern to other three locations, rotating
;*                              pattern before each write
;*                      - Check each pattern by reading back. HW_HandleException if bad.
;*                      - Repeat for all four variations of a pattern in all four locations
;*                      - Check for ECC error when reading shadow area - HW_HandleException for error.
;*                      - Invert the hi bits in the final pattern
;*                      - Write inverted pattern to all four locations
;*                      - Check each location by reading back. HW_HandleException if bad.
;*                      - Invert the lo bits in the pattern (so all bits are inverted compared to final pattern)
;*                      - Write inverted pattern to all four locations
;*                      - Check each location by reading back. HW_HandleException if bad.
;*                      - Restore original data to the four locations
;*                      - Enable cache
;*                      - Enable interrupts
;*                      - Check if done.  If not, repeat.
;*
;* EDIT HISTORY:
;* [0] 08-APR-2005 WBC  Initial version
;* [1] 30-AUG-2013 SP  Added shadow RAM check and ECC errors scrubbing
;*
;*      Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;*
;*      Registers altered:      None
;*
;*      Registers used:         r0 (a1) = (I) Number of 4-word groups to check
;*                                        (R) Zero if success, should call
;*                                        HW_HandleException and not return if failure.
;*                              r1 (a2) = (I) Starting address (word boundary)
;*                              r2 (a3) = (I) Shadow RAM Address (word boundary)
;*                              r3 (a4) = Scratch used for holding pattern
;*                              r4 (v1) = dgn_PatternTest loop index - number of bits
;*                                        original pattern should be shifted for each step
;*                              r5 (v2) = Original Pattern
;*                              r6 (v3) = Original data from first location
;*                              r7 (v4) = Original data from second location
;*                              r8 (v5) = Original data from third location
;*                              r9 (v6) = Original data from fourth location
;*                              r10 (v7) = Scratch
;***********************************************************************

ApexDiagRamDataRunTestAsm
                                        ;Save used registers and stack
                                        ; backtrace info.
        MOV     ip,sp
        STMFD   sp!, {r4-r10, fp, ip, lr, pc}
        SUB     fp, ip, #4


        MOV     r10, r0                 ;Keep r0 from getting clobbered

        BLX     HI_DisableIrq           ;Disable interrupts


        ; Clean and disable cache
        STMFD   sp!, {r0-r3}            ;Save r0-r3 into the stack
        bl      DisableDataCache        ;Disable and clean cache
        LDMFD   sp!, {r0-r3}            ;Restore r0-r3


        MOV     r0, r10                 ;Restore r0

        CMP     r0, #0                  ;Do we have 0 groups to check (i.e.
                                        ; are we doing an overhead measurement)
        BEQ     dgnDataDone             ;If so, finish up

dgn_LoopBegin

        LDMIA   r1, {r6-r9}             ;Save orig data from first 4 locations
        LDR     r5, dgn_RAMFinalPattern      ;Get RAM pattern (stored in ROM)
        MOV     r4, #12                 ;dgn_PatternTest loop index - the value
                                        ; represents how many bits original pattern
                                        ; should be shifted to the right with each
                                        ; step, starting with 12 and decreasing by 4
                                        ; every loop

        BL      dgnScrubEccError        ;Scrub any previous ECC errors
        LDR     r10, [r2]               ;Read shadow area
        BL      dgnScrubEccError        ;Scrub shadow ram in case of ECC error

dgn_PatternTest
                                        ;Store pattern in 4 locations, rotating
                                        ; it 4 bits left (28 bits right) before each store
                                        ;Pattern is shifted by 4 bits so at some point we'll
                                        ; have to "wrap" it around, for example:
                                        ;   Pattern 1 = 0x99c966c6
                                        ;   Pattern 2 = 0x9c966c69
                                        ;   Pattern 3 = 0xc966c699
                                        ;   Pattern 4 = 0x966c699c
                                        ;   Pattern 5 = 0x66c699c9 <---- to "convert it" to
                                        ;                                Pattern 1 we'll have to
                                        ;                                shift it by additional
                                        ;                                16 bits to wrap around

        MOV     r3, r5, ROR r4          ;Get the pattern and shift it depending on the loop
                                        ; step (12, 8 , 4 or 0 bits)
        STR     r3, [r1], #4

        CMP     r4, #0                  ;Depending on the step shift pattern by:
        MOVNE   r3, r3, ROR #28         ; 4 bits right or
        MOVEQ   r3, r3, ROR #12         ; 4 bits and 16 to wrap pattern around
        STR     r3, [r1], #4

        CMP     r4, #4                  ;Depending on the step shift pattern by:
        MOVNE   r3, r3, ROR #28         ; 4 bits right or
        MOVEQ   r3, r3, ROR #12         ; 4 bits and 16 to wrap pattern around
        STR     r3, [r1], #4

        CMP     r4, #8                  ;Depending on the step shift pattern by:
        MOVNE   r3, r3, ROR #28         ; 4 bits right or
        MOVEQ   r3, r3, ROR #12         ; 4 bits and 16 to wrap pattern around
        STR     r3, [r1]


        SUB     r1, r1, #12             ; Decrement addr to first location
        MOV     r3, r5, ROR r4          ; Copy first pattern to r3

                                        ;Check each location
        LDR     r10,[r1], #4

        ;Start Fault Injection Point 1
        ;MOV     r10, r10, ROR #1
        ;If above line of code is uncommented then APEX will fault during pattern
        ;testing
        ;End Fault Injection Point 1

        CMP     r3, r10
        MOVNE   a2, #1                  ;Data faults, 1st word, are "LINE 1"
        BLNE    dgnRAMError
        CMP     r4, #0
        MOVNE   r3, r3, ROR #28
        MOVEQ   r3, r3, ROR #12

        LDR     r10,[r1], #4
        CMP     r3, r10
        MOVNE   a2, #2                  ;Data faults, 2nd word, are "LINE 2"
        BLNE    dgnRAMError
        CMP     r4, #4
        MOVNE   r3, r3, ROR #28
        MOVEQ   r3, r3, ROR #12

        LDR     r10,[r1], #4
        CMP     r3, r10
        MOVNE   a2, #3                  ;Data faults, 3rd word, are "LINE 3"
        BLNE    dgnRAMError
        CMP     r4, #8
        MOVNE   r3, r3, ROR #28
        MOVEQ   r3, r3, ROR #12

        LDR     r10,[r1]
        CMP     r3, r10
        MOVNE   a2, #4                  ;Data faults, 4th word, are "LINE 4"
        BLNE    dgnRAMError

        SUB     r1, r1, #12             ; Decrement addr to first location

        SUBS    r4, r4, #4              ; did we check with all four variations
        BPL     dgn_PatternTest         ; loop if not

                                        ; Pattern tests done, check shadow area and test with inverted patterns

        BL      dgnCheckShadowArea      ; Check shadow area

        EOR     r3, r5, #0xFF000000     ; r5 holds pattern "final" pattern
        EOR     r3, r3, #0x00FF0000     ; r3 = inverted hi bits of final pattern

                                        ;Store inverted pattern in all four locations (r1 points to last one)
        STR     r3, [r1], #4
        STR     r3, [r1], #4
        STR     r3, [r1], #4
        STR     r3, [r1]

                                        ; check inverted pattern was read correctly
        LDR     r10,[r1],#-4

        CMP     r3, r10
        MOVNE   a2, #5                  ;Data faults, 1st word, inverted pattern, are "LINE 5"
        BLNE    dgnRAMError

        LDR     r10,[r1],#-4
        CMP     r3, r10
        MOVNE   a2, #6                  ;Data faults, 2nd word, inverted pattern, are "LINE 6"
        BLNE    dgnRAMError

        LDR     r10,[r1],#-4
        CMP     r3, r10
        MOVNE   a2, #7                  ;Data faults, 3rd word, inverted pattern, are "LINE 7"
        BLNE    dgnRAMError

        LDR     r10,[r1]
        CMP     r3, r10
        MOVNE   a2, #8                  ;Data faults, 4th word, inverted pattern, are "LINE 8"
        BLNE    dgnRAMError

        BL      dgnCheckShadowArea      ; Check shadow area

        EOR     r3, r3, #0x0000FF00
        EOR     r3, r3, #0x000000FF     ; r5 = inverted all bits of final pattern (high bits are already inverted)

                                        ;Store second inverted pattern in all four locations
        STR     r3, [r1], #4
        STR     r3, [r1], #4
        STR     r3, [r1], #4
        STR     r3, [r1]

                                        ; check inverted pattern was read correctly
        LDR     r10,[r1],#-4
        CMP     r3, r10
        MOVNE   a2, #9                  ;Data faults, 1st word, inverted pattern, are "LINE 9"
        BLNE    dgnRAMError

        LDR     r10,[r1],#-4
        CMP     r3, r10
        MOVNE   a2, #10                 ;Data faults, 2nd word, inverted pattern, are "LINE 10"
        BLNE    dgnRAMError

        LDR     r10,[r1],#-4
        CMP     r3, r10
        MOVNE   a2, #11                 ;Data faults, 3rd word, inverted pattern, are "LINE 11"
        BLNE    dgnRAMError

        LDR     r10,[r1]
        CMP     r3, r10
        MOVNE   a2, #12                 ;Data faults, 4th word, inverted pattern, are "LINE 12"
        BLNE    dgnRAMError

        BL      dgnCheckShadowArea      ; Check shadow area

        STMIA   r1!,{r6-r9}             ;Restore orig data to first 4 locations
                                        ; and advance pointer to next block
                                        ; of 4 data locations

        SUBS     r0, r0, #1             ;Test next block of 4 words if not done
        BNE     dgn_LoopBegin

dgnDataDone
        ; Flush and enable cache
        BL      FlushDataCache
        BL      EnableDataCache

        BLX     HI_EnableIrq            ;Enable interrupts

        MOV     r0, #0                  ;Return value for success
                                        ;Restore used registers and return
        LDMEA   fp, {r4-r10, fp, sp, pc}


;***********************************************************************
;*
;* FUNCTION NAME:       ApexDiagRamAddressRunTestAsm
;*
;* FULL DESC:           RAM address line independence diagnostics.  This
;*                      routine will check the ability of the hardware to
;*                      write and read back independent data from addresses
;*                      that differ by one bit.  The checks are done by
;*                      comparing two addresses at a time, and the number of
;*                      pairs to be checked per diagnostic slice is found in
;*                      r0 on entry.  Starting addresses to be compared are
;*                      supplied in r1 and r2 upon entry.
;*
;*                      The function will return zero in r0 if successful,
;*                      but not totally complete, and will return one in r0
;*                      if all pairs have been checked.
;*
;*                      If the iterations have completed successfully, but
;*                      all pairs have not yet been checked, the function
;*                      will also update the current address values in
;*                      DgnSafeRam.
;*
;*                      If a failure is detected, the function will call
;*                      HW_HandleException from within the function (and not return).
;*
;* EDIT HISTORY:
;* [0] 08-APR-2005 WBC  Initial version
;*
;*      Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;*
;*      Registers altered:      None
;*
;*      Registers used:         r0 (a1) = Number of address pairs to check
;*                                        (R) Zero if success but incomplete,
;*                                        one if complete & successful.  Calls
;*                                        HW_HandleException and won't return if failure.
;*                              r1 (a2) = Address 1
;*                              r2 (a3) = Address 2
;*                              r3 (a4) = Number of significant address bits
;*                              r4 (v1) = Memory type
;*                              r5 (v2) = Scratch
;*                              r6 (v3) = Scratch
;*                              r7 (v4) = Scratch
;*                              r8 (v5) = Scratch
;*                              r9 (v6) = Current bit to invert
;*                              r10 (v7) = Scratch
;***********************************************************************
ApexDiagRamAddressRunTestAsm
                                        ;Save used registers and stack
                                        ; backtrace info.
        MOV     ip,sp
        STMFD   sp!, {r4-r10, fp, ip, lr, pc}
        SUB     fp, ip, #4

        LDR     r5, =DgnSafeRam          ;Get pointer to base of DgnSafeRam

        LDR     r0, [r5, #dgn_AddrIterationsLoc]    ;Get Num Iterations in DgnSafeRam
        LDR     r3, [r5, #dgn_AddrIterationsPrimeLoc] ;Get Num Iterations prime in DgnSafeRam
        MVN     r3, r3                              ;  and invert
        CMP     r3, r0                              ;Make sure they're valid
        MOVNE   a2, #13                             ;Invalid Addr iterations are "LINE 13"
        BLNE    dgnRAMError

        LDR     r1, [r5, #dgn_Addr1Loc]             ;Get Addr1 in DgnSafeRam
        LDR     r3, [r5, #dgn_Addr1PrimeLoc]        ;Get Addr1 prime in DgnSafeRam
        MVN     r3, r3                              ;  and invert
        CMP     r3, r1                              ;Make sure they're valid
        MOVNE   a2, #14                             ;Invalid Addr1 is "LINE 14"
        BLNE    dgnRAMError

        LDR     r2, [r5, #dgn_Addr2Loc]             ;Get Addr2 in DgnSafeRam
        LDR     r3, [r5, #dgn_Addr2PrimeLoc]        ;Get Addr2 prime in DgnSafeRam
        MVN     r3, r3                              ;  and invert
        CMP     r3, r2                              ;Make sure they're valid
        MOVNE   a2, #15                             ;Invalid Addr2 is "LINE 15"
        BLNE    dgnRAMError


        CMP     r1, r2                  ;Addr1 and Addr2 shouldn't be equal
        MOVEQ   a2, #16                 ;Addr1 = Addr2 fault is "LINE 16"
        BEQ     dgnRAMError

        MOV     r10, #0
        
dgn_intDataRAMRange
        LDR     r8, dgn_IntDataRAMBegin
        MOV     r6, r8                       ;Hold start addr for bounds chks
        LDR     r7, [r5, #dgn_IntDataRAMSiz] ;Int data RAM size (in DgnSafeRam)
                
dgn_calcMaxAddr
        ADD     r8, r8, r7              ;Add size to base address
        SUB     r8, r8, #1              ;Subtract 1 to get max address
        BIC     r8, r8, #3              ;Put it on a 32-bit word boundary

dgn_AddrBoundsChk

        CMP     r1, r6
        MOVLT   a2, #17                 ;Addr1 too low faults are "LINE 17"
        BLT     dgnRAMError             ;If Address 1 < start address, fail
        CMP     r1, r8
        MOVGT   a2, #18                 ;Addr1 too high faults are "LINE 18"
        BGT     dgnRAMError             ;If Address 1 > max address, fail
        CMP     r2, r6
        MOVLT   a2, #19                 ;Addr2 too low faults are "LINE 19"
        BLT     dgnRAMError             ;If Address 2 < start address, fail
        CMP     r2, r8
        MOVGT   a2, #20                 ;Addr2 too high faults are "LINE 20"
        BGT     dgnRAMError             ;If Address 2 > max address, fail

                                        ; Calculate # of signif address bits
                                        ; by masking off upper unchanging bits
                                        ; of the max addr and then counting the
                                        ; remaining bits from the leading 'one'

        MOV     r6, r8                  ;Put max address into scratch register

        BIC     r6, r6, #DGN_INT_DATA_RAM_START ;Mask leading non-zero bits

        CMP     r6, #0                  ;Make sure we have at least one 'one'
        MOVEQ   a2, #21                 ;No leading ones fault is "LINE 21"
        BEQ     dgnRAMError

                                        ;Find leading 'one' to see how many bits
                                        ; we will need to toggle during test.
        MOV     r3, #31                 ;Start counting backward from bit 31

dgn_findLeadingOne
                                        ;Keep checking MS bit until a 1 is found
        TST     r6, #&80000000          ;Is upper bit set?
        SUBEQ   r3, r3, #1              ;If not, decrement bit count
        MOVEQ   r6, r6, LSL #1          ; ...and shift bits by one to chk next
        BEQ     dgn_findLeadingOne

dgn_findTestBitPosition
                                        ;Compare Addr1 and Addr2 to figure out
                                        ; where the last addr test left off
        MOV     r9, #2                  ;Start looking at bit 2
        MOV     r6, #1                  ;This 1 will be used to test bits
        EOR     r7, r1, r2              ;Find different bit(s)

dgn_findDifferentBit

        CMP     r9, r3                  ;Did we chk all bits & no differences?
        MOVGT   a2, #22                 ;No different significant bits "LINE 22"
        BGT     dgnRAMError

        TST     r7, r6, LSL r9          ;Is this bit different?
        ADDEQ   r9, r9, #1              ;If not, point to next one
        BEQ     dgn_findDifferentBit

        CMP     r0, #0                   ; If iterations = 0 (i.e., we're
        BEQ     dgnAddrStore             ; measuring overhead), finish up

dgn_checkAddrPair
        
        STMFD	sp!, {r0-r3}			;Put r0-r3 into the stack
        
        BLX     HI_DisableIrq           ;Disable interrupts

        ; Clean and disable cache
		bl		DisableDataCache		;Clean and disable cache
		LDMFD   sp!, {r0-r3}            ;Restore r0-r3 from the stack              

        ;Start Fault Injection Point 3
        ;MOV        r2, r1
        ;End Fault Injection Point 3
        
        ;Start Fault Injection Point 4
        ;LDR        r6, =InjectFaultFlag
        ;LDR        r7, [r6]
        ;CMP        r7, #1
        ;BNE        WithoutFaultInjection
        ;Simulate that Addr2 is the same as Addr1
        ;MOV        r2, r1
;WithoutFaultInjection
        ;End Fault Injection Point 4

        LDR     r6, [r1]                ;Save original RAM values
        LDR     r7, [r2]

        LDR     r10, dgn_AddrPattern    ;Get pattern
        STR     r10, [r1]               ;Put it at Addr1
        MVN     r10, r10                ;Invert pattern
        STR     r10, [r2]               ;Put it at Addr2
        MVN     r10, r10
        LDR     r5, [r1]
        CMP     r5, r10                 ;Check data at Addr1
        MOVNE   a2, #23                 ;Address dependency error is "LINE 23"
        BLNE    dgnRAMError

        MVN     r10, r10
        LDR     r5, [r2]
        CMP     r5, r10                 ;Check data at Addr2
        MOVNE   a2, #24                 ;Address dependency error is "LINE 24"
        BLNE    dgnRAMError

        STR     r6, [r1]                ;Restore original RAM values
        STR     r7, [r2]

		STMFD	sp!, {r0-r3}			;Put r0-r3 into the stack

        ; Flush and enable cache
        BL      FlushDataCache
        BL      EnableDataCache

        BLX     HI_EnableIrq            ;Enable interrupts       
        LDMFD   sp!, {r0-r3}            ;Restore r0-r3 from the stack    
        MOV     r6, #1                  ;This 1 will be used to change bits

dgnFormNextAddr

        EOR     r2, r2, r6, LSL r9      ;Uninvert bit in Addr2
        ADD     r9, r9, #1              ;Point to next bit
        CMP     r9, r3                  ;Have we finished testing signif bits?
        BGT     dgnDoneWithBits

dgnBitInvert

        EOR     r2, r2, r6, LSL r9      ;Invert next bit in Addr2
        CMP     r2, r8
        BGT     dgnFormNextAddr         ;If Addr2 greater than the max address,
                                        ; try again.
        BLE     dgnAddrDoneChks         ;Otherwise, see if we can check another
                                        ; location with the new address

dgnDoneWithBits

        TST     r1, #4                  ;Was Addr1 a minimum (mostly zeroes)
                                        ; address?
        MOVNE   r0, #1                  ;If not, get ready to return a 1,
        BNE     dgnAddrRetDone          ; and return
        MOV     r1, r8                  ;If so, make it a max (mostly ones)
                                        ; address.
        MOV     r2, r1                  ;Copy it to Addr2
        BIC     r2, r2, #&4             ; ...and clear bit 2
        MOV     r9, #2                  ; ...and point again to bit 2 of Addr2

dgnAddrDoneChks
        SUB     r0, r0, #1              ;Decrement iteration count
        CMP     r0, #0                  ;Have we used up all allowed iterations?
        BNE     dgn_checkAddrPair       ;If not, check next address pair

dgnAddrStore
        LDR     r5, =DgnSafeRam                 ;Get base of DgnSafeRam

        STR     r1, [r5, #dgn_Addr1Loc]         ;Store Addr1 in DgnSafeRam
        MVN     r1, r1                          ;Invert to form Addr1 prime
        STR     r1, [r5, #dgn_Addr1PrimeLoc]    ;Store Addr1 prime in DgnSafeRam

        STR     r2, [r5, #dgn_Addr2Loc]         ;Store Addr2 in DgnSafeRam
        MVN     r2, r2                          ;Invert to form Addr2 prime
        STR     r2, [r5, #dgn_Addr2PrimeLoc]    ;Store Addr2 prime in DgnSafeRam

        MOV     r0, #0                  ;Return zero, since incomplete
                                        ;Restore used registers and return.
dgnAddrRetDone

        LDMEA   fp, {r4-r10, fp, sp, pc}

;***************** FILE NAME STRING USED IF HW_HandleException CALLED ***********
dgnramtstFile
        DCB     "apxDiagRAMassy.s",0    ;File name string used if HW_HandleException
                                        ;       called
                ALIGN


;******************************************************************************;
;*   END of file
;******************************************************************************;

        END


