#include <asm_sprt.h>                              /* Added for ssync( ), cli/sti( ) */
#include "BlackfinDiagRegTestCommon.hpp"

	.section/DOUBLEANY program;
	.align 4;

_BlackfinDiagRegSanityChk:
	link 0;
     // 
    // The idea here is to check a few registes manually for operation sanity by using 
    // a few simple test patterns.  Then if those registers pass use those known 
    // operational registers to test the rest against an input test pattern array.
    //
    // We're going to test the sanity of r0, r1, r2, and p1.  We'll write and read back
    // all ones, all zeros, alternating 1's and 0's with the msb set and reset.
    //
    r0.h = AllOnesPattern;
    r0.l = AllOnesPattern;  // All ones
    r1.h = AllOnesPattern;
    r1.l = AllOnesPattern;
    cc = r0 == r1;
    if cc jump R0R1AllOnesPassed;
    //
    // First Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R1SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R1AllOnesPassed:
	r2.h = AllOnesPattern;
	r2.l = AllOnesPattern;
	
    cc = r0 == r2;
    if cc jump R0R2AllOnesPassed;
    //
    // Second Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R2AllOnesPassed:
    cc = r1 == r2;
    if cc jump R1R2AllOnesPassed;
    //
    // Third Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R1R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R1R2AllOnesPassed:
    r0.h = AllZerosPattern;
    r0.l = AllZerosPattern;  // All zeros
    r1.h = AllZerosPattern;
    r1.l = AllZerosPattern;
    cc = r0 == r1;
    if cc jump R0R1AllZerosPassed;
    //
    // Fourth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R1SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R1AllZerosPassed:
	r2.h = AllZerosPattern;
	r2.l = AllZerosPattern;
    cc = r0 == r2;
    if cc jump R0R2AllZerosPassed;
    //
    // Fifth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R2AllZerosPassed:
    cc = r1 == r2;
    if cc jump R1R2AllZerosPassed;
    //
    // Sixth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R1R2SanityCheckFail;
    // r0.h Already has the FailurePattern
Exit:
    unlink;
    rts;
R1R2AllZerosPassed:
    r0.h = AlternatingOnesZeros;
    r0.l = AlternatingOnesZeros;  // Alternating ones, zeros
    r1.h = AlternatingOnesZeros;
    r1.l = AlternatingOnesZeros;
    cc = r0 == r1;
    if cc jump R0R1AltOnesZerosPassed;
    //
    // Seventh Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R1SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R1AltOnesZerosPassed:
	r2.h = AlternatingOnesZeros;
	r2.l = AlternatingOnesZeros;
    cc = r0 == r2;
    if cc jump R0R2AltOnesZerosPassed;
    //
    // Eighth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R2AltOnesZerosPassed:
    cc = r1 == r2;
    if cc jump R1R2AltOnesZerosPassed;
    //
    // Nineth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R1R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
R1R2AltOnesZerosPassed:
    r0.h = AlternatingZerosOnes;
    r0.l = AlternatingZerosOnes;  // Alternating ones, zeros
    r1.h = AlternatingZerosOnes;
    r1.l = AlternatingZerosOnes;
    cc = r0 == r1;
    if cc jump R0R1AltZerosOnesPassed;
    //
    // Tenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R1SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R1AltZerosOnesPassed:
	r2.h = AlternatingZerosOnes;
	r2.l = AlternatingZerosOnes;
    cc = r0 == r2;
    if cc jump R0R2AltZerosOnesPassed;
    //
    // Eleventh Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R2AltZerosOnesPassed:
    cc = r1 == r2;
    if cc jump R1R2AltZerosOnesPassed;
    //
    // Twelth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R1R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
R1R2AltZerosOnesPassed:
//
// At this point we know that r0-r2 are at least comparing inverting pattens correctly.
// So we'll assume they are working based on the results.  We need P1 to loop through
// Input patters so lets do sanity checks on it.
//
    r0.h = AllOnesPattern;
    r0.l = AllOnesPattern;
    p1.h = AllOnesPattern;
    p1.l = AllOnesPattern;
    r1 = p1;
    cc = r0 == r1;
    if cc jump P0AllOnesPassed;
    //
    // Thirteenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | P0SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
