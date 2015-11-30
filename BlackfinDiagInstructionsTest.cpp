#include "BlackfinDiagInstructionsTest.hpp"
#include "Os_iotk.h"
#include "Hw.h"
namespace BlackfinDiagTesting 
{
	
    BlackfinDiagTest::TestState BlackfinDiagInstructionsTest::RunTest( UINT32 & rErrorCode ) 
    {
        UINT32 error = BlackfinDiagInstrTest();
        
        if ( error )
        {
    	    error &= BlackfinDiagTest::DIAG_ERROR_MASK;
			
    		error |= ( GetTestType() << BlackfinDiagTest::DIAG_ERROR_TYPE_BIT_POS );
			
    	    OS_Assert( error );
        }
        
    	return BlackfinDiagTest::TEST_LOOP_COMPLETE;
    }
    
    void BlackfinDiagInstructionsTest::ConfigureForNextTestCycle() 
    {
    }
};
