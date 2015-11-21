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
 	r0 = 0xffffffff;

 	call _BlackfinSanityChkPattern;
 	
    r1 = 0;
    
    cc = r0 == r1;
    
    if cc jump NextSanityPattern;
    
ExitSanityTest:

    unlink;
    
    rts;
NextSanityPattern:

    call _BlackfinSanityChkPattern;
    jump.s ExitSanityTest;
        
_BlackfinDiagRegSanityChk.end:

_BlackfinSanityChkPattern:
    link 0;
    r1 = r0;
    cc = r0 == r1;
    if cc jump R0R1Passed;
    //
    // First Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R1SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R1Passed:
	r2 = r0;
    cc = r0 == r2;
    if cc jump R0R2Passed;
    //
    // Second Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R0R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R0R2Passed:
    cc = r1 == r2;
    if cc jump R1R2Passed;
    //
    // Third Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | R1R2SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;

R1R2Passed:
    p0 = r0;

    nop;
    
    nop;
    
    nop;
    
    nop;
    
    r1 = p0;
    
    cc = r0 == r1;
    if cc jump P0Passed;
    //
    // Thirteenth Sanity Check Failure Indicate as such upon return.
    //
    r0.l = ( SanityCheckFailure << TestFailurePos ) | P0SanityCheckFail;
    // r0.h Already has the FailurePattern
    jump.s Exit;
P0Passed:
    r0 = 0;
Exit:
    unlink;
    
    rts;
            
_BlackfinSanityChkPattern.end:

	
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
    
    r5 = r0;
    
    r6 = r0;
    
    r7 = r0;    

    p5 = r0;
    
    p4 = r0;
    
    p3 = r0;
    
    p2 = r0;
    
    p1 = r0;
    
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
 	
 	if !cc jump TestError;
    
   	r2 = R5Failure;
    
 	cc = r0 == r5;
 	
 	if !cc jump TestError;
    
   	r2 = R4Failure;
    
 	cc = r0 == r4;
 	
 	if !cc jump TestError;
    
   	r2 = R3Failure;
    
 	cc = r0 == r3;
 	
 	if !cc jump TestError;
    
    r3 = p1;
    
    r4 = p2;
    
    r5 = p3;
    
    r6 = p4;
    
    r7 = p5;
    
    r2 = P1Failure;
    
    cc = r0 == r3;
    
    if !cc jump TestError;
    
    r2 = P2Failure;
    
    cc = r0 == r4;
    
    if !cc jump TestError;
    
    r2 = P3Failure;
    
    cc = r0 == r5;
    
    if !cc jump TestError;
    
    r2 = P4Failure;
    
    cc = r0 == r6;
    
    if !cc jump TestError;
    
    r2 = P5Failure;
    
    cc = r0 == r7;
    
    if !cc jump TestError;
    
    r3 = m0;
    
    r4 = m1;
    
    r5 = m2;
    
    r6 = m3;
    
    r2 = Modify0Failure;
    
    cc = r0 == r3;
    
    if !cc jump TestError;
    
    r2 = Modify1Failure;
    
    cc = r0 == r4;
    
    if !cc jump TestError;

    r2 = Modify2Failure;
    
    cc = r0 == r5;
    
    if !cc jump TestError;
    
    r2 = Modify3Failure;
    
    cc = r0 == r6;
    
    if !cc jump TestError;
    
    r3 = l0;
    
    r4 = l1;
    
    r5 = l2;
    
    r6 = l3;
    
    r2 = Length0Failure;
    
    cc = r0 == r3;
    
    if !cc jump TestError;
    
    r2 = Length1Failure;
    
    cc = r0 == r4;
    
    if !cc jump TestError;
    
    r2 = Length2Failure;
    
    cc = r0 == r5;
    
    if !cc jump TestError;
    
    r2 = Length3Failure;
    
    cc = r0 == r6;
    
    if !cc jump TestError;
    
    r3 = i0;
    
    r4 = i1;
    
    r5 = i2;
    
    r6 = i3;
    
    r2 = Idx0Failure;
    
    cc = r0 == r3;
    
    if !cc jump TestError;
    
    r2 = Idx1Failure;
    
    cc = r0 == r4;
    
    if !cc jump TestError;
    
    r2 = Idx2Failure;
    
    cc = r0 == r5;
    
    if !cc jump TestError;
    
    r2 = Idx3Failure;
    
    cc = r0 == r6;
    
    if !cc jump TestError;
    
    r3 = b0;
    
    r4 = b1;
    
    r5 = b2;
    
    r6 = b3;
    
    r2 = Base0Failure;
    
    cc = r0 == r3;
    
    if !cc jump TestError;
    
    r2 = Base1Failure;
    
    cc = r0 == r4;
    
    if !cc jump TestError;
    
    r2 = Base2Failure;
    
    cc = r0 == r5;
    
    if !cc jump TestError;
    
    r2 = Base3Failure;
    
    cc = r0 == r6;
    
    if !cc jump TestError;
    
	r2 = A0Failure32;
	
	a0.x = r0.l;
	
	a0.w = r0;
	
	r3 = a0.w;
				
	cc = r0 == r3;             // Patterns Match?
	
    if !cc jump TestError;
    
    r2 = A0Failure8;
	     
    r3.l = a0.x;
    
    r4.h = 0;
    
    r4.l = 0xff;
    
    r3 = r3 & r4;
    
    r1 = r0 & r4;
    
    cc = r3 == r1;
    
    if !cc jump TestError;
    
	r2 = A1Failure32;
	
	a1.x = r0.l;
	
	a1.w = r0;
	
	r3 = a1.w;
				
	cc = r0 == r3;             // Patterns Match?
	
    if !cc jump TestError;
    
    r2 = A1Failure8;
	     
    r3.l = a1.x;
    
    r3 = r3 & r4;
    
    r1 = r0 & r4;
    
    cc = r1 == r3;
    
    if !cc jump TestError;
    
    r0 = 0;
    
    jump.s  TestExit;

_BlackfinDiagTestPattern.end:
    



.GLOBAL _BlackfinDiagRegSanityChk;

.GLOBAL _BlackfinDiagRegChk;
