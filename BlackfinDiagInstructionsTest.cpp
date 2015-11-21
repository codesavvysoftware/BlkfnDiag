#include "BlackfinDiagInstructionsTest.hpp"
#include "Os_iotk.h"
#include "Hw.h"
namespace BlackfinDiagTesting 
{
	
    TestState BlackfinDiagInstructionsTest::RunTest( UINT32 & rErrorCode ) 
    {
        UINT32 error = BlackfinDiagInstrTest();
		
        if ( error )
        {
    	    error &= DIAG_ERROR_MASK;
			
    		error |= ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    	    OS_Assert( error );
        }
        
    	return TEST_LOOP_COMPLETE;
    }
    
    void BlackfinDiagInstructionsTest::ConfigureForNextTestCycle() 
    {
    }
};
