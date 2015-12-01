/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticTesting.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Contains the namespace for the base class, DiagnosticTest.  All diagnostic tests are 
/// derived from the base class.  The base class specifies two pure virtual methods which the derived class must 
/// implement.  These methods are RunTest and ConfigForNextTestCycle.  Also the attributes that the scheduler needs
/// to schedule and monitor individual tests are contained within the base class.  The attributes are accessed via
/// Set and Get methods.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(DIAGNOSTIC_TESTING_HPP)
#define DIAGNOSTIC_TESTING_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include "Defs.h"
 
// C++ PROJECT INCLUDES
// (none)

// FORWARD REFERENCES
// (none)

namespace DiagnosticTesting 
{
    class DiagnosticTest 
    {
        public:

            // Mask and bit position for configuring error codes when errors are detected
            enum
            {
                DIAG_ERROR_MASK         = 0x7fffffff,
                DIAG_ERROR_TYPE_BIT_POS = 26
            };
                
            // Diagnostic test states
            typedef enum _TestState 
            {
                TEST_LOOP_COMPLETE,
                TEST_IN_PROGRESS,
                TEST_FAILURE,
                TEST_IDLE
            } 
            TestState;
	        
            // Test type enumeration.  Each diagnostic has a unique test type
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
	
            // Attributes contained in a struct that scheduler uses to execute and monitor tests
            typedef struct 
            {
    	        UINT32	                                     m_IterationPeriod;
                UINT64                                       m_IterationCompleteTimestamp;
                UINT64                                       m_TestCompleteTimestamp;
                UINT32                                       m_MaximumTimeBetweenTestCompletions;
                UINT32                                       m_CurrentIterationDuration;
                UINT64                                       m_TestStartTimestamp;
   		        UINT32              				         m_NmbrTimesToRunPerDiagCycle;  		// Number of times to run the test per diagnostic cycle
   		        UINT32		               			         m_NmbrTimesRanThisDiagCycle;   		// Times test has run this cycle
		        DiagnosticTestTypes 				         m_TestType;
                TestState                                    m_CurrentTestState;	
            } 
            ExecuteTestData;    

    
            //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
			
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetCurrentTestState
            ///
            /// @par Full Description
            ///      Get the current TestState for the test.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           Current testing state of the test.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            TestState                                        GetCurrentTestState();
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetIterationCompletedTimestamp
            ///
            /// @par Full Description
            ///      Get the timestamp for when the iteration completes.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The timestamp when the iteration of the test completed.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT64                                           GetIterationCompletedTimestamp();

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetIterationPeriod
            ///
            /// @par Full Description
            ///      Get the period of time between individual test iterations for the test.  Some done run iteratively
            ///      though.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The period of time between individual test iterations.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT32                                           GetIterationPeriod();
	        
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetMaxTimeBetweenTestCompletions
            ///
            /// @par Full Description
            ///      Get the period of time that has been recorded for the maximum time period the test has run in.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The maximum period of time the test has run in.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT32                                           GetMaxTimeBetweenTestCompletions();
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetNumberOfTimesToRunPerDiagCycle
            ///
            /// @par Full Description
            ///      Get the number of times the test needs to run per diagnostic cycle.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The number of times the test has completed this diagnostic cycle
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT32                                           GetNumberOfTimesToRunPerDiagCycle();
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetNumberOfTimesRanThisDiagCycle
            ///
            /// @par Full Description
            ///      Get the number of times the test has completed during the current diagnostic cycle.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The number of times the test has completed this diagnostic cycle
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT32                                           GetNumberOfTimesRanThisDiagCycle();
	        
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetTestCompletedTimestamp
            ///
            /// @par Full Description
            ///      Get the timestamp of when the test completed for the diagnostic cycle
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The timestamp of when the test completed during the cycle
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        UINT64                                           GetTestCompletedTimestamp();
	        
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetTestStartTime
            ///
            /// @par Full Description
            ///      Get the timestamp of when the test started its first iteration in the diagnostic cycle.
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The timestamp of when the test completed during the cycle
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            UINT64                                           GetTestStartTime();
            	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: GetTestType
            ///
            /// @par Full Description
            ///      Get DiagnosticTestType that is associated with the test
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           DiagnosticTestType that is associated with the test
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        DiagnosticTestTypes                              GetTestType();
	        
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetCurrentIterationDuration
            ///
            /// @par Full Description
            ///      Set the elapsed time from the start of the test that this iteration has consummend
            ///      
            ///
            /// @param                            None
            ///                               
            /// @return                           The elapsed time from the start of the test that this iteration has 
            ///                                   consummend
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        void                                             SetCurrentIterationDuration( UINT32 duration );
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetCurrentTestState
            ///
            /// @par Full Description
            ///      Set the current TestState for the test.
            ///      
            ///
            /// @param                            The current TestState of the test
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        void                  					         SetCurrentTestState( TestState ts );
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest:SGetIterationCompletedTimestamp
            ///
            /// @par Full Description
            ///      Saves the timestamp for when the iteration completes.
            ///      
            ///
            /// @param                            The timestamp for when the iteration completed.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void				                             SetIterationCompletedTimestamp( UINT64 timestamp);

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetIterationPeriod
            ///
            /// @par Full Description
            ///      Set the period of time between individual test iterations for the test.  Some done run iteratively
            ///      though.
            ///      
            ///
            /// @param                            The period of time between individual test iterations.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void			                                 SetIterationPeriod( UINT32 period );
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetMaxTimeBetweenTestCompletions
            ///
            /// @par Full Description
            ///      Set the period of time that has been recorded for the maximum time period the test has run in.
            ///      
            ///
            /// @param                            The maximum period of time the test has run in
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        void                                             SetMaxTimeBetweenTestCompletions( UINT32 period );
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetNumberOfTimesRanThisDiagCycle
            ///
            /// @par Full Description
            ///      Set the number of times the test has completed during the current diagnostic cycle.
            ///      
            ///
            /// @param                            The number of times the test has completed this diagnostic cycle.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                             SetNumberOfTimesRanThisDiagCycle(UINT32);
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetTestCompletedTimestamp
            ///
            /// @par Full Description
            ///      Set the timestamp of when the test completed for the diagnostic cycle
            ///      
            ///
            /// @param                            The timestamp of when the test completed during the cycle
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                             SetTestCompletedTimestamp( UINT64 timestamp );
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: SetTestStartTime
            ///
            /// @par Full Description
            ///      Set the timestamp of when the test started its first iteration in the diagnostic cycle.
            ///      
            ///
            /// @param                            The timestamp of when the test completed during the cycle
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                             SetTestStartTime( UINT64 time);
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: RunTest
            ///
            /// @par Full Description
            ///      A pure virtual method that inheriting classes must implement.  The scheduler will call this method 
            ///      to run iterations of the diagnostic test..
            ///      
            ///
            /// @param                            Reference to a possible error code returned from the scheduler.
            ///                               
            /// @return                           Status of executing a test
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            virtual DiagnosticTest::TestState                                RunTest( UINT32 & rErrorCode ) = 0;

