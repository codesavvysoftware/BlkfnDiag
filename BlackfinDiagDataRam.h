#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagDataRam : public BlackfinDiagTest {

public:
	BlackfinDiagDataRam( BlackfinDataRamTestSuite * DataRamTestSuite;
	                     const UINT8 *              TestPatternsForRamTesting,
	                     UINT32                     NumberOfRamTestingPatterns,
	                     DiagnosticTestTypes TestType = DiagDataRamTestType   ) 
	                     : 	DataRamTestSuite       ( DataRamTestSuite ),
	                        BlackfinDiagTest       ( TestType, PeriodPerTestIteration_Milleseconds ), 
	                     	pCurrentRamAddress     ( pStartRamAddress ),
							pFirstRamAddressToTest ( pStartRamAddress),
							NumberOfBytesTested    ( 0 ),
							RamTestPatterns        ( TestPatternsForRamTesting ),
							NumberOfTestPatterns   ( NumberOfRamTestingPatterns )
	{}

	virtual ~BlackfinDiagDataRam() {}

	virtual TestState RunTest(UINT32 & ErrorCode, DiagTime_t TimeTestStarted_microseconds = GetSystemTime()  );

private:
	static const UINT32        NumberOfBytesToTestPerIteration     = 0x400;

	static const UINT32        TotalNumberOfRamBytesToTest         = 0x10000;
	
	static const UINT32        PeriodPerTestIteration_Milleseconds = 500;

	const UINT8 *              RamTestPatterns;

	UINT32                     NumberOfTestPatterns;
	
	BlackfinDataRamTestSuite * DataRamTestSuite;

	INT                        Critical;                    

	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}

	BOOL TestAByte(UINT8 * pByteToTest, UINT8 & PatternThatFailed);

	BOOL TestByteForAllTestPatterns(UINT8 * pByteToTest, UINT8 & PatternThatFailed);
};


