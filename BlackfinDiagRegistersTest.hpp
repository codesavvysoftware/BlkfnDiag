#pragma once
//#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


namespace BlackfinDiagTesting 
{
    class BlackfinDiagRegistersTest : public BlackfinDiagTest 
    {

        public:

            typedef UINT32 (* const pRegisterTest)(const UINT32 *, UINT32);
	
			typedef struct
			{
				const pRegisterTest * m_pRegisterTests;
				UINT32                m_NmbrRegisterTests;
				BOOL                  m_TestsCompleted;
			}
			RegisterTestDescriptor;

			BlackfinDiagRegistersTest( const RegisterTestDescriptor  registerTestSuite[],
                                             UINT32                  numberOfDescriptorsInTestSuite, 
                                       const UINT32                  testPatterns[], 
                                             UINT32                  numberOfPatterns,
                                             UINT32                  corruptedRegTestSuiteErr,
                                             BlackfinExecTestData &  rTestData ) 
		    		     		:	BlackfinDiagTest             	  ( rTestData ),
									m_CorruptedRegisterTestSuiteErr   ( corruptedRegTestSuiteErr ),
									m_NmbrOfRegisterPatterns          ( numberOfPatterns ), 
									m_pTestPatternsForRegisterTesting ( testPatterns ),
									m_NmbrOfRegisterTests             ( numberOfDescriptorsInTestSuite),
									m_pRegisterTestSuite              ( registerTestSuite ) 
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
	

        	const UINT32                      m_CorruptedRegisterTestSuiteErr;

            const UINT32                      m_NmbrOfRegisterPatterns;

        	const UINT32                      m_NmbrOfRegisterTests;
	
        	const RegisterTestDescriptor *    m_pRegisterTestSuite;
	
            const UINT32 *                    m_pTestPatternsForRegisterTesting;
    
        	INT                               m_Critical;                    // Temp to allow disabling interrupts around critical sections 

        	BOOL FindTestToRun( RegisterTestDescriptor * & rtdTests );
	
        	void DisableInterrupts() 
        	{
        		m_Critical = cli();
        	}

        	void EnableInterrupts() 
        	{
        		sti(m_Critical);
        	}
    
            BOOL RunRegisterTests( RegisterTestDescriptor  * rtdTests, UINT32 & FailureInfo );
    };


};