P0AllOnesPassed:
    r0.h = AllZerosPattern;
    r0.l = AllZerosPattern;
    p1.h = AllZerosPattern;
    p1.l = AllZerosPattern;
    r1 = p1;
    cc = r0 == r1;
    if cc jump P0AllZerosPassed;
    //
    // Fourteenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | P0SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
P0AllZerosPassed:
    r0.h = AlternatingOnesZeros;
    r0.l = AlternatingOnesZeros;
    p1.h = AlternatingOnesZeros;
    p1.l = AlternatingOnesZeros;
    r1 = p1;
    cc = r0 == r1;
    if cc jump P0AltOnesZerosPassed;
    //
    // Fifteenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | P0SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
P0AltOnesZerosPassed:
    r0.h = AlternatingZerosOnes;
    r0.l = AlternatingZerosOnes;
    p1.h = AlternatingZerosOnes;
    p1.l = AlternatingZerosOnes;
    r1 = p1;
    cc = r0 == r1;
    if cc jump P0AltZerosOnesPassed;
    //
    // Sixteenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | P0SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
P0AltZerosOnesPassed:
    r0 = 0;              // Test passed
    jump.s Exit;
_BlackfinDiagRegSanityChk.end:

	
_BlackfinDiagRegDataReg7Chk:
	link 0;
//
// Preserve Callers r7 register
//
 	[--sp] = r7;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckR7:
	cc = r2 < r1 (iu);
	
	if cc jump NextR7Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
R7Exit:
//
// Restore caller's r7
//
	r7 = [sp++];
	unlink;
	rts;    

NextR7Pattern:
	r0 = [p1];                 // Load next pattern
	
	r7 = [p1];                 // into regs
	
	cc = r0 == r7;             // Patterns Match?
	
	if cc jump R7Continue;     // Yes try for another pattern to test against
	
	r0 = r2;                           // Save index of pattern failure in upper 16 bits
    r0 <<= TestIdxBitPos;
    r0.l = ( DataRegFailure << TestFailurePos ) | R7Failure; // Indicated error type to the caller
    
    jump.s R7Exit;               

R7Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckR7;
_BlackfinDiagRegDataReg7Chk.end:

_BlackfinDiagRegDataReg6Chk:
	link 0;
//
// Preserve Callers r6 register
//
 	[--sp] = r6;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckR6:
	cc = r2 < r1 (iu);
	
	if cc jump NextR6Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
R6Exit:
//
// Restore caller's r6
//
	r6 = [sp++];
	unlink;
	rts;    

NextR6Pattern:
	r0 = [p1];                 // Load next pattern
	
	r6 = [p1];                 // into regs
	
	cc = r0 == r6;             // Patterns Match?
	
	if cc jump R6Continue;     // Yes try for another pattern to test against
	
	r0 = r2;                           // Save index of pattern failure in upper 16 bits
    r0 <<= TestIdxBitPos;
    r0.l = ( DataRegFailure << TestFailurePos ) | R6Failure; // Indicated error type to the caller
    
    jump.s R6Exit;               

R6Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckR6;
_BlackfinDiagRegDataReg6Chk.end:

_BlackfinDiagRegDataReg5Chk:
	link 0;
//
// Preserve Callers r5 register
//
 	[--sp] = r5;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckR5:
	cc = r2 < r1 (iu);
	
	if cc jump NextR6Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
R5Exit:
//
// Restore caller's r5
//
	r5 = [sp++];
	unlink;
	rts;    

NextR5Pattern:
	r0 = [p1];                 // Load next pattern
	
	r5 = [p1];                 // into regs
	
	cc = r0 == r5;             // Patterns Match?
	
	if cc jump R5Continue;     // Yes try for another pattern to test against
	
	r0 = r2;                           // Save index of pattern failure in upper 16 bits
    r0 <<= TestIdxBitPos;
    r0.l = ( DataRegFailure << TestFailurePos ) | R5Failure; // Indicated error type to the caller
    
    jump.s R5Exit;               

R5Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckR5;

_BlackfinDiagRegDataReg5Chk.end:

_BlackfinDiagRegDataReg4Chk:
	link 0;
