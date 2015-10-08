#pragma once
#include "BlackfinDiag.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagDataRam : public BlackfinDiagTest {

public:
	BlackfinDiagDataRam() : 
		BlackfinDiagTest(PeriodPerTestIteration_Milleseconds), 
		pCurrentRamAddress(const_cast<UINT8 *>(pRAMDataStart)),
		pFirstRamAddressToTest(const_cast<UINT8 *>(pRAMDataStart)),
		NumberOfBytesTested(0),
		RamTestPatterns(TestPatternsForRamTesting),
		NumberOfTestPatterns(NumberOfTestingPatterns)
	{}

	virtual ~BlackfinDiagDataRam() {}

	virtual TestState RunTest();

private:
	const UINT8 * RamTestPatterns;

	const UINT32 NumberOfTestPatterns;

	UINT8 * pFirstRamAddressToTest;

	static const UINT32 NumberOfBytesToTestPerIteration = 0x400;

	static const UINT32 TotalNumberOfRamBytesToTest = 0x10000;
	
	static const UINT32 PeriodPerTestIteration_Milleseconds = 500;

	UINT8 * pCurrentRamAddress;

	UINT32  NumberOfBytesTested;

	INT                           Critical;                    /* Temp to allow disabling interrupts around critical sections */

	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}

	BOOL TestAByte(UINT8 * pByteToTest, UINT8 & PatternThatFailed);

	BOOL TestByteForAllTestPatterns(UINT8 * pByteToTest, UINT8 & PatternThatFailed);
};


