/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagTest.hpp
///
/// Namespace for the base class for all Blackfin diagnostic tests
///
/// @par Full Description
/// 
/// Contains the namespace for the base class, BlackfinDiagTest, for Blackfin Diagnostics.  All diagnostic tests are 
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

#if !defined(BLACKFIN_DIAG_TEST_HPP)
#define BLACKFIN_DIAG_TEST_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include "Defs.h"
#include "DiagnosticDefs.h"
 
// C++ PROJECT INCLUDES
// (none)

// FORWARD REFERENCES
// (none)

namespace BlackfinDiagTesting 
{
    class BlackfinDiagTest 
    {
        public:


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
            BlackfinExecTestData;    

    
            //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTest: BlackfinDiagTest
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
	        BlackfinDiagTest( BlackfinExecTestData  newTestExec );		
			
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTest: GetCurrentTestState
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
            ///	METHOD NAME: BlackfinDiagTest: GetIterationCompletedTimestamp
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
            ///	METHOD NAME: BlackfinDiagTest: GetIterationPeriod
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
            ///	METHOD NAME: BlackfinDiagTest: GetMaxTimeBetweenTestCompletions
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
            ///	METHOD NAME: BlackfinDiagTest: GetNumberOfTimesToRunPerDiagCycle
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
            ///	METHOD NAME: BlackfinDiagTest: GetNumberOfTimesRanThisDiagCycle
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
            ///	METHOD NAME: BlackfinDiagTest: GetTestCompletedTimestamp
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
            ///	METHOD NAME: BlackfinDiagTest: GetTestStartTime
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
            ///	METHOD NAME: BlackfinDiagTest: GetTestType
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
            ///	METHOD NAME: BlackfinDiagTest: SetCurrentIterationDuration
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
            ///	METHOD NAME: BlackfinDiagTest: SetCurrentTestState
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
            ///	METHOD NAME: BlackfinDiagTest:SGetIterationCompletedTimestamp
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
            ///	METHOD NAME: BlackfinDiagTest: SetIterationPeriod
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
            ///	METHOD NAME: BlackfinDiagTest: SetMaxTimeBetweenTestCompletions
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
            ///	METHOD NAME: BlackfinDiagTest: SetNumberOfTimesRanThisDiagCycle
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
            ///	METHOD NAME: BlackfinDiagTest: SetTestCompletedTimestamp
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
            ///	METHOD NAME: BlackfinDiagTest: SetTestStartTime
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
            ///	METHOD NAME: BlackfinDiagTest: RunTest
            ///
            /// @par Full Description
            ///      A pure virtual method that inheriting classes must implement.  The scheduler will call this method 
            ///      to run iterations of the diagnostic test..
            ///      
            ///
            /// @param                            Reference to a possible error code returned from the scheduler.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            virtual TestState                                RunTest( UINT32 & rErrorCode ) = 0;

        protected:

            //***************************************************************************
            // PROTECTED METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTest: ConfigureForNextTestCycle
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
            ///	METHOD NAME: BlackfinDiagTest: ConfigForAnyNewDiagCycle
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
            void                                             ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
        private:
	
            BlackfinExecTestData                             m_TestExecutionData;
	
            //***************************************************************************
            // PRIVATE METHODS
            //***************************************************************************
            
            // Hide certain members of a class that are automatically created by the compiler if not defined;
            // Don't want the default constructer, the copy constructor, or the assignment operator to be 
            // accessable.
            BlackfinDiagTest();
	
            BlackfinDiagTest(const BlackfinDiagTest &);

            const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
            
    };
};

#endif // if !defined(BLACKFIN_DIAG_TEST_HPP)