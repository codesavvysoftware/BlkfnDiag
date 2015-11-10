#pragma once
#include "BlackfinDiag.hpp"

namespace BlackfinDiagTesting 
{
    class BlackfinDiagTest 
    {
        public:

            typedef enum _TestState 
            {
		        TEST_LOOP_COMPLETE,
		        TEST_IN_PROGRESS,
		        TEST_FAILURE,
		        TEST_IDLE
	        } 
	        TestState;

	        typedef enum _DiagnosticTestTypes 
	        {
		        DIAG_DATA_RAM_TEST_TYPE       = 1,
		        DIAG_INTRUCTION_RAM_TEST_TYPE = 2,
		        DIAG_REGISTER_TEST_TEST_TYPE  = 3,
		        DIAG_SCHEDULER_TEST_TYPE      = 4,
		        DIAG_TIMER_TEST_TYPE          = 5,
		        DIAG_INSTRUCTION_TEST_TYPE    = 6
	        } 
	        DiagnosticTestTypes;
	
            typedef UINT32 (* const pRegisterTest)(const UINT32 *, UINT32);
	
            typedef struct 
            {
    	        const pRegisterTest * m_pRegisterTests;
                UINT32                m_NmbrRegisterTests;
		        BOOL                  m_TestsCompleted;
            }  
            RegisterTestDescriptor;
    
		
            typedef struct 
            {
    	        UINT8 * m_pDataRamAddressStart;
    	        UINT32  m_NmbrContiguousBytesToTest;
    	        UINT32  m_NmbrBytesTested;
    	        BOOL    m_TestCompleted;
            } 
            DataRamTestDescriptor;
    
            typedef struct 
            {
    	        DataRamTestDescriptor  m_BankA;
    	        DataRamTestDescriptor  m_BankB;
    	        DataRamTestDescriptor  m_BankC;
            } 
            BlackfinDataRamTestSuite;
    
            typedef struct 
            {
    	        DiagnosticCommon::DiagElapsedTime	         m_IterationPeriod;
                DiagnosticCommon::DiagTimestampTime          m_IterationCompleteTimestamp;
   		        UINT32              				         m_NmbrTimesToRunPerDiagCycle;  		// Number of times to run the test per diagnostic cycle
   		        UINT32		               			         m_NmbrTimesRanThisDiagCycle;   		// Times test has run this cycle
		        DiagnosticTestTypes 				         m_TestType;
                TestState                                    m_CurrentTestState;	
            } 
            BlackfinExecTestData;    

    
	        BlackfinDiagTest( BlackfinExecTestData & newTestExec );		
			
	        virtual ~BlackfinDiagTest()  
	        {
	        }

            TestState                                        GetCurrentTestState();
	
	        DiagnosticCommon::DiagTimestampTime              GetIterationCompletedTimestamp();

	        DiagnosticCommon::DiagElapsedTime                GetIterationPeriod();
	
	        UINT32                                           GetNumberOfTimesToRunPerDiagCycle();
	
	        UINT32                                           GetNumberOfTimesRanThisDiagCycle();
	
	        DiagnosticTestTypes                              GetTestType();
	
	        void                  					         SetCurrentTestState(TestState);
	
            void				                             SetIterationCompletedTimestamp(DiagnosticCommon::DiagTimestampTime timestamp);

            void			                                 SetIterationPeriod(DiagnosticCommon::DiagElapsedTime period);
	
            void                                             SetNumberOfTimesToRunPerDiagCycle(UINT32);
	
            void                                             SetNumberOfTimesRanThisDiagCycle(UINT32);
	
            void				                             SetTestType(DiagnosticTestTypes test_type);

            virtual TestState                                RunTest( UINT32 & errorCode ) = 0;

        protected:

            virtual void 		                             ConfigureForNextTestCycle() = 0;	

            void                                             ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
        private:
	
            static const DiagnosticCommon::DiagElapsedTime   DIAG_TIME_SLICE_PERIOD_MILLESECONDS = 50;  // Resolution of microseconds

            BlackfinExecTestData                             m_TestExecutionData;
	
            BlackfinDiagTest();
	
            BlackfinDiagTest(const BlackfinDiagTest &);

            const BlackfinDiagTest & operator = (const BlackfinDiagTest &);

    };
};

