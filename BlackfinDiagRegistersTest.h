#pragma once
#include "BlackfinDiag.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagRegistersTest : public BlackfinDiagTest {

public:
	BlackfinDiagRegistersTest() : 
		BlackfinDiagTest(PeriodPerTestIteration_Milleseconds){}

	virtual ~BlackfinDiagRegistersTest() {}

	virtual TestState RunTest();

private:
	static const UINT32 PeriodPerTestIteration_Milleseconds = 500;

	RegisterFailureData    rfdCurrentFailure;
	
	INT                           Critical;                    /* Temp to allow disabling interrupts around critical sections */

    void DecodeFailureResult( UINT32 Result, RegisterFailureData & rfdDecodedData );
    
	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}
    
	TestState RunRegisterTests( REGISTER_TEST * RegTestArray , const UINT32 NumberOfRegisterTests );
};


