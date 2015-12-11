//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDataRamTestByte.s
///
/// Blackfin assembly language subroutine to test one byte of RAM. Called from BlackfinDataRam.cpp
///
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <asm_sprt.h>                              /* Added for ssync( ), cli/sti( ) */

	.section/DOUBLEANY program;
	.align 4;

_TestAByteOfRam:
	link 0;
	//
	// Since we're testing RAM we want to work with registers only.
	//
	[--sp] = r4;			// Save non scratchpad registers
	[--sp] = r3;
	[--sp] = p3;
	[--sp] = p2;
	//
	// Pointer to following structure is passed in r1.
	//
	//   	typedef struct {
	//		      UINT8  * pByteToTest;
	//		      UINT8  * pPatternThatFailed;
	//		const UINT8  * pTestPatterns;
	//		      UINT32   NumberOfTestPatterns;
	//	} ByteTestParameters;
	//
	p0 = r1;				// Pointer to struct in p0 for indirect addressing
	p1 = [p0];              // p1 = pByteToTest
	p2 = [p0+4];        	// p2 = pPatternThatFailed, returned to caller upon failure
	p3 = [p0+8];			// p3 = pTestPatterns
	r2 = [p0+12];			// r2 = NumberOfTestPatters
	
	r1 = B[p1];				// Save actual memory contents in r1
	r3 = 0;             	// Loop counter for looping through test patterns
CheckForMoreTestPatterns:
	cc = r3 < r2;				// Are we finished checking all the patterns?
	if cc jump NextPattern;	// No do the next test pattern
	r0 = 1; 				// Finished with all the patterns, no errors found, return TRUE
Exit:
       		
	B [p1] = r1;			// Restore orignal memory contents
	
	p2 = [sp++];			// Save non scratchpad registers
	p3 = [sp++];
	r3 = [sp++];
	r4 = [sp++];
	unlink;
	rts;
NextPattern:
	r0 = B [p3];            // Next test pattern in r0
	B [p1] = r0;
	nop;
	nop;
	nop;
	nop;                    // Save test pattern in memory location being tested.
	r4 = B [p1];            // Read it back
	cc = r0 == r4;          // Did it read back correctly ?
	if cc jump PrepareForNextPattern;
	B [p2] = r0;     		// Save test pattern that failed
	r0 = 0;                 // Indicate an error to the caller
	jump.s	Exit;			// Finish up
PrepareForNextPattern:
	p3 += 1;				// Point to next pattern
	r3 += 1;				// Increment the number of patterns tested.
	jump.s CheckForMoreTestPatterns;
_TestAByteOfRam.end:
.GLOBAL _TestAByteOfRam;
	 
	
	