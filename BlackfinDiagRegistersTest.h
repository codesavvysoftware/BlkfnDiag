#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagRegistersTest : public BlackfinDiagTest {

public:
	BlackfinDiagRegistersTest( 	const BlackfinRegisterTestSuite * RegisterTestSuite, 
								const UINT32 *                    TestPatterns, 
								const UINT32                      NumberOfPatterns,
								DiagnosticTestTypes               TestType = DiagRegisterTestType ) 
								:
									TestPatternsForRegisterTesting ( TestPatterns ),
									NumberOfRegisterPatterns       ( NumberOfPatterns ), 
									BlackfinDiagTest               ( TestType, PeriodPerTestIteration_Milleseconds ),
									RegisterTestSuite              ( RegisterTestSuite ) 
	{}

	virtual ~BlackfinDiagRegistersTest() {}

	virtual TestState RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime = GetSystemTime() );

private:
	static const UINT32 PeriodPerTestIteration_Milleseconds = 500;

    const UINT32 * TestPatternsForRegisterTesting;
    
    const UINT32 NumberOfRegisterPatterns;

	const BlackfinRegisterTestSuite * RegisterTestSuite;
	
	RegisterFailureData    rfdCurrentFailure;
	
	INT                           Critical;                    /* Temp to allow disabling interrupts around critical sections */

	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}
    
    TestState RunRegisterTests( RegisterTestDescriptor  rtdTests);
};


