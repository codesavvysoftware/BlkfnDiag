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

	
_BlackfinDiagRegChk:
	link 0;
//
// Preserve Registers 
//
// 	[--sp] = r7;
 	
    [--sp] = (r7:3, p5:0 );
    
    [--sp] = b3;
    [--sp] = b2;
    [--sp] = b1;
    [--sp] = b0;
    
    [--sp] = i3;
    [--sp] = i2;
    [--sp] = i1;
    [--sp] = i0;
    
    [--sp] = l3;
    [--sp] = l2;
    [--sp] = l1;
    [--sp] = l0;
    
    [--sp] = m3;
    [--sp] = m2;
    [--sp] = m1;
    [--sp] = m0;
    
 	[--sp] = a0.w;
 	[--sp] = a0.x;
 	[--sp] = a1.w;
 	[--sp] = a1.x;
 	
 	r0 = 0xffffffff;
 	
    call _BlackfinDiagRegTestPattern;
    
    r1 = 0;
    
    cc = r0 == r1;
    
    if cc jump NextPattern;
    
ExitRegTest:
    a1.x = [sp++];
    a1.w = [sp++];
    a0.x = [sp++];
    a0.w = [sp++];

    m0 = [sp++];
    m1 = [sp++];
    m2 = [sp++];
    m3 = [sp++];

    l0 = [sp++];
    l1 = [sp++];
    l2 = [sp++];
    l3 = [sp++];

    i0 = [sp++];
    i1 = [sp++];
    i2 = [sp++];
    i3 = [sp++];

    b0 = [sp++];
    b1 = [sp++];
    b2 = [sp++];
    b3 = [sp++];

    
    (r7:3, p5:0 ) = [sp++];
    
    unlink;
    
    rts;

NextPattern:

 	r0 = 0;
 	
    call _BlackfinDiagRegTestPattern;
    
    jump.s ExitRegTest;
    
_BlackfinDiagRegChk.end:

_BlackfinDiagRegTestPattern:

    link 0;
    
    r3 = r0;
    
    r4 = r0;
    
    r4 = r0;
    
    r6 = r0;
    
    r7 = r0;    

    p5 = r0;
    
    p4 = r0;
    
    p3 = r0;
    
    p2 = r0;
    
    p0 = r0;
    
    m3 = r0;
    
    m2 = r0;
    
    m1 = r0;
    
    m0 = r0;
    
    l3 = r0;
    
    l2 = r0;
    
    l1 = r0;
    
    l0 = r0;
    
    i3 = r0;
    
    i2 = r0;
    
    i1 = r0;
    
    i0 = r0;
    
    b3 = r0;
    
    b2 = r0;
    
    b1 = r0;
    
    b0 = r0;
    
   	r2 = R7Failure;
    
 	cc = r0 == r7;
 	
 	if cc jump R7Continue;
 	
TestError:

	r0 <<= TestIdxBitPos;
 	
    r0.l = ( DataRegFailure << TestFailurePos ); // Indicated error type to the caller

    r0 = r0 | r2;
 	
TestExit:

 	unlink;
 	
 	rts;

R7Continue:

   	r2 = R6Failure;
    
 	cc = r0 == r6;
 	
 	if cc jump R6Continue;
    
    jump.s TestError;
    
R6Continue:    
   	r2 = R5Failure;
    
 	cc = r0 == r5;
 	
 	if cc jump R5Continue;
    
    jump.s TestError;

R5Continue:    
   	r2 = R4Failure;
    
 	cc = r0 == r4;
 	
 	if cc jump R4Continue;
    
    jump.s TestError;

R4Continue:    
   	r2 = R3Failure;
    
 	cc = r0 == r3;
 	
 	if cc jump R3Continue;
    
    jump.s TestError;

R3Continue:    
    r3 = p0;
    
    r4 = p2;
    
    r5 = p3;
    
    r6 = p4;
    
    r7 = p5;
    
    r2 = P0Failure;
    
    cc = r0 == r3;
    
    if cc jump p0Continue;
    
    jump.s TestError;
    
p0Continue:

    r2 = P2Failure;
    
    cc = r0 == r4;
    
    if cc jump p2Continue;
    
    jump.s TestError;
    
p2Continue:

    r2 = P3Failure;
    
    cc = r0 == r5;
    
    if cc jump p3Continue;
    
    jump.s TestError;
    

p3Continue:

    r2 = P4Failure;
    
    cc = r0 == r6;
    
    if cc jump p4Continue;
    
    jump.s TestError;
    

