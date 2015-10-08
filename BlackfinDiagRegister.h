#pragma once

#include "BlackfinDiag.h"

using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagRegister : public BlackfinDiagTest {

public:
	BlackfinDiagRegister() : 
	
	BlackfinDiagTest(PeriodPerTestIteration_Milleseconds){}


	virtual ~BlackfinDiagRegister() {}

	virtual TestState RunTest();

private:

	INT                           Critical;                    /* Temp to allow disabling interrupts around critical sections */

	static const UINT32 PeriodPerTestIteration_Milleseconds = 500;

    void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}
};


