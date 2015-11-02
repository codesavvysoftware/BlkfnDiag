#include "BlackfinDiagScheduler.h"
#include "BlackfinDiagRuntime.h"

using namespace DiagnosticCommon;

namespace BlackfinDiagTests {

DiagTimestampTime_t          defaultTestStartTime                = 0;
DiagTimestampTime_t          defaultIterationComplete            = 0;
UINT32                       defaultNmbrTimesToRunPerDiagCycle   = 1;
UINT32                       defaultNmbrTimesRanThisDiagCycle    = 1;
BlackfinDiagTest::TestState  defaultInitialTestExecutionState    = BlackfinDiagTest::TEST_IDLE;

//***********************************************************************************************************
//                                                                                                          *
// Data RAM testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagTest::BlackfinDataRamTestSuite  dataRamTestInfo_ = { 
																{ reinterpret_cast<UINT8 *>(0xff800000), 0x8000, 0, FALSE }, // Bank A
                                                                { reinterpret_cast<UINT8 *>(0xff900000), 0x8000, 0, FALSE }, // Bank B
                                                                { reinterpret_cast<UINT8 *>(0xffb00000), 0x1000, 0, FALSE }  // Bank C
                                                              };

const UINT8            testPatternsForRamTesting_[]             = { 0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 };

UINT32           numberOfRamTestingPatterns_              = sizeof( testPatternsForRamTesting_ ) / sizeof( UINT8 );
UINT32           nmberDataRAMBytesToTestPerIteration_     = 0x400; // Test 1k at a time for now

DiagElapsedTime_t       dataRAMTestIterationPeriodMS_            = 1000; // 1 second for now
DiagElapsedTime_t       dataRAMStartOffsetFromDiagCycleStartMS_  = 20;   // Start 20 ms after the diagnostic cycle starts

BlackfinDiagTest::DiagnosticTestTypes  testTypeDataRam = BlackfinDiagTest::DiagDataRamTestType;

BlackfinDiagTest::BlackfinExecTestData testDataDataRAMTest = {
																dataRAMTestIterationPeriodMS_,
																dataRAMStartOffsetFromDiagCycleStartMS_,
																defaultIterationComplete,
																defaultNmbrTimesToRunPerDiagCycle,
																defaultNmbrTimesRanThisDiagCycle,
																testTypeDataRam,
																defaultInitialTestExecutionState
															};
											
BlackfinDiagDataRam dataRamTest_(  &dataRamTestInfo_, 
                                   testPatternsForRamTesting_, 
                                   numberOfRamTestingPatterns_, 
                                   nmberDataRAMBytesToTestPerIteration_,
                                   testDataDataRAMTest );
    
static BlackfinDiagTest * dataRamTestPtr = &dataRamTest_;

//***********************************************************************************************************
//                                                                                                          *
// Register testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************
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
const UINT32 testPatternsForRegisterTesting_[]   = { 0xffffffff, 0xaaaaaaaa, 0x55555555, 0 };
UINT32 nmbrOfRegisterPatterns_             = sizeof(testPatternsForRegisterTesting_)/sizeof(UINT32);
	
  	
    
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
BlackfinDiagTest::REGISTER_TEST sanityCheck_[] 
    	= { BlackfinDiagRegSanityChk };
UINT32 nmbrOfSanityChecks_ = sizeof(sanityCheck_)/sizeof( BlackfinDiagTest::REGISTER_TEST );
    		
BlackfinDiagTest::REGISTER_TEST dataRegisters_[]      
		= {
			BlackfinDiagRegDataReg7Chk, 
            BlackfinDiagRegDataReg6Chk,
            BlackfinDiagRegDataReg5Chk,
            BlackfinDiagRegDataReg4Chk,
            BlackfinDiagRegDataReg3Chk
          };

UINT32 nmbrOfDataRegTests_ = sizeof(dataRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );
    
BlackfinDiagTest::REGISTER_TEST pointerRegisters_[] 
    	= {
    		BlackfinDiagRegPointerReg5Chk, 
            BlackfinDiagRegPointerReg4Chk,
            BlackfinDiagRegPointerReg3Chk,
            BlackfinDiagRegPointerReg2Chk,
            BlackfinDiagRegPointerReg0Chk
          };
UINT32 nmbrOfPointerRegTests_ = sizeof(pointerRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );


BlackfinDiagTest::REGISTER_TEST accumulators_[]     
    	= {
    		BlackfinDiagAccum0Chk, 
    		BlackfinDiagAccum1Chk
    	  };
UINT32 nmbrOfAccumulatorRegTests_ = sizeof(accumulators_)/sizeof( BlackfinDiagTest::REGISTER_TEST );
	
BlackfinDiagTest::REGISTER_TEST modifyRegisters_[]  
		= {
			BlackfinDiagRegModifyReg3Chk,
            BlackfinDiagRegModifyReg2Chk,
            BlackfinDiagRegModifyReg1Chk,
            BlackfinDiagRegModifyReg0Chk
          };