        protected:

            //***************************************************************************
            // PROTECTED METHODS
            //***************************************************************************
            
             ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: DiagnosticTest
            ///
            /// @par Full Description
            ///      Construction that is used to construct the base class componenent of the inheriting test.
            ///      This the one and only method for instantiating the base class component..
            ///      
            ///
            /// @param newTestExec                Initial configuration data for the base class that the scheduler uses
            ///                               
            /// @return                           Base class component data initialized.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        DiagnosticTest( ExecuteTestData  newTestExec );		
	        
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: ConfigureForNextTestCycle
            ///
            /// @par Full Description
            ///      A pure virtual method that inheriting classes must implement. This method is called at that start
            ///      of testing for the test during a new diagnostics cycle. The data that needs to be initialized for
            ///      an individual test is initialized.      
            ///
            /// @param                            None.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            virtual void 		                             ConfigureForNextTestCycle() = 0;	

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnosticTest: ConfigForAnyNewDiagCycle
            ///
            /// @par Full Description
            ///       This method is called at that start of testing by the scheduler for each test    
            ///
            /// @param                            Pointer to the test object that will have its data initialized by the 
            ///                                   ConfigureForNexCycleCall.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                             ConfigForAnyNewDiagCycle( DiagnosticTest * btd ); 
	
        private:
	
            ExecuteTestData                             m_TestExecutionData;
	
            //***************************************************************************
            // PRIVATE METHODS
            //***************************************************************************
            
            // Hide certain members of a class that are automatically created by the compiler if not defined;
            // Don't want the default constructer, the copy constructor, or the assignment operator to be 
            // accessable.
            DiagnosticTest();
	
            DiagnosticTest(const DiagnosticTest &);

            const DiagnosticTest & operator = (const DiagnosticTest &);
            
    };
};

#endif // if !defined(DIAGNOSTIC_TESTING_HPP)




