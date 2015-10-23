#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


using namespace DiagnosticCommon;

class BlackfinDiagRegistersTest : public BlackfinDiagTest {

public:
	BlackfinDiagRegistersTest( 	const RegisterTestDescriptor    RegisterTestSuite[],
	                            UINT32                            NumberOfDescriptorsInTestSuite, 
								const UINT32 *                    TestPatterns, 
								UINT32                            NumberOfPatterns,
								DiagnosticTestTypes               TestType = DiagRegisterTestType ) 
								:
									TestPatternsForRegisterTesting ( TestPatterns ),
									NumberOfRegisterTests(NumberOfDescriptorsInTestSuite),
									NumberOfRegisterPatterns       ( NumberOfPatterns ), 
									BlackfinDiagTest               ( 
																	 TestType, 
																	 PeriodPerTestIteration_Milleseconds, 
																	 ScheduledTime_Milleseconds 
																   ),
									RegisterTestSuite              ( RegisterTestSuite ) 
	{}

	virtual ~BlackfinDiagRegistersTest() {}

	virtual TestState RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime = GetSystemTime() );

protected:

	virtual void ConfigureForNextTestCycle();
	
private:
	static const DiagTime_t	PeriodPerTestIteration_Milleseconds = 50;
	
	static const DiagTime_t	ScheduledTime_Milleseconds          = 0;	

	static const UINT32 CorruptedRegisterTestSuite = 0xff;

    const UINT32 *                    TestPatternsForRegisterTesting;
    
    const UINT32                      NumberOfRegisterPatterns;

	const RegisterTestDescriptor *    RegisterTestSuite;
	
	const UINT32                      NumberOfRegisterTests;
	
	INT                           Critical;                    /* Temp to allow disabling interrupts around critical sections */

	BOOL FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & rtdTests );
	
	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}
    
    BOOL RunRegisterTests( RegisterTestDescriptor  * rtdTests, UINT32 & FailureInfo );
};


