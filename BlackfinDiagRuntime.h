#pragma once
#include "BlackfinDiagTest.h"
#include "BlackfinDiagInstructionRam.h"
#include "BlackfinDiagDataRam.h"
#include "BlackfinDiagRegistersTest.h"

namespace BlackfinDiagTests {  
template<typename T, size_t N>
	T * end(T (&ra)[N]) {
		return ra + N;
	}
class BlackfinDiagRuntime {

private:
	//
	// Var suffix of:
	// 
	// 	MS == milleseconds
	// 	US == microseconds
	//	
	//***********************************************************************************************************
	//                                                                                                          *
	// Data RAM testing parameters, structures and definitions.                                                 *
	//                                                                                                          *
	//***********************************************************************************************************
	static const UINT32                                                   nmberDataRAMBytesToTestPerIteration_;
	
	static const DiagnosticCommon::DiagElapaedTime_t                             dataRAMTestIterationPeriodMS_;
	
	static const DiagnosticCommon::DiagElapsedTime_t                             dataRAMStartOffsetFromDiagCycleStartMS_;
	
	static const BlackfinDiagTest::BlackfinDataRamTestSuite               dataRamTestInfo_;

	static const UINT8                                                    testPatternsForRamTesting_[];
	static const UINT32                                                   nmbrOfRamTestingPatterns_;
	
	static const BlackfinDiagTest::BlackfinExecTestData                   dataDataRAMTes_t;

    static const BlackfinDiagTest::DiagnosticTestTypes                                      testTypeDataRam;
    
    static BlackfinDiagDataRam                                            dataRamTest_;
    
    
    
	//***********************************************************************************************************
	//                                                                                                          *
	// Register testing parameters, structures and definitions.                                                 *
	//                                                                                                          *
	//***********************************************************************************************************
	//
 	static const UINT32           testPatternsForRegisterTesting_[];
	static const UINT32           nmbrOfRegisterPatterns_;
	
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
    static const BlackfinDiagTest::REGISTER_TEST              sanityCheck_[]; 
    static const UINT32                                       nmbrOfSanityChecks_;
    		
    static const BlackfinDiagTest::REGISTER_TEST              dataRegisters_[];      
    static const UINT32                                       nmbrOfDataRegTests_;
    
    static const BlackfinDiagTest::REGISTER_TEST              pointerRegisters_[];
    static const UINT32                                       nmbrOfPointerRegTests_;


    static const BlackfinDiagTest::REGISTER_TEST              accumulators_[];
	static const UINT32                                       nmbrOfAccumulatorRegTests_;
	
	static const BlackfinDiagTest::REGISTER_TEST              modifyRegisters_[];
	static const UINT32                                       nmbrOfModifyRegTests_;

	static const BlackfinDiagTest::REGISTER_TEST              lengthRegisters_[];
	static const UINT32                                       nmbrOfLengthRegTests_;

	static const BlackfinDiagTest::REGISTER_TEST              indexRegisters_[];
	static const UINT32                                       nmbrOfIndexRegTests_;

	static const BlackfinDiagTest::REGISTER_TEST              baseRegisters_[];   
	static const UINT32                                       nmbrOfBaseRegTests_;

    static const BlackfinDiagTest::RegisterTestDescriptor     registerTestSuite_[];
    
    static const UINT32                                       nmbrOfRegisterTestDescriptors_;
 
	static const DiagnosticCommon::DiagElapsedTime_t                 registerTestIterationPeriodMS_;
	
	static const DiagnosticCommon::DiagElapsedTime_t                 registerTestStartOffsetFromDiagCycleStartMS_;
	
    static const BlackfinDiagTest::BlackfinExecTestData       testDataRegistersTest;
    
    static BlackfinDiagRegistersTest                          registerTest_;
    
	//***********************************************************************************************************
	//                                                                                                          *
	// Instruction RAM testing parameters, structures and definitions.                                          *
	//                                                                                                          *
	//***********************************************************************************************************
	static const DiagnosticCommon::DiagElapsedTime_t                  instructionRAMTestIterationPeriodMS_;
	
	static const DiagnosticCommon::DiagElapsedTime_t                  instructionRAMStartOffsetFromDiagCycleStartMS_;
	
    static const BlackfinDiagTest::BlackfinExecTestData        testDataInstructionRAMTest;
    
	static BlackfinDiagInstructionRam                          instructionRamTest_;
	
	//***********************************************************************************************************
	//                                                                                                          *
	// Run the complete tests definitions.                                                                      *
	//                                                                                                          *
	//***********************************************************************************************************
    // Define the array of tests to run for the diagnostics
    static BlackfinDiagTest *                        diagnosticTests_[];

    // Use a vector, it makes iterating through the tests easier when executing the tests
    static std::vector <BlackfinDiagTest *>          diagnostics_;//(DiagnosticTests, end(DiagnosticTests));
    
    // Constructor definition
    BlackfinDiagRuntime();
    
public:
	
    static void ExecuteDiagnostics();

};

};

