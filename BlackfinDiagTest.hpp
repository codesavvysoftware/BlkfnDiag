#pragma once
#include "BlackfinDiag.hpp"
#include "DiagnosticDefs.h"

namespace BlackfinDiagTesting 
{
    class BlackfinDiagTest 
    {
        public:

	        typedef enum _DiagnosticTestTypes 
	        {
		        DIAG_DATA_RAM_TEST_TYPE       = 1,
		        DIAG_INTRUCTION_RAM_TEST_TYPE = 2,
		        DIAG_REGISTER_TEST_TEST_TYPE  = 3,
		        DIAG_SCHEDULER_TEST_TYPE      = 4,
		        DIAG_TIMER_TEST_TYPE          = 5,
		        DIAG_INSTRUCTIONS_TEST_TYPE   = 6,
		        DIAG_NO_TEST_TYPE             = 7
	        } 
	        DiagnosticTestTypes;
	
            typedef struct 
            {
    	        DiagElapsedTime	                             m_IterationPeriod;
                DiagTimestampTime                            m_IterationCompleteTimestamp;
                DiagTimestampTime                            m_TestCompleteTimestamp;
                DiagElapsedTime                              m_MaximumTimeBetweenTestCompletions;
                DiagElapsedTime                              m_CurrentIterationDuration;
                DiagTimestampTime                            m_TestStartTimestamp;
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

            DiagElapsedTime                                  GetCurrentIterationDuration();
            
            TestState                                        GetCurrentTestState();
	
	        DiagTimestampTime                                GetIterationCompletedTimestamp();

	        DiagElapsedTime                                  GetIterationPeriod();
	        
	        DiagElapsedTime                                  GetMaxTimeBetweenTestCompletions();
	
	        UINT32                                           GetNumberOfTimesToRunPerDiagCycle();
	
	        UINT32                                           GetNumberOfTimesRanThisDiagCycle();
	        
	        DiagTimestampTime                                GetTestCompletedTimestamp();
	        
            DiagTimestampTime                                GetTestStartTime();
            	
	        DiagnosticTestTypes                              GetTestType();
	        
	        void                                             SetCurrentIterationDuration( DiagElapsedTime duration );
	
	        void                  					         SetCurrentTestState( TestState ts );
	
            void				                             SetIterationCompletedTimestamp( DiagTimestampTime timestamp);

            void			                                 SetIterationPeriod( DiagElapsedTime period );
	
	        void                                             SetMaxTimeBetweenTestCompletions( DiagElapsedTime period );
	
            void                                             SetNumberOfTimesToRunPerDiagCycle(UINT32);
	
            void                                             SetNumberOfTimesRanThisDiagCycle(UINT32);
            
            void                                             SetTestCompletedTimestamp( DiagTimestampTime timestamp );
	
            void                                             SetTestStartTime( DiagTimestampTime time);
            
            void				                             SetTestType(DiagnosticTestTypes test_type);

            virtual TestState                                RunTest( UINT32 & errorCode ) = 0;

        protected:

            virtual void 		                             ConfigureForNextTestCycle() = 0;	

            void                                             ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
        private:
	
            BlackfinExecTestData                             m_TestExecutionData;
	
            BlackfinDiagTest();
	
            BlackfinDiagTest(const BlackfinDiagTest &);

            const BlackfinDiagTest & operator = (const BlackfinDiagTest &);

    };
};