p4Continue:

    r2 = P5Failure;
    
    cc = r0 == r7;
    
    if cc jump p2Continue;
    
    jump.s TestError;
    

p5Continue:

    r3 = m0;
    
    r4 = m1;
    
    r5 = m2;
    
    r6 = m3;
    
    r2 = Modify0Failure;
    
    cc = r0 == r3;
    
    if cc jump m0Continue;
    
    jump.s TestError;
    
m0Continue:

    r2 = Modify1Failure;
    
    cc = r0 == r4;
    
    if cc jump m1Continue;
    
    jump.s TestError;
    
m1Continue:

    r2 = Modify2Failure;
    
    cc = r0 == r5;
    
    if cc jump m2Continue;
    
    jump.s TestError;
    

m2Continue:

    r2 = Modify3Failure;
    
    cc = r0 == r6;
    
    if cc jump m3Continue;
    
    jump.s TestError;
    

m3Continue:
    
    r3 = l0;
    
    r4 = l1;
    
    r5 = l2;
    
    r6 = l3;
    
    r2 = Length0Failure;
    
    cc = r0 == r3;
    
    if cc jump l0Continue;
    
    jump.s TestError;
    
l0Continue:

    r2 = Length1Failure;
    
    cc = r0 == r4;
    
    if cc jump l1Continue;
    
    jump.s TestError;
    
l1Continue:

    r2 = Length2Failure;
    
    cc = r0 == r5;
    
    if cc jump l2Continue;
    
    jump.s TestError;
    

l2Continue:

    r2 = Length3Failure;
    
    cc = r0 == r6;
    
    if cc jump l3Continue;
    
    jump.s TestError;
    

l3Continue:

    r3 = i0;
    
    r4 = i1;
    
    r5 = i2;
    
    r6 = i3;
    
    r2 = Idx0Failure;
    
    cc = r0 == r3;
    
    if cc jump i0Continue;
    
    jump.s TestError;
    
i0Continue:

    r2 = Idx1Failure;
    
    cc = r0 == r4;
    
    if cc jump i1Continue;
    
    jump.s TestError;
    
i1Continue:

    r2 = Idx2Failure;
    
    cc = r0 == r5;
    
    if cc jump i2Continue;
    
    jump.s TestError;
    

i2Continue:

    r2 = Idx3Failure;
    
    cc = r0 == r6;
    
    if cc jump i3Continue;
    
    jump.s TestError;
    

i3Continue:

    r3 = b0;
    
    r4 = b1;
    
    r5 = b2;
    
    r6 = b3;
    
    r2 = Base0Failure;
    
    cc = r0 == r3;
    
    if cc jump b0Continue;
    
    jump.s TestError;
    
b0Continue:

    r2 = Base1Failure;
    
    cc = r0 == r4;
    
    if cc jump b1Continue;
    
    jump.s TestError;
    
b1Continue:

    r2 = Base2Failure;
    
    cc = r0 == r5;
    
    if cc jump b2Continue;
    
    jump.s TestError;
    

b2Continue:

    r2 = Base3Failure;
    
    cc = r0 == r6;
    
    if cc jump b3Continue;
    
    jump.s TestError;
    

b3Continue:

	r2 = A0Failure32;
	
	a0.x = r0.l;
	
	a0.w = r0;
	
	r3 = a0.w;
				
	cc = r0 == r3;             // Patterns Match?
	
	if cc jump A0Cont;
	
	jump.s TestError;
	
A0Cont:

    r2 = A0Failure8;
	     
    r3.l = a0.x;
    
    r4.h = 0;
    
    r4.l = 0xff;
    
    r3 = r3 & r4;
    
    r1 = r0 & r4;
    
    cc = r3 == r1;
    
    if cc jump A0Ok;
    
    jump.s TestError;
    
A0Ok:

	r2 = A1Failure32;
	
	a1.x = r0.l;
	
	a1.w = r0;
	
	r3 = a1.w;
				
	cc = r0 == r3;             // Patterns Match?
	
	if cc jump A1Cont;
	
	jump.s TestError;
	
A1Cont:

    r2 = A1Failure8;
	     
    r3.l = a1.x;
    
    r4.h = 0;
    
    r4.l = 0xff;
    
    r1 = r0 & r4;
    
    cc = r1 == r3;
    
    if cc jump A1Ok;
    
    jump.s TestError;
    
A1Ok:

    r0 = 0;
    
    jump.s  TestExit;

_BlackfinDiagTestPattern.end:
    



.GLOBAL _BlackfinDiagRegSanityChk;

.GLOBAL _BlackfinDiagRegChk;