UINT32 nmbrOfModifyRegTests_ = sizeof(modifyRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST lengthRegisters_[]  
		= {
			BlackfinDiagRegLengthReg3Chk,
            BlackfinDiagRegLengthReg2Chk,
            BlackfinDiagRegLengthReg1Chk,
            BlackfinDiagRegLengthReg0Chk
          };
UINT32 nmbrOfLengthRegTests_ = sizeof(lengthRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST indexRegisters_[]   
		= {
			BlackfinDiagRegIndexReg3Chk,
            BlackfinDiagRegIndexReg2Chk,
            BlackfinDiagRegIndexReg1Chk,
            BlackfinDiagRegIndexReg0Chk
          };
UINT32 nmbrOfIndexRegTests_ = sizeof(indexRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST baseRegisters_[]    
		= {
			BlackfinDiagRegBaseReg3Chk,
    	    BlackfinDiagRegBaseReg2Chk,
            BlackfinDiagRegBaseReg1Chk,
            BlackfinDiagRegBaseReg0Chk
          };
UINT32 nmbrOfBaseRegTests_ = sizeof(baseRegisters_)/sizeof( BlackfinDiagTest::REGISTER_TEST );

const BlackfinDiagTest::RegisterTestDescriptor registerTestSuite_[] = 
								{
									{ 
										sanityCheck_,      
										nmbrOfSanityChecks_,        
										FALSE 
									},
									{ 
										dataRegisters_,
										nmbrOfDataRegTests_,        
										FALSE  
									},
									{ 
										pointerRegisters_, 
										nmbrOfPointerRegTests_,     
										FALSE 
									},
									{ 
										accumulators_,     
										nmbrOfAccumulatorRegTests_, 
										FALSE  
									},
									{ 
										baseRegisters_,    
										nmbrOfBaseRegTests_,        
										FALSE 
									},
									{ 
										indexRegisters_,   
										nmbrOfIndexRegTests_,       
										FALSE  
									},
									{ 
										lengthRegisters_,  
										nmbrOfLengthRegTests_,      
										FALSE 
									},
									{ 
										modifyRegisters_,  
										nmbrOfModifyRegTests_,      
										FALSE  
									}
								};
	 
UINT32 nmbrOfRegisterTestDescriptors_ = sizeof( registerTestSuite_ ) / sizeof(BlackfinDiagTest::RegisterTestDescriptor); 

DiagElapsedTime_t                registerTestIterationPeriodMS_ = 2000;          // Every Two Seconds	
DiagElapsedTime_t                registerTestStartOffsetFromDiagCycleStartMS_;  // Start 250 ms in
	
BlackfinDiagTest::BlackfinExecTestData testDataRegistersTest = {
																	registerTestIterationPeriodMS_,
																	registerTestStartOffsetFromDiagCycleStartMS_,
    																defaultIterationComplete,
																	defaultNmbrTimesToRunPerDiagCycle,
																	defaultNmbrTimesRanThisDiagCycle,
																	BlackfinDiagTest::DiagRegisterTestType,
																	defaultInitialTestExecutionState
																};

BlackfinDiagRegistersTest registerTest_( registerTestSuite_, 
                                         nmbrOfRegisterTestDescriptors_, 
                                         testPatternsForRegisterTesting_, 
                                         nmbrOfRegisterPatterns_,
                                         testDataRegistersTest ); 
 
static BlackfinDiagTest * registerTestPtr = &registerTest_;
                                         
 

//***********************************************************************************************************
//                                                                                                          *
// Instruction RAM testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
DiagElapsedTime_t       instructionRamTestIterationPeriodMS_            = 2000; // 2 second for now
DiagElapsedTime_t       instructionRAMStartOffsetFromDiagCycleStartMS_  = 500;   // Start 500 ms after the diagnostic cycle starts

BlackfinDiagTest::BlackfinExecTestData testDataInstructionRAMTest = {
					      												instructionRamTestIterationPeriodMS_,
				 	    												instructionRAMStartOffsetFromDiagCycleStartMS_,
																		defaultIterationComplete,
											    						defaultNmbrTimesToRunPerDiagCycle,
							    										defaultNmbrTimesRanThisDiagCycle,
								    									BlackfinDiagTest::DiagInstructionRamTestType,
														    			defaultInitialTestExecutionState
															         };



BlackfinDiagInstructionRam instructionRamTest_( testDataInstructionRAMTest );

static BlackfinDiagTest * instructionRamTestPtr = &instructionRamTest_;
	
//***********************************************************************************************************
//                                                                                                          *
// Runtime Blackfin Diagnostic Test Data                                                                    *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagTest * BlackfinDiagRuntime::diagnosticTests_[] 
	= {
//		registerTestPtr,
		dataRamTestPtr, 
//		instructionRamTestPtr
	  }; 


std::vector <BlackfinDiagTest *> BlackfinDiagRuntime::diagnostics_(diagnosticTests_, end(diagnosticTests_));

void BlackfinDiagRuntime::ExecuteDiagnostics() { 
   
    static BlackfinDiagScheduler Schedule(&BlackfinDiagRuntime::diagnostics_);

	Schedule.RunScheduled(); 
}

BlackfinDiagRuntime::BlackfinDiagRuntime() {

}


};


