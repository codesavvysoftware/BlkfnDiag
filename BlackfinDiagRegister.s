#include <asm_sprt.h>                              /* Added for ssync( ), cli/sti( ) */

#define AllOnesPattern       0xffff
#define AllZerosPattern      0
#define AlternatingOnesZeros 0xaaaa
#define AlternatingZerosOnes 0x5555

#define FirstSanityCheckFail        1
#define SecondSanityCheckFail       2
#define ThirdSanityCheckFail        3
#define FourthSanityCheckFail       4
#define FifthSanityCheckFail        5
#define SixthSanityCheckFail        6
#define SeventhSanityCheckFail      7
#define EighthSanityCheckFail       8
#define NinethSanityCheckFail       9
#define TenthSanityCheckFail       10
#define EleventhSanityCheckFail    11
#define TwelthSanityCheckFail      12
#define ThirteenthSanityCheckFail  13
#define FourteenthSanityCheckFail  14
#define FifteenthSanityCheckFail   15
#define SixteenthSanityCheckFail   16
#define DataRegFailure             1
#define PointerRegFailure          2
#define R7Failure                  4
#define R6Failure                  3
#define R5Failure                  2
#define R4Failure                  1
#define R3Failure                  0
#define P5Failure                  5    
#define P4Failure                  4    
#define P3Failure                  3    
#define P2Failure                  2    
#define P0Failure                  0    

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
    if cc jump FirstSanityCheckPassed;
    //
    // First Sanity Check Failure Indicate as such upon return.
    //
    r0 = FirstSanityCheckFail;
    jump Exit;

FirstSanityCheckPassed:
	r2.h = AllOnesPattern;
	r2.l = AllOnesPattern;
	
    cc = r0 == r2;
    if cc jump SecondSanityCheckPassed;
    //
    // Second Sanity Check Failure Indicate as such upon return.
    //
    r0 = SecondSanityCheckFail;
    jump Exit;

SecondSanityCheckPassed:
    cc = r1 == r2;
    if cc jump ThirdSanityCheckPassed;
    //
    // Third Sanity Check Failure Indicate as such upon return.
    //
    r0 = ThirdSanityCheckFail;
    jump Exit;

ThirdSanityCheckPassed:
    r0.h = AllZerosPattern;
    r0.l = AllZerosPattern;  // All zeros
    r1.h = AllZerosPattern;
    r1.l = AllZerosPattern;
    cc = r0 == r1;
    if cc jump FourthSanityCheckPassed;
    //
    // Fourth Sanity Check Failure Indicate as such upon return.
    //
    r0 = FourthSanityCheckFail;
    jump Exit;

FourthSanityCheckPassed:
	r2.h = AllZerosPattern;
	r2.l = AllZerosPattern;
    cc = r0 == r2;
    if cc jump FifthSanityCheckPassed;
    //
    // Fifth Sanity Check Failure Indicate as such upon return.
    //
    r0 = FifthSanityCheckFail;
    jump Exit;

FifthSanityCheckPassed:
    cc = r1 == r2;
    if cc jump SixthSanityCheckPassed;
    //
    // Sixth Sanity Check Failure Indicate as such upon return.
    //
    r0 = SixthSanityCheckFail;
Exit:
    unlink;
    rts;
SixthSanityCheckPassed:
    r0.h = AlternatingOnesZeros;
    r0.l = AlternatingOnesZeros;  // Alternating ones, zeros
    r1.h = AlternatingOnesZeros;
    r1.l = AlternatingOnesZeros;
    cc = r0 == r1;
    if cc jump SeventhSanityCheckPassed;
    //
    // Seventh Sanity Check Failure Indicate as such upon return.
    //
    r0 = SeventhSanityCheckFail;
    jump Exit;

SeventhSanityCheckPassed:
	r2.h = AlternatingOnesZeros;
	r2.l = AlternatingOnesZeros;
    cc = r0 == r2;
    if cc jump EighthSanityCheckPassed;
    //
    // Eighth Sanity Check Failure Indicate as such upon return.
    //
    r0 = EighthSanityCheckFail;
    jump Exit;

EighthSanityCheckPassed:
    cc = r1 == r2;
    if cc jump NinethSanityCheckPassed;
    //
    // Nineth Sanity Check Failure Indicate as such upon return.
    //
    r0 = NinethSanityCheckFail;
    jump Exit;
NinethSanityCheckPassed:
    r0.h = AlternatingZerosOnes;
    r0.l = AlternatingZerosOnes;  // Alternating ones, zeros
    r1.h = AlternatingZerosOnes;
    r1.l = AlternatingZerosOnes;
    cc = r0 == r1;
    if cc jump TenthSanityCheckPassed;
    //
    // Tenth Sanity Check Failure Indicate as such upon return.
    //
    r0 = TenthSanityCheckFail;
    jump Exit;

TenthSanityCheckPassed:
	r2.h = AlternatingZerosOnes;
	r2.l = AlternatingZerosOnes;
    cc = r0 == r2;
    if cc jump EleventhSanityCheckPassed;
    //
    // Eleventh Sanity Check Failure Indicate as such upon return.
    //
    r0 = EleventhSanityCheckFail;
    jump Exit;