//
// Preserve Callers r4 register
//
 	[--sp] = r4;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckR4:
	cc = r2 < r1 (iu);
	
	if cc jump NextR4Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
R4Exit:
//
// Restore caller's r4
//
	r4 = [sp++];
	unlink;
	rts;    

NextR4Pattern:
	r0 = [p1];                 // Load next pattern
	
	r4 = [p1];                 // into regs
	
	cc = r0 == r4;             // Patterns Match?
	
	if cc jump R4Continue;     // Yes try for another pattern to test against
	
	r0 = r2;                           // Save index of pattern failure in upper 16 bits
    r0 <<= 16;                         // 
    r0 <<= TestIdxBitPos;
    r0.l = ( DataRegFailure << TestFailurePos ) | R4Failure; // Indicated error type to the caller
    
    
    jump.s R4Exit;               

R4Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckR4;

_BlackfinDiagRegDataReg4Chk.end:

_BlackfinDiagRegDataReg3Chk:
	link 0;
//
// Preserve Callers r3 register
//
 	[--sp] = r3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckR3:
	cc = r2 < r1 (iu);
	
	if cc jump NextR3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
R3Exit:
//
// Restore caller's r3
//
	r3 = [sp++];
	unlink;
	rts;    

NextR3Pattern:
	r0 = [p1];                 // Load next pattern
	
	r6 = [p1];                 // into regs
	
	cc = r0 == r6;             // Patterns Match?
	
	if cc jump R3Continue;     // Yes try for another pattern to test against
	
	r0 = r2;                           // Save index of pattern failure in upper 16 bits
    r0 <<= TestIdxBitPos;
    r0.l = ( DataRegFailure << TestFailurePos ) | R3Failure; // Indicated error type to the caller
    jump.s R3Exit;               

R3Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckR3;

_BlackfinDiagRegDataReg3Chk.end:

_BlackfinDiagRegPointerReg5Chk:
	link 4;
//
// Preserve Callers p5 register
//
 	[--sp] = p5;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckP5:
   
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextP5Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
P5Exit:
//
// Restore caller's p5
//
	p5 = [sp++];
	unlink;
	rts;    

NextP5Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	p5 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = p5;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump P5Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( PointerRegFailure<< TestFailurePos ) | P5Failure; // Indicated error type to the caller
    jump.s P5Exit;               

P5Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckP5;
_BlackfinDiagRegPointerReg5Chk.end:

_BlackfinDiagRegPointerReg4Chk:
	link 4;
//
// Preserve Callers p4 register
//
 	[--sp] = p4;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckP4:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextP4Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
P4Exit:
//
// Restore caller's p4
//
	p4 = [sp++];
	unlink;
	rts;    

NextP4Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	p4 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;
	
	r2 = p4;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump P4Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( PointerRegFailure<< TestFailurePos ) | P4Failure; // Indicated error type to the caller
    
    jump.s P4Exit;               

P4Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckP4;
_BlackfinDiagRegPointerReg4Chk.end:

_BlackfinDiagRegPointerReg3Chk:
	link 4;
//
// Preserve Callers p3 register
//
 	[--sp] = p3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckP3:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextP3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
P3Exit:
//
// Restore caller's p3
//
	p3 = [sp++];
	unlink;
	rts;    

NextP3Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	p3 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;
	r2 = p3;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump P3Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( PointerRegFailure<< TestFailurePos ) | P3Failure; // Indicated error type to the caller
    
    
    jump.s P3Exit;               

P3Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckP3;
_BlackfinDiagRegPointerReg3Chk.end:

_BlackfinDiagRegPointerReg2Chk:
	link 4;
//
// Preserve Callers p2 register
//
 	[--sp] = p2;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckP2:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextP2Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
P2Exit:
//
// Restore caller's p2
//
	p2 = [sp++];
	unlink;
	rts;    

NextP2Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	p2 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = p2;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump P2Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( PointerRegFailure<< TestFailurePos ) | P2Failure; // Indicated error type to the caller
    
    
    jump.s P2Exit;               

P2Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckP2;
_BlackfinDiagRegPointerReg2Chk.end:

_BlackfinDiagRegPointerReg0Chk:
	link 4;
