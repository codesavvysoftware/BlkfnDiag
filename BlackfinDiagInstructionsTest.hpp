#pragma once
#include "DiagnosticTesting.hpp"

extern "C" UINT32  BlackfinDiagInstrTest();

namespace BlackfinDiagnosticTesting 
{
    class BlackfinDiagInstructionsTest : public DiagnosticTesting::DiagnosticTest 
    {

        public:
            BlackfinDiagInstructionsTest( DiagnosticTesting::DiagnosticTest::ExecuteTestData &  rTestData ) 
		    		     		:	DiagnosticTesting::DiagnosticTest             	  ( rTestData )
									
        	{
        	}

        	DiagnosticTesting::DiagnosticTest::TestState RunTest( UINT32 & rErrorCode );

        protected:

        	void ConfigureForNextTestCycle();
	
        private:
	        BlackfinDiagInstructionsTest(const BlackfinDiagInstructionsTest &);
	
	        const BlackfinDiagInstructionsTest & operator = (const BlackfinDiagInstructionsTest & );
		
            BlackfinDiagInstructionsTest();
            
    };
};