EleventhSanityCheckPassed:
    cc = r1 == r2;
    if cc jump TwelthSanityCheckPassed;
    //
    // Twelth Sanity Check Failure Indicate as such upon return.
    //
    r0 = TwelthSanityCheckFail;
    jump Exit;
TwelthSanityCheckPassed:
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
    if cc jump ThirteenthSanityCheckPassed;
    //
    // Thirteenth Sanity Check Failure Indicate as such upon return.
    //
    r0 = ThirteenthSanityCheckFail;
    jump Exit;
ThirteenthSanityCheckPassed:
    r0.h = AllZerosPattern;
    r0.l = AllZerosPattern;
    p1.h = AllZerosPattern;
    p1.l = AllZerosPattern;
    r1 = p1;
    cc = r0 == r1;
    if cc jump FourteenthSanityCheckPassed;
    //
    // Fourteenth Sanity Check Failure Indicate as such upon return.
    //
    r0 = FourteenthSanityCheckFail;
    jump Exit;
FourteenthSanityCheckPassed:
    r0.h = AlternatingOnesZeros;
    r0.l = AlternatingOnesZeros;
    p1.h = AlternatingOnesZeros;
    p1.l = AlternatingOnesZeros;
    r1 = p1;
    cc = r0 == r1;
    if cc jump FifteenthSanityCheckPassed;
    //
    // Fifteenth Sanity Check Failure Indicate as such upon return.
    //
    r0 = FifteenthSanityCheckFail;
    jump Exit;
FifteenthSanityCheckPassed:
    r0.h = AlternatingZerosOnes;
    r0.l = AlternatingZerosOnes;
    p1.h = AlternatingZerosOnes;
    p1.l = AlternatingZerosOnes;
    r1 = p1;
    cc = r0 == r1;
    if cc jump SixteenthSanityCheckPassed;
    //
    // Sixteenth Sanity Check Failure Indicate as such upon return.
    //
    r0 = SixteenthSanityCheckFail;
    jump Exit;
SixteenthSanityCheckPassed:
    r0 = 0;              // Test passed
    jump Exit;

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
	
    r0.h = DataRegFailure;     // No
    r0.l = R7Failure;          // Indicated error type to the caller
    
    jump R7Exit;               

R7Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump CheckR7;

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
	
    r0.h = DataRegFailure;     // No
    r0.l = R6Failure;          // Indicated error type to the caller
    
    jump R6Exit;               

R6Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump CheckR6;

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
	
    r0.h = DataRegFailure;     // No
    r0.l = R5Failure;          // Indicated error type to the caller
    
    jump R5Exit;               

R5Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump CheckR5;

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
	
    r0.h = DataRegFailure;     // No
    r0.l = R4Failure;          // Indicated error type to the caller
    
    jump R6Exit;               

R4Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump CheckR4;

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
	
    r0.h = DataRegFailure;     // No
    r0.l = R3Failure;          // Indicated error type to the caller
    
    jump R6Exit;               

R3Continue:
	r2 += 1;
	
	p1 += 4;
	
	jump CheckR3;

_BlackfinDiagRegPointerReg5Chk:
	link 8;
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
    [fp+4] = r2;
    	
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
	[fp+4] = r2;
	
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
	
    r0.h = PointerRegFailure;  // No
    r0.l = P5Failure;          // Indicated error type to the caller
    
    jump P5Exit;               

P5Continue:
	r2 = [fp+4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump CheckP5;

_BlackfinDiagRegPointerReg4Chk:
	link 8;
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
    [fp+4] = r2;
    	
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
	[fp+4] = r2;
	
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
	
    r0.h = PointerRegFailure;  // No
    r0.l = P4Failure;          // Indicated error type to the caller
    
    jump P4Exit;               

P4Continue:
	r2 = [fp+4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump CheckP4;

_BlackfinDiagRegPointerReg3Chk:
	link 8;
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
    [fp+4] = r2;
    	
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
	[fp+4] = r2;
	
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
	
    r0.h = PointerRegFailure;  // No
    r0.l = P3Failure;          // Indicated error type to the caller
    
    jump P3Exit;               

P3Continue:
	r2 = [fp+4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump CheckP3;

_BlackfinDiagRegPointerReg2Chk:
	link 8;
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
    [fp+4] = r2;
    	
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
	[fp+4] = r2;
	
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
	
    r0.h = PointerRegFailure;  // No
    r0.l = P2Failure;          // Indicated error type to the caller
    
    jump P2Exit;               

P2Continue:
	r2 = [fp+4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump CheckP2;

_BlackfinDiagRegPointerReg0Chk:
	link 8;
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
    [fp+4] = r2;
    	
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
	[fp+4] = r2;
	
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
	
    r0.h = PointerRegFailure;  // No
    r0.l = P0Failure;          // Indicated error type to the caller
    
    jump P0Exit;               

P0Continue:
	r2 = [fp+4];
	
	r2 += 1;
	
	p1 += 4;
	
	jump CheckP0;

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
