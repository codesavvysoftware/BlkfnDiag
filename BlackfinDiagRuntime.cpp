#include "BlackfinDiagScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"

using namespace DiagnosticCommon;
using namespace BlackfinDiagTesting;


namespace BlackfinDiagRuntimeEnvironment {

DiagTimestampTime_t          defaultTestStartTime                = 0;
DiagTimestampTime_t          defaultIterationComplete            = 0;
UINT32                       defaultNmbrTimesToRunPerDiagCycle   = 1;
UINT32                       defaultNmbrTimesRanThisDiagCycle    = 0;
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

BlackfinDiagTest::DiagnosticTestTypes  testTypeDataRam = BlackfinDiagTest::DIAG_DATA_RAM_TEST_TYPE;

BlackfinDiagTest::BlackfinExecTestData testDataDataRAMTest_ = {
																dataRAMTestIterationPeriodMS_,
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
                                   testDataDataRAMTest_ );
    
static BlackfinDiagTest * dataRamTestPtr = &dataRamTest_;

//***********************************************************************************************************
//                                                                                                          *
// Register testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************

const UINT32 testPatternsForRegisterTesting_[]   = { 0xffffffff, 0xaaaaaaaa, 0x55555555, 0 };
UINT32 nmbrOfRegisterPatterns_                   = sizeof(testPatternsForRegisterTesting_)/sizeof(UINT32);
	
  	
    
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
BlackfinDiagTest::pRegisterTest sanityCheck_[] 
    	= { BlackfinDiagRegSanityChk };
UINT32 nmbrOfSanityChecks_ = sizeof(sanityCheck_)/sizeof( BlackfinDiagTest::pRegisterTest );
    		
BlackfinDiagTest::pRegisterTest dataRegisters_[]      
		= {
			BlackfinDiagRegDataReg7Chk, 
            BlackfinDiagRegDataReg6Chk,
            BlackfinDiagRegDataReg5Chk,
            BlackfinDiagRegDataReg4Chk,
            BlackfinDiagRegDataReg3Chk
          };

UINT32 nmbrOfDataRegTests_ = sizeof(dataRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );
    
BlackfinDiagTest::pRegisterTest pointerRegisters_[] 
    	= {
    		BlackfinDiagRegPointerReg5Chk, 
            BlackfinDiagRegPointerReg4Chk,
            BlackfinDiagRegPointerReg3Chk,
            BlackfinDiagRegPointerReg2Chk,
            BlackfinDiagRegPointerReg0Chk
          };
UINT32 nmbrOfPointerRegTests_ = sizeof(pointerRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );


BlackfinDiagTest::pRegisterTest accumulators_[]     
    	= {
    		BlackfinDiagAccum0Chk, 
    		BlackfinDiagAccum1Chk
    	  };
UINT32 nmbrOfAccumulatorRegTests_ = sizeof(accumulators_)/sizeof( BlackfinDiagTest::pRegisterTest );
	
BlackfinDiagTest::pRegisterTest modifyRegisters_[]  
		= {
			BlackfinDiagRegModifyReg3Chk,
            BlackfinDiagRegModifyReg2Chk,
            BlackfinDiagRegModifyReg1Chk,
            BlackfinDiagRegModifyReg0Chk
          };
UINT32 nmbrOfModifyRegTests_ = sizeof(modifyRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );

BlackfinDiagTest::pRegisterTest lengthRegisters_[]  
		= {
			BlackfinDiagRegLengthReg3Chk,
            BlackfinDiagRegLengthReg2Chk,
            BlackfinDiagRegLengthReg1Chk,
            BlackfinDiagRegLengthReg0Chk
          };
UINT32 nmbrOfLengthRegTests_ = sizeof(lengthRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );

BlackfinDiagTest::pRegisterTest indexRegisters_[]   
		= {
			BlackfinDiagRegIndexReg3Chk,
            BlackfinDiagRegIndexReg2Chk,
            BlackfinDiagRegIndexReg1Chk,
            BlackfinDiagRegIndexReg0Chk
          };
UINT32 nmbrOfIndexRegTests_ = sizeof(indexRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );

BlackfinDiagTest::pRegisterTest baseRegisters_[]    
		= {
			BlackfinDiagRegBaseReg3Chk,
    	    BlackfinDiagRegBaseReg2Chk,
            BlackfinDiagRegBaseReg1Chk,
            BlackfinDiagRegBaseReg0Chk
          };
UINT32 nmbrOfBaseRegTests_ = sizeof(baseRegisters_)/sizeof( BlackfinDiagTest::pRegisterTest );

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
	
BlackfinDiagTest::BlackfinExecTestData testDataRegistersTest_ = {
																	registerTestIterationPeriodMS_,
    																defaultIterationComplete,
																	defaultNmbrTimesToRunPerDiagCycle,
																	defaultNmbrTimesRanThisDiagCycle,
																	BlackfinDiagTest::DIAG_REGISTER_TEST_TEST_TYPE,
																	defaultInitialTestExecutionState
																};

BlackfinDiagRegistersTest registerTest_( registerTestSuite_, 
                                         nmbrOfRegisterTestDescriptors_, 
                                         testPatternsForRegisterTesting_, 
                                         nmbrOfRegisterPatterns_,
                                         testDataRegistersTest_ ); 
 
static BlackfinDiagTest * registerTestPtr = &registerTest_;
                                         
 

//***********************************************************************************************************
//                                                                                                          *
// Instruction RAM testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
DiagElapsedTime_t       instructionRamTestIterationPeriodMS_            = 2000; // 2 second for now

BlackfinDiagTest::BlackfinExecTestData testDataInstructionRAMTest_ = {
					      												instructionRamTestIterationPeriodMS_,
																		defaultIterationComplete,
											    						defaultNmbrTimesToRunPerDiagCycle,
							    										defaultNmbrTimesRanThisDiagCycle,
								    									BlackfinDiagTest::DiagInstructionRamTestType,
														    			defaultInitialTestExecutionState
															         };



BlackfinDiagInstructionRam instructionRamTest_( testDataInstructionRAMTest_ );

static BlackfinDiagTest * instructionRamTestPtr = &instructionRamTest_;
	
//***********************************************************************************************************
//                                                                                                          *
// Timer testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
DiagnosticCommon::DiagElapsedTime_t         timerTestIterationPeriodMS_ = 1000;  // Every second.
	
UINT32                                      errorTimerTestApexTimer_     = 1;
UINT32                                      errorTimerTestHostTimer_     = 2;
DiagElapsedTime_t                           maxTimerTestElapsedTimeApex_ = 950;
DiagElapsedTime_t                           maxTimerTestElapsedTimeHost_ = 1050;
DiagElapsedTime_t                           minTimerTestElapsedTimeApex_ = 950;
DiagElapsedTime_t                           minTimerTestElapsedTimeHost_ = 1050; 
BlackfinDiagTest::BlackfinExecTestData      timerTestData_ = {
																timerTestIterationPeriodMS_,
																defaultIterationComplete,
											    				defaultNmbrTimesToRunPerDiagCycle,
							    								defaultNmbrTimesRanThisDiagCycle,
								    							BlackfinDiagTest::DiagTimerTestType,
														    	defaultInitialTestExecutionState
															 };

BlackfinDiagTesting::BlackfinDiagTimerTest    timerTest_( errorTimerTestApexTimer_,
     	                                                  errorTimerTestHostTimer_,
	    												  maxTimerTestElapsedTimeApex_,
														  maxTimerTestElapsedTimeHost_,
					                                      minTimerTestElapsedTimeApex_,
					                                      minTimerTestElapsedTimeHost_ ,
					                                      timerTestData_ );
					                                                 
			
static BlackfinDiagTest * timerTestPtr = &timerTest_;

	
//***********************************************************************************************************
//                                                                                                          *
// Runtime Blackfin Diagnostic Test Data                                                                    *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagTest * BlackfinDiagRuntime::diagnosticTests_[] 
	= {
//		registerTestPtr,
//		dataRamTestPtr, 
		timerTestPtr,
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