//
// Preserve Callers p0 register
//
 	[--sp] = p0;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckP0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextP0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
P0Exit:
//
// Restore caller's p0
//
	p0 = [sp++];
	unlink;
	rts;    

NextP0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	p0 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = p0;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump P0Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( PointerRegFailure<< TestFailurePos ) | P0Failure; // Indicated error type to the caller
    
    
    jump.s P0Exit;               

P0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckP0;
_BlackfinDiagRegPointerReg0Chk.end:

_BlackfinDiagAccum0Chk:
	link 8;
//
// Preserve Callers p0 register
//
 	[--sp] = a0.w;
 	[--sp] = a0.x;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckA0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextA0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Accum0Exit:
//
// Restore caller's a0
//
	a0.x = [sp++];
	a0.w = [sp++];
	
	unlink;
	rts;    

NextA0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	a0.x = r0.l;
	
	a0.w = r0;
	
	r2 = a0.w;
				
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Accum0ContinueWithX;     // Yes try for another pattern to test against
	
Accum0Failure:
    r0 = [fp-4];                       // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( AccumFailure << TestFailurePos ) | A0Failure32; // Indicated error type to the caller
    
    
    jump.s Accum0Exit;  
                 
Accum0ContinueWithX:

	[fp-8] = r0;
	
	r2.l = a0.x;
	r2.h = 0;
	
	r0.l = 0xff;
	r0.h = 0;
	
	r2 = r0 & r2;
	
	r0 = [fp-8];
	
	[fp-8] = r2;
	
	r2.l = 0xff;
	
	r2.h = 0;
	
	r0 = r0 & r2;
	
	r2 = [fp-8];
	
	cc = r0 == r2;

	if cc jump Accum0Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                       // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( AccumFailure << TestFailurePos ) | A0Failure8; // Indicated error type to the caller
    
    
    jump.s Accum0Exit;  
	
Accum0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckA0;
_BlackfinDiagAccum0Chk.end:

_BlackfinDiagAccum1Chk:
	link 4;
//
// Preserve Callers p0 register
//
 	[--sp] = a1.w;
 	[--sp] = a1.x;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckA1:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextA1Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Accum1Exit:
//
// Restore caller's a1
//
	a1.x = [sp++];
	a1.w = [sp++];
	
	unlink;
	rts;    

NextA1Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	a1.x = r0.l;
	
	a1.w = r0;
	
	r2 = a1.w;
				
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Accum1ContinueWithX;     // Yes try for another pattern to test against
	
Accum1Failure:
    r0 = [fp-4];                       // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( AccumFailure << TestFailurePos ) | A1Failure32; // Indicated error type to the caller
    jump.s Accum1Exit;  
                 
Accum1ContinueWithX:

	[fp-8] = r0;
	
	r2.l = a1.x;
	r2.h = 0;
	
	r0.l = 0xff;
	r0.h = 0;
	
	r2 = r0 & r2;
	
	r0 = [fp-8];
	
	[fp-8] = r2;
	
	r2.l = 0xff;
	
	r2.h = 0;
	
	r0 = r0 & r2;
	
	r2 = [fp-8];
	
	cc = r0 == r2;

	if cc jump Accum1Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                       // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( AccumFailure << TestFailurePos ) | A0Failure8; // Indicated error type to the caller
    jump.s Accum1Exit;  
	
Accum1Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckA1;
_BlackfinDiagAccum1Chk.end:

_BlackfinDiagRegIndexReg0Chk:
	link 4;
//
// Preserve Callers i0 register
//
 	[--sp] = i0;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckIdx0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextIdx0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Idx0Exit:
//
// Restore caller's i0
//
	i0 = [sp++];
	unlink;
	rts;    

NextIdx0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	i0 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = i0;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Idx0Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( IndexRegFailure << TestFailurePos ) | Idx0Failure; // Indicated error type to the caller
    
    
    jump.s Idx0Exit;               

Idx0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckIdx0;
_BlackfinDiagRegIndexReg0Chk.end:

_BlackfinDiagRegIndexReg1Chk:
	link 4;
//
// Preserve Callers i1 register
//
 	[--sp] = i1;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckIdx1:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextIdx1Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Idx1Exit:
