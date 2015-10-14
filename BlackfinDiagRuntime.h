#pragma once
#include "BlackfinDiagTest.h"
#include "BlackfinDiagInstructionRam.h"
#include "BlackfinDiagDataRam.h"
#include "BlackfinDiagRegistersTest.h"

  
template<typename T, size_t N>
	T * end(T (&ra)[N]) {
		return ra + N;
	}
class BlackfinDiagRuntime {

private:
		
	static UINT8 * pRAMDataStart;

	static const UINT8  TestPatternsForRamTesting[];
	static const UINT32 NumberOfRamTestingPatterns;

	static const UINT32 TestPatternsForRegisterTesting[];
	static const UINT32 NumberOfRegisterPatterns;
	
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
    static const BlackfinDiagTest::REGISTER_TEST SanityCheck[]; 
    static const UINT32 NumberOfSanityChecks;
    		
    static const BlackfinDiagTest::REGISTER_TEST DataRegisters[];      
    static const UINT32 NumberOfDataRegTests;
    
    static const BlackfinDiagTest::REGISTER_TEST PointerRegisters[];
    static const UINT32 NumberOfPointerRegTests;


    static const BlackfinDiagTest::REGISTER_TEST Accumulators[];
	static const UINT32 NumberOfAccumulatorRegTests;
	
	static const BlackfinDiagTest::REGISTER_TEST ModifyRegisters[];
	static const UINT32 NumberOfModifyRegTests;

	static const BlackfinDiagTest::REGISTER_TEST LengthRegisters[];
	static const UINT32 NumberOfLengthRegTests;

	static const BlackfinDiagTest::REGISTER_TEST IndexRegisters[];
	static const UINT32 NumberOfIndexRegTests;

	static const BlackfinDiagTest::REGISTER_TEST BaseRegisters[];   
	static const UINT32 NumberOfBaseRegTests;

    static const BlackfinDiagTest::BlackfinRegisterTestSuite RegisterTestSuite;
    
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
    BlackfinDiagRuntime();
    
    static void ExecuteDiagnostics();

};


