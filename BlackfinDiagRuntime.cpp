#include "BlackfinDiagScheduler.h"
#include "BlackfinDiagRuntime.h"


//***********************************************************************************************************
//                                                                                                          *
// Data RAM testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagTest::BlackfinDataRamTestSuite  DataRamTestInfo = { 
																{ reinterpret_cast<UINT8 *>(0xff800000), 0x8000, 0, FALSE }, // Bank A
                                                                { reinterpret_cast<UINT8 *>(0xff900000), 0x8000, 0, FALSE }, // Bank B
                                                                { reinterpret_cast<UINT8 *>(0xffb00000), 0x1000, 0, FALSE }  // Bank C
                                                              };

UINT8  TestPatternsForRamTesting[]         = { 0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 };
UINT32 NumberOfRamTestingPatterns          = sizeof(TestPatternsForRamTesting) / sizeof(UINT8);

BlackfinDiagDataRam BlackfinDiagRuntime::DataRamTest( &::DataRamTestInfo, ::TestPatternsForRamTesting, ::NumberOfRamTestingPatterns );


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
UINT32 TestPatternsForRegisterTesting[]   = { 0xffffffff, 0xaaaaaaaa, 0x55555555, 0 };
UINT32 NumberOfRegisterPatterns           = sizeof(TestPatternsForRegisterTesting)/sizeof(UINT32);
	
  	
    
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
BlackfinDiagTest::REGISTER_TEST SanityCheck[] 
    	= { BlackfinDiagRegSanityChk };
UINT32 NumberOfSanityChecks = sizeof(SanityCheck)/sizeof( BlackfinDiagTest::REGISTER_TEST );
    		
BlackfinDiagTest::REGISTER_TEST DataRegisters[]      
		= {
			BlackfinDiagRegDataReg7Chk, 
            BlackfinDiagRegDataReg6Chk,
            BlackfinDiagRegDataReg5Chk,
            BlackfinDiagRegDataReg4Chk,
            BlackfinDiagRegDataReg3Chk
          };

UINT32 NumberOfDataRegTests = sizeof(DataRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );
    
BlackfinDiagTest::REGISTER_TEST PointerRegisters[] 
    	= {
    		BlackfinDiagRegPointerReg5Chk, 
            BlackfinDiagRegPointerReg4Chk,
            BlackfinDiagRegPointerReg3Chk,
            BlackfinDiagRegPointerReg2Chk,
            BlackfinDiagRegPointerReg0Chk
          };
UINT32 NumberOfPointerRegTests = sizeof(PointerRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );


BlackfinDiagTest::REGISTER_TEST Accumulators[]     
    	= {
    		BlackfinDiagAccum0Chk, 
    		BlackfinDiagAccum1Chk
    	  };
UINT32 NumberOfAccumulatorRegTests = sizeof(Accumulators)/sizeof( BlackfinDiagTest::REGISTER_TEST );
	
BlackfinDiagTest::REGISTER_TEST ModifyRegisters[]  
		= {
			BlackfinDiagRegModifyReg3Chk,
            BlackfinDiagRegModifyReg2Chk,
            BlackfinDiagRegModifyReg1Chk,
            BlackfinDiagRegModifyReg0Chk
          };
UINT32 NumberOfModifyRegTests = sizeof(ModifyRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST LengthRegisters[]  
		= {
			BlackfinDiagRegLengthReg3Chk,
            BlackfinDiagRegLengthReg2Chk,
            BlackfinDiagRegLengthReg1Chk,
            BlackfinDiagRegLengthReg0Chk
          };
UINT32 NumberOfLengthRegTests = sizeof(LengthRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST IndexRegisters[]   
		= {
			BlackfinDiagRegIndexReg3Chk,
            BlackfinDiagRegIndexReg2Chk,
            BlackfinDiagRegIndexReg1Chk,
            BlackfinDiagRegIndexReg0Chk
          };
UINT32 NumberOfIndexRegTests = sizeof(IndexRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::REGISTER_TEST BaseRegisters[]    
		= {
			BlackfinDiagRegBaseReg3Chk,
    	    BlackfinDiagRegBaseReg2Chk,
            BlackfinDiagRegBaseReg1Chk,
            BlackfinDiagRegBaseReg0Chk
          };
UINT32 NumberOfBaseRegTests = sizeof(BaseRegisters)/sizeof( BlackfinDiagTest::REGISTER_TEST );

BlackfinDiagTest::RegisterTestDescriptor RegisterTestSuite[] = 
								{
									{ ::SanityCheck,      ::NumberOfSanityChecks,        FALSE  },
									{ ::DataRegisters,    ::NumberOfDataRegTests,        FALSE  },
									{ ::PointerRegisters, ::NumberOfPointerRegTests,     FALSE  },
									{ ::Accumulators,     ::NumberOfAccumulatorRegTests, FALSE  },
									{ ::BaseRegisters,    ::NumberOfBaseRegTests,        FALSE  },
									{ ::IndexRegisters,   ::NumberOfIndexRegTests,       FALSE  },
									{ ::LengthRegisters,  ::NumberOfLengthRegTests,      FALSE  },
									{ ::ModifyRegisters,  ::NumberOfModifyRegTests,      FALSE  }
								};
	 
UINT32 NumberOfRegisterTestDescriptors = sizeof( RegisterTestSuite ) / sizeof(BlackfinDiagTest::RegisterTestDescriptor); 

BlackfinDiagRegistersTest BlackfinDiagRuntime::RegisterTest( ::RegisterTestSuite, 
                                                             ::NumberOfRegisterTestDescriptors, 
                                                             ::TestPatternsForRegisterTesting, 
                                                             ::NumberOfRegisterPatterns );
 

//***********************************************************************************************************
//                                                                                                          *
// Instruction RAM testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagInstructionRam BlackfinDiagRuntime::InstructionRamTest;
	
    
//***********************************************************************************************************
//                                                                                                          *
// Runtime Blackfin Diagnostic Test Data                                                                    *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagTest * BlackfinDiagRuntime::DiagnosticTests[] 
	= {
		//&BlackfinDiagRuntime::RegisterTest,
		&BlackfinDiagRuntime::DataRamTest, 
		//&BlackfinDiagRuntime::InstructionRamTest
	  };

std::vector <BlackfinDiagTest *> BlackfinDiagRuntime::Diagnostics(DiagnosticTests, end(DiagnosticTests));

void BlackfinDiagRuntime::ExecuteDiagnostics() { 
   
    static BlackfinDiagScheduler Schedule(&BlackfinDiagRuntime::Diagnostics);

	Schedule.RunScheduled(); 
}

BlackfinDiagRuntime::BlackfinDiagRuntime() {

}

