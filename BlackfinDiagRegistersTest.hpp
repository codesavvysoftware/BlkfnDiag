#pragma once
//#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


namespace BlackfinDiagTesting 
{
    class BlackfinDiagRegistersTest : public BlackfinDiagTest 
    {

        public:

            typedef UINT32 (* const pRegisterTest)();
	
			BlackfinDiagRegistersTest( pRegisterTest           pRegisterSanityTest,
			                           pRegisterTest           pRegisterTest, 
                                       UINT32                  corruptedRegTestErr,
                                       BlackfinExecTestData &  rTestData ) 
		    		     		:	BlackfinDiagTest             ( rTestData ),
									m_CorruptedRegisterTestErr   ( corruptedRegTestErr ),
									m_pSanityTest                ( pRegisterSanityTest ),
									m_pRegisterTest              ( pRegisterTest ),
									m_SanityTestRan              ( FALSE ),
									m_RegisterTestRan            ( FALSE ) 
        	{
        	}

        	virtual ~BlackfinDiagRegistersTest() 
        	{
        	}

        	virtual TestState RunTest( UINT32 & rErrorCode );

        protected:

        	virtual void ConfigureForNextTestCycle();
	
        private:
        	//
        	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
        	// If using C++ 11 and later use the delete keyword to do this.
        	//
        	BlackfinDiagRegistersTest(const BlackfinDiagRegistersTest &);
	
        	const BlackfinDiagRegistersTest & operator = (const BlackfinDiagRegistersTest & );
	

        	const UINT32                      m_CorruptedRegisterTestErr;
        	
        	pRegisterTest                     m_pSanityTest;
        	
        	pRegisterTest                     m_pRegisterTest;
        	
        	BOOL                              m_SanityTestRan;
        	
        	BOOL                              m_RegisterTestRan;

    
        	INT                               m_Critical;                    // Temp to allow disabling interrupts around critical sections 

        	void DisableInterrupts() 
        	{
        		m_Critical = cli();
        	}

        	void EnableInterrupts() 
        	{
        		sti(m_Critical);
        	}
    
    };


};


