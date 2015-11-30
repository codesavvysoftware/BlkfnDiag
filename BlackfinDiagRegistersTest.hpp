#pragma once
#include "DiagnosticTesting.hpp"
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


namespace BlackfinDiagnosticTesting 
{
    #define CORRUPTED_REG_TST 0xff
	
      //
    // For linkage to c callable assembly language register tests
    //
    extern "C" UINT32  BlackfinDiagRegSanityChk();
    extern "C" UINT32  BlackfinDiagRegChk();

    class BlackfinDiagRegistersTest : public DiagnosticTesting::DiagnosticTest 
    {

        public:

            typedef UINT32 (* const pRegisterTest)();
	
			BlackfinDiagRegistersTest( DiagnosticTesting::DiagnosticTest::ExecuteTestData &  rTestData ) 
		    		     		:	   DiagnosticTesting::DiagnosticTest ( rTestData ),
									   m_SanityTestRan                   ( FALSE ),
									   m_RegisterTestRan                 ( FALSE ) 
        	{
        	}

        	DiagnosticTesting::DiagnosticTest::TestState RunTest( UINT32 & rErrorCode );

        protected:

        	void ConfigureForNextTestCycle();
	
        private:
        	//
        	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
        	// If using C++ 11 and later use the delete keyword to do this.
        	//
        	BlackfinDiagRegistersTest(const BlackfinDiagRegistersTest &);
	
        	const BlackfinDiagRegistersTest & operator = (const BlackfinDiagRegistersTest & );
	
            BlackfinDiagRegistersTest();
           
        	BOOL                              m_SanityTestRan;
        	
        	BOOL                              m_RegisterTestRan;
    
        	INT                               m_Critical;                    // Temp to allow disabling interrupts around critical sections 

        	inline void DisableInterrupts() 
        	{
        		m_Critical = cli();
        	}

        	inline void EnableInterrupts() 
        	{
        		sti(m_Critical);
        	}
    
    };


};


