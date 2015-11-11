#pragma once
#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"

namespace BlackfinDiagTesting 
{
    class BlackfinDiagInstructionsTest : public BlackfinDiagTest 
    {

        public:
            BlackfinDiagInstructionsTest( BlackfinExecTestData &  rTestData ) 
		    		     		:	BlackfinDiagTest             	  ( rTestData )
									
        	{
        	}

        	virtual ~BlackfinDiagInstructionsTest() 
        	{
        	}

        	virtual DiagnosticCommon::TestState RunTest( UINT32 & rErrorCode );

        protected:

        	virtual void ConfigureForNextTestCycle();
	
        private:
    };
};
