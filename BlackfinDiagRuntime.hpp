#pragma once
#include "BlackfinDiagTest.h"
#include "BlackfinDiagInstructionRam.h"
#include "BlackfinDiagDataRam.h"
#include "BlackfinDiagRegistersTest.h"
#include "BlackfinDiagTimerTest.h"

namespace BlackfinDiagRuntimeEnvironment {  
//
// For linkage to c callable assembly language register tests
//
extern "C" UINT32  BlackfinDiagRegSanityChk( const UINT32 *, UINT32);
extern "C" UINT32  BlackfinDiagRegDataReg7Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegDataReg6Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegDataReg5Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegDataReg4Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegDataReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegPointerReg5Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegPointerReg4Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegPointerReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegPointerReg2Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegPointerReg0Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagAccum0Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagAccum1Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegModifyReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegModifyReg2Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegModifyReg1Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegModifyReg0Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegLengthReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegLengthReg2Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegLengthReg1Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegLengthReg0Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegIndexReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegIndexReg2Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegIndexReg1Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegIndexReg0Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegBaseReg3Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegBaseReg2Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegBaseReg1Chk( const UINT32 *, UINT32 );
extern "C" UINT32  BlackfinDiagRegBaseReg0Chk( const UINT32 *, UINT32 );	

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
	static const UINT32                                                           nmberDataRAMBytesToTestPerIteration_;
	
	static const DiagnosticCommon::DiagElapsedTime_t                              dataRAMTestIterationPeriodMS_;
	
	static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinDataRamTestSuite  dataRamTestInfo_;

	static const UINT8                                                            testPatternsForRamTesting_[];

	static const UINT32                                                           nmbrOfRamTestingPatterns_;
	
	static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      dataDataRAMTes_t;

    static const BlackfinDiagTesting::BlackfinDiagTest::DiagnosticTestTypes       testTypeDataRam_;
    
    static BlackfinDiagTesting::BlackfinDiagDataRam                               dataRamTest_;
    
    
    
	//***********************************************************************************************************
	//                                                                                                          *
	// Register testing parameters, structures and definitions.                                                 *
	//                                                                                                          *
	//***********************************************************************************************************
	//
 	static const UINT32                                                           testPatternsForRegisterTesting_[];
	static const UINT32                                                           nmbrOfRegisterPatterns_;
	
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
    static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             sanityCheck_[]; 
    static const UINT32                                                           nmbrOfSanityChecks_;
    		
    static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             dataRegisters_[];      
    static const UINT32                                                           nmbrOfDataRegTests_;
    
    static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             pointerRegisters_[];
    static const UINT32                                                           nmbrOfPointerRegTests_;


    static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             accumulators_[];
	static const UINT32                                                           nmbrOfAccumulatorRegTests_;
	
	static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             modifyRegisters_[];
	static const UINT32                                                           nmbrOfModifyRegTests_;

	static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             lengthRegisters_[];
	static const UINT32                                                           nmbrOfLengthRegTests_;

	static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             indexRegisters_[];
	static const UINT32                                                           nmbrOfIndexRegTests_;

	static const BlackfinDiagTesting::BlackfinDiagTest::REGISTER_TEST             baseRegisters_[];   
	static const UINT32                                                           nmbrOfBaseRegTests_;

    static const BlackfinDiagTesting::BlackfinDiagTest::RegisterTestDescriptor    registerTestSuite_[];
    
    static const UINT32                                                           nmbrOfRegisterTestDescriptors_;
 
	static const DiagnosticCommon::DiagElapsedTime_t                              registerTestIterationPeriodMS_;
	
    static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      testDataRegistersTest;
    
    static BlackfinDiagTesting::BlackfinDiagRegistersTest                         registerTest_;
    
	//***********************************************************************************************************
	//                                                                                                          *
	// Instruction RAM testing parameters, structures and definitions.                                          *
	//                                                                                                          *
	//***********************************************************************************************************
	static const DiagnosticCommon::DiagElapsedTime_t                              instructionRAMTestIterationPeriodMS_;
	
    static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      testDataInstructionRAMTest_;
    
	static BlackfinDiagTesting::BlackfinDiagInstructionRam                        instructionRamTest_;
	
	//***********************************************************************************************************
	//                                                                                                          *
	// Timer testing parameters, structures and definitions.                                          *
	//                                                                                                          *
	//***********************************************************************************************************
	static const DiagnosticCommon::DiagElapsedTime_t                              timerTestIterationPeriodMS_;
	
	static const UINT32                                                           errorTimerTestApexTimer_;
	static const UINT32                                                           errorTimerTestHostTimer_;
	static const DiagnosticCommon::DiagElapsedTime_t                              maxTimerTestElapsedTimeApex_;
	static const DiagnosticCommon::DiagElapsedTime_t                              maxTimerTestElapsedTimeHost_;
	static const DiagnosticCommon::DiagElapsedTime_t                              minTimerTestElapsedTimeApex_;
	static const DiagnosticCommon::DiagElapsedTime_t                              minTimerTestElapsedTimeHost_; 
    static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      timerTestData_;
    
	static BlackfinDiagTesting::BlackfinDiagTimerTest                             timerTest_;
	
	//***********************************************************************************************************
	//                                                                                                          *
	// Run the complete tests definitions.                                                                      *
	//                                                                                                          *
	//***********************************************************************************************************
    // Define the array of tests to run for the diagnostics
    static BlackfinDiagTesting::BlackfinDiagTest *                                diagnosticTests_[];

    // Use a vector, it makes iterating through the tests easier when executing the tests
    static std::vector <BlackfinDiagTesting::BlackfinDiagTest *>                  diagnostics_;
    
    // Constructor definition
    BlackfinDiagRuntime();
    
public:
	
    static void ExecuteDiagnostics();

};

};

