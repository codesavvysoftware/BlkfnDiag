#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagInstructionRam.h"
#include "BlackfinDiagDataRam.h"
#include "BlackfinDiagRegistersTest.h"


using namespace BlackfinDiagnosticGlobals;

class BlackfinRuntimeDiagnostics {

private:
		
	// Define Tests Here
	static BlackfinDiagInstructionRam InstructionRamTest;
	
    static BlackfinDiagDataRam DataRamTest;
    
    static BlackfinDiagRegistersTest RegisterTest;
    
    // Define the array of tests to run for the diagnostics
    static BlackfinDiagTest * DiagnosticTests[];// = {&DataRamTest, &InstructionRamTest };

    // Use a vector, it makes iterating through the tests easier when executing the tests
    static std::vector <BlackfinDiagTest *> Diagnostics;//(DiagnosticTests, end(DiagnosticTests));
    
public:
	
    // Constructor definition
    BlackfinRuntimeDiagnostics();
    
    static void ExecuteDiagnostics();

};