//
// Restore caller's i1
//
	i1 = [sp++];
	unlink;
	rts;    

NextIdx1Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	i1 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = i1;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Idx1Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( IndexRegFailure << TestFailurePos ) | Idx1Failure; // Indicated error type to the caller
    
    
    jump.s Idx1Exit;               

Idx1Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckIdx1;
	
_BlackfinDiagRegIndexReg1Chk.end:

_BlackfinDiagRegIndexReg2Chk:
	link 4;
//
// Preserve Callers i2 register
//
 	[--sp] = i2;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckIdx2:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextIdx2Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Idx2Exit:
//
// Restore caller's i2
//
	i2 = [sp++];
	unlink;
	rts;    

NextIdx2Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	i2 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = i2;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Idx2Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( IndexRegFailure << TestFailurePos ) | Idx2Failure; // Indicated error type to the caller
    
    
    jump.s Idx2Exit;               

Idx2Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckIdx2;
_BlackfinDiagRegIndexReg2Chk.end:

_BlackfinDiagRegIndexReg3Chk:
	link 4;
//
// Preserve Callers i3 register
//
 	[--sp] = i3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckIdx3:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextIdx3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Idx3Exit:
//
// Restore caller's i3
//
	i3 = [sp++];
	unlink;
	rts;    

NextIdx3Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	i3 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = i3;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Idx3Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( IndexRegFailure << TestFailurePos ) | Idx3Failure; // Indicated error type to the caller
    
    
    jump.s Idx3Exit;               

Idx3Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckIdx3;
	
_BlackfinDiagRegIndexReg3Chk.end:

_BlackfinDiagRegModifyReg0Chk:
	link 4;
//
// Preserve Callers m0 register
//
 	[--sp] = m0;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckMdfy0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextMdfy0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Mdfy0Exit:
//
// Restore caller's m0
//
	m0 = [sp++];
	unlink;
	rts;    

NextMdfy0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                                // Load next pattern
	
	m0 = r0;
	
	nop;                                     // Assembler warning need extra cycles for read after right
	                                         // Could change the order but I think this is more indicative
	                                         // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = m0;
			
	cc = r0 == r2;                            // Patterns Match?
	
	if cc jump Mdfy0Continue;                 // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( ModifyRegFailure << TestFailurePos ) | Modify0Failure; // Indicated error type to the caller
    
    
    jump.s Mdfy0Exit;               

Mdfy0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckMdfy0;
_BlackfinDiagRegModifyReg0Chk.end:

_BlackfinDiagRegModifyReg1Chk:
	link 4;
//
// Preserve Callers m1 register
//
 	[--sp] = m1;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckMdfy1:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextMdfy1Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Mdfy1Exit:
//
// Restore caller's m1
//
	m1 = [sp++];
	unlink;
	rts;    

NextMdfy1Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	m1 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = m1;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Mdfy1Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( ModifyRegFailure << TestFailurePos ) | Modify1Failure; // Indicated error type to the caller
    
    
    
    jump.s Mdfy1Exit;               

Mdfy1Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckMdfy1;
_BlackfinDiagRegModifyReg1Chk.end:

_BlackfinDiagRegModifyReg2Chk:
	link 4;
//
// Preserve Callers m2 register
//
 	[--sp] = m2;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckMdfy2:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextMdfy2Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Mdfy2Exit:
//
// Restore caller's m2
//
	m2 = [sp++];
	unlink;
	rts;    

NextMdfy2Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                                // Load next pattern
	
	m2 = r0;
	
	nop;                                      // Assembler warning need extra cycles for read after right
	                                          // Could change the order but I think this is more indicative
	                                          // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = m2;
			
	cc = r0 == r2;                            // Patterns Match?
	
	if cc jump Mdfy2Continue;                 // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( ModifyRegFailure << TestFailurePos ) | Modify2Failure; // Indicated error type to the caller
    
    
    
    jump.s Mdfy2Exit;               

Mdfy2Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckMdfy2;
_BlackfinDiagRegModifyReg2Chk.end:

_BlackfinDiagRegModifyReg3Chk:
	link 4;
