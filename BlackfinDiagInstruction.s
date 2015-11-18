#include <asm_sprt.h>                              /* Added for ssync( ), cli/sti( ) */
#include "BlackfinDiagRegTestCommon.hpp"

	.section/DOUBLEANY program;
	.align 4;

_BlackfinDiagInstrTest:
	link 0;

	//
	// Test zero overhead loop instructions
	//
	r0 = 1;
	
	r2 = 8;
	
	p5 = r2;
	
	// loop 8 times, 2**8
	
    nop;
    
    nop;
    
    nop;
    
    nop;
    
    loop lc0_test lc0 = p5;
	
	loop_begin lc0_test;
	
	r0 <<= 1;
	
	loop_end lc0_test;
	
	r1 = 256;
	
	cc = r0 == r1;
	
	if !cc jump TestError;
	
	r0 = 1;
	
	r2 = 8;
	
	p5 = r2;
	
	nop;
	
	nop;
	
	nop;
	
	nop;
	
	// loop 8 times, 2**8
	
	loop lc1_test lc1 = p5;
	
	loop_begin lc1_test;
	
	r0 <<= 1;
	
	loop_end lc1_test;
	
	r1 = 256;
	
	cc = r0 == r1;
	
	if !cc jump TestError;

    // Test moving just data registers into cc
    
    r0 = 0;
    
    cc = r0;
    
    if cc jump TestError;
    
    r0 = 1;
    
    cc = r0;
    
    if !cc jump TestError;
    
    // Test logical operations
    r0.l = 0xaaaa;
    
    r0.h = 0xaaaa;
    
    r1.l = 0x5555;
    
    r1.h = 0x5555;
    
    r2 = r0 ^ r1;
    
    r3 = 0xffffffff;
    
    cc = r2 == r3;
    
    if !cc jump TestError;
    
    r2 = r2 ^ r3;
    
    cc = r2;
    
    if cc jump TestError;
    
    r2 = r1 ^ r3;
    
    cc = r0 == r2;
    
    if !cc jump TestError;
    
    r2 = r0 ^ r3;
    
    cc = r1 == r2;
    
    if !cc jump TestError;
    
    r2 = r0 | r1;
    
    cc = r2 == r3;
    
    if !cc jump TestError;
    
    r2 = 0;
    
    r2 = r0 | r2;
    
    cc = r0 == r2;
    
    if !cc jump TestError;
    
    r2 = r0 & r1;
    
    cc = r2;
    
    if cc jump TestError;
    
    r2 = r0 & r3;
    
    cc = r0 == r2;
    
    if !cc jump TestError;

    // Test bit field extraction
    r0.l = 0xaaaa;
    
    r0.h = 0xaaaa;
    
    r4 = r0;
    
    r2.l = 0x5555;
    
    r2.h = 0x5555;
    
    // Change 0xaaaaaaaa to 0x55555555
    r1.l = 0x11f;
    
    r3 = extract (r0, r1.l) (z);
    
    cc = r3 == r2;
    
    if !cc jump TestError;
    
    // Chage 0xaaaaaaaa to 0xffffffff
    
    r2.l = 0xffff;
    
    r2.h = 0xffff;
    
    r0 = r4;
    
    r1.l = 0x101;
    
    r3 = extract( r0, r1.l) (x);
	
    cc = r3 == r2;
    
    if !cc jump TestError;
    
    // Bit instructions
    
    r0 = r4;
    
    bittgl (r0, 31);
    
    r1.h = 0x2aaa;
    
    r1.l = 0xaaaa;
    
    cc = r0 == r1;
    
    if !cc jump TestError;
    
    bittgl (r1,31);
    
    cc = r1 == r4;
    
    if !cc jump TestError;
    
    r0 = r4;
    
    bitclr (r0, 1);
    
    r1 = r4;
    
    bittgl (r1, 1);
    
    cc = r0 == r1;
    
    if !cc jump TestError;
    
    bitset (r0, 1);
    
    r1 = r4;
    
    cc = r0 == r1;
    
    if !cc jump TestError;
    
    bittgl (r0, 14);
    
    bitset (r1, 14);
    
    cc = r0 == r1;
    
    if !cc jump TestError;
    
    bittgl (r1,14);
    
    bitclr (r0,14);
    
    cc = r0 == r4;
    
    if !cc jump TestError;
    
    cc = r1 == r4;
    
    if !cc jump TestError;
    
    // ashift - make sure sign extension works ok
    
    // Start with right shifts
    
    r2.l = -15;
    
    r0.h = 0x8000;
    
    r0.l = 0x4000;
    
    r4 = r0;
    
    r1.h = ashift r0.h by r2.l;
    
    r2.l = -14;
    
    r1.l = ashift r0.l by r2.l;
    
    r3.h = 0xffff;
    
    r3.l = 0x1;
    
    cc = r1 == r3;
    
    if !cc jump TestError;
    
    // Now left shifts
    r2.l = 15;
    
    r0.h = 0x1;
    
    r0.l = 0x1;
    
    r1.h = ashift r0.h by r2.l;
    
    r2.l = 14;
    
    r1.l = ashift r0.l by r2.l;
    
    r3 = r4;
        
    cc = r1 == r3;
    
    if !cc jump TestError;
       
	r0 = 0;
	
Exit:

    unlink;
    
    rts;
    
TestError:

	r0 <<= TestIdxBitPos;
 	
    r0.l = ( DataRegFailure << TestFailurePos ); // Indicated error type to the caller

    r0 = r0 | r2;
    
    jump.s Exit;
_BlackfinDiagInstrTest.end:
    



.GLOBAL _BlackfinDiagInstrTest;
	