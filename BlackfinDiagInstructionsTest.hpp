#pragma once
//#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"

extern "C" UINT32  BlackfinDiagInstrTest();

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

        	virtual TestState RunTest( UINT32 & rErrorCode );

        protected:

        	virtual void ConfigureForNextTestCycle();
	
        private:
    };
};