//
// Preserve Callers m3 register
//
 	[--sp] = m3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckMdfy3:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextMdfy3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Mdfy3Exit:
//
// Restore caller's i0
//
	m3 = [sp++];
	unlink;
	rts;    

NextMdfy3Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	m3 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = m3;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Mdfy3Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( ModifyRegFailure << TestFailurePos ) | Modify3Failure; // Indicated error type to the caller
    
    jump.s Mdfy3Exit;               

Mdfy3Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckMdfy3;
_BlackfinDiagRegModifyReg3Chk.end:

_BlackfinDiagRegLengthReg0Chk:
	link 4;
//
// Preserve Callers l0 register
//
 	[--sp] = l0;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckLength0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextLength0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Length0Exit:
//
// Restore caller's l0
//
	l0 = [sp++];
	unlink;
	rts;    

NextLength0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	l0 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = l0;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Length0Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( LengthRegFailure << TestFailurePos ) | Length0Failure; // Indicated error type to the caller
    
    
    jump.s Length0Exit;               

Length0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckLength0;
_BlackfinDiagRegLengthReg0Chk.end:

_BlackfinDiagRegLengthReg1Chk:
	link 4;
//
// Preserve Callers l1 register
//
 	[--sp] = l1;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckLength1:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextLength1Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Length1Exit:
//
// Restore caller's l1
//
	l1 = [sp++];
	unlink;
	rts;    

NextLength1Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	l1 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = l1;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Length1Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( LengthRegFailure << TestFailurePos ) | Length1Failure; // Indicated error type to the caller
    
    
    jump.s Length1Exit;               

Length1Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckLength1;
_BlackfinDiagRegLengthReg1Chk.end:

_BlackfinDiagRegLengthReg2Chk:
	link 4;
//
// Preserve Callers l2 register
//
 	[--sp] = l2;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckLength2:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextLength2Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Length2Exit:
//
// Restore caller's l2
//
	l2 = [sp++];
	unlink;
	rts;    

NextLength2Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	l2 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = l2;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Length2Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( LengthRegFailure << TestFailurePos ) | Length2Failure; // Indicated error type to the caller
    
    
    jump.s Length2Exit;               

Length2Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckLength2;
_BlackfinDiagRegLengthReg2Chk.end:

_BlackfinDiagRegLengthReg3Chk:
	link 4;
//
// Preserve Callers l3 register
//
 	[--sp] = l3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckLength3:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextLength3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Length3Exit:
//
// Restore caller's l3
//
	l3 = [sp++];
	unlink;
	rts;    

NextLength3Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	l3 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = l3;
			
	cc = r0 == r2;                            // Patterns Match?
	
	if cc jump Length3Continue;               // Yes try for another pattern to test against
	
    r0 = [fp-4];                              // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( LengthRegFailure << TestFailurePos ) | Length3Failure; // Indicated error type to the caller
    
    
    jump.s Length3Exit;               

Length3Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckLength3;
_BlackfinDiagRegLengthReg3Chk.end:

_BlackfinDiagRegBaseReg0Chk:
	link 4;
//
// Preserve Callers l0 register
//
 	[--sp] = b0;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckBase0:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextBase0Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Base0Exit:
//
// Restore caller's b0
//
	b0 = [sp++];
	unlink;
	rts;    

NextBase0Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	b0 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = b0;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Base0Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( BaseRegFailure << TestFailurePos ) | Base0Failure; // Indicated error type to the caller
    
    
    jump.s Base0Exit;               

Base0Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckBase0;
_BlackfinDiagRegBaseReg0Chk.end:

_BlackfinDiagRegBaseReg1Chk:
	link 4;
//
// Preserve Callers b1 register
//
 	[--sp] = b1;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckBase1:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextBase1Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Base1Exit:
//
// Restore caller's b1
//
	b1 = [sp++];
	unlink;
	rts;    

NextBase1Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	b1 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = b1;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Base1Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( BaseRegFailure << TestFailurePos ) | Base1Failure; // Indicated error type to the caller
    
    
    jump.s Base1Exit;               

Base1Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckBase1;
_BlackfinDiagRegBaseReg1Chk.end:

_BlackfinDiagRegBaseReg2Chk:
	link 4;
//
// Preserve Callers b2 register
//
 	[--sp] = b2;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckBase2:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextBase2Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Base2Exit:
//
// Restore caller's b2
//
	b2 = [sp++];
	unlink;
	rts;    

NextBase2Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	b2 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = b2;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Base2Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( BaseRegFailure << TestFailurePos ) | Base2Failure; // Indicated error type to the caller
    
    
    jump.s Base2Exit;               

Base2Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckBase2;
_BlackfinDiagRegBaseReg2Chk.end:

_BlackfinDiagRegBaseReg3Chk:
	link 4;
//
// Preserve Callers b3 register
//
 	[--sp] = b3;
    
//
// r0 == pointer to array of patterns to try.
//
// r1 == number of patterns;
//
// r2 == current patter index iterations
//
	p1 = r0;
		
	r2 = 0;
	
CheckBase3:
    [fp-4] = r2;
    	
	cc = r2 < r1 (iu);
	
	if cc jump NextBase3Pattern;  // More patterns to try
	
	r0 = 0;                    // 0 indicates no errors
							   // No more patterns to try		
Base3Exit:
//
// Restore caller's b3
//
	b3 = [sp++];
	unlink;
	rts;    

NextBase3Pattern:
	[fp-4] = r2;
	
	r0 = [p1];                 // Load next pattern
	
	b3 = r0;
	
	nop;                       // Assembler warning need extra cycles for read after right
	                           // Could change the order but I think this is more indicative
	                           // of the requirement.
	
	nop;
	
	nop;
	
	nop;

	r2 = b3;
			
	cc = r0 == r2;             // Patterns Match?
	
	if cc jump Base3Continue;     // Yes try for another pattern to test against
	
    r0 = [fp-4];                          // Put failure pattern idx in upper 16 bits  	
    r0 <<= TestIdxBitPos;
    r0.l = ( BaseRegFailure << TestFailurePos ) | Base3Failure; // Indicated error type to the caller
    
    
    jump.s Length3Exit;               

Base3Continue:
	r2 = [fp-4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump.s CheckBase3;
_BlackfinDiagRegBaseReg3Chk.end:
.GLOBAL _BlackfinDiagRegSanityChk;

.GLOBAL _BlackfinDiagRegDataReg7Chk;
.GLOBAL _BlackfinDiagRegDataReg6Chk;
.GLOBAL _BlackfinDiagRegDataReg5Chk;
.GLOBAL _BlackfinDiagRegDataReg4Chk;
.GLOBAL _BlackfinDiagRegDataReg3Chk;
.GLOBAL _BlackfinDiagRegPointerReg5Chk;
.GLOBAL _BlackfinDiagRegPointerReg4Chk;
.GLOBAL _BlackfinDiagRegPointerReg3Chk;
.GLOBAL _BlackfinDiagRegPointerReg2Chk;
.GLOBAL _BlackfinDiagRegPointerReg0Chk;
.GLOBAL _BlackfinDiagAccum0Chk;
.GLOBAL _BlackfinDiagAccum1Chk;
.GLOBAL _BlackfinDiagRegModifyReg3Chk;
.GLOBAL _BlackfinDiagRegModifyReg2Chk;
.GLOBAL _BlackfinDiagRegModifyReg1Chk;
.GLOBAL _BlackfinDiagRegModifyReg0Chk;
.GLOBAL _BlackfinDiagRegLengthReg3Chk;
.GLOBAL _BlackfinDiagRegLengthReg2Chk;
.GLOBAL _BlackfinDiagRegLengthReg1Chk;
.GLOBAL _BlackfinDiagRegLengthReg0Chk;
.GLOBAL _BlackfinDiagRegIndexReg3Chk;
.GLOBAL _BlackfinDiagRegIndexReg2Chk;
.GLOBAL _BlackfinDiagRegIndexReg1Chk;
.GLOBAL _BlackfinDiagRegIndexReg0Chk;
.GLOBAL _BlackfinDiagRegBaseReg3Chk;
.GLOBAL _BlackfinDiagRegBaseReg2Chk;
.GLOBAL _BlackfinDiagRegBaseReg1Chk;
.GLOBAL _BlackfinDiagRegBaseReg0Chk;
