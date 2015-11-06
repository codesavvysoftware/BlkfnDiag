#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

namespace BlackfinDiagTesting {

class BlackfinDiagTimerTest : public BlackfinDiagTest {


public:

	BlackfinDiagTimerTest( UINT32                              errorApexTimer,
					       UINT32                              errorHostTimer,
					       DiagnosticCommon::DiagElapsedTime_t maxElapsedTimeApex,
	                       DiagnosticCommon::DiagElapsedTime_t maxElapsedTimeHost,
	                       DiagnosticCommon::DiagElapsedTime_t minElapsedTimeApex,
	                       DiagnosticCommon::DiagElapsedTime_t minElapsedTimeHost, 
	                       BlackfinExecTestData &              testData ) 
   					   	    :  BlackfinDiagTest      ( testData ), 
   					   	       errorApexTimer_       ( errorApexTimer ),
   					   	       errorHostTimer_       ( errorHostTimer ),  
						       beingInstantiated_    ( TRUE ),
						       maxElapsedTimeApex_   ( maxElapsedTimeApex ),
						       maxElapsedTimeHost_   ( maxElapsedTimeHost ),
						       minElapsedTimeApex_   ( minElapsedTimeApex ),
						       minElapsedTimeHost_   ( minElapsedTimeHost ),
                               timerValueStartApex_  ( 0 ),
						       timerValueStartHost_  ( 0 )
	{}

	virtual ~BlackfinDiagTimerTest() {}

	virtual TestState RunTest( UINT32 & ErrorCode  );
	
protected:

	virtual void ConfigureForNextTestCycle();
	
private:

	//
	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	// If using C++ 11 and later use the delete keyword to do this.
	//
	BlackfinDiagTimerTest(const BlackfinDiagTimerTest &);
	
	const BlackfinDiagTimerTest & operator = (const BlackfinDiagTimerTest & );
		
	BOOL                                    beingInstantiated_;
 
	UINT32                                  errorApexTimer_;
	
	UINT32									errorHostTimer_;
	
	DiagnosticCommon::DiagElapsedTime_t     maxElapsedTimeApex_;
	
	DiagnosticCommon::DiagElapsedTime_t     maxElapsedTimeHost_;	// Read the current Apex2 System Time Register value.
    
	DiagnosticCommon::DiagElapsedTime_t     minElapsedTimeApex_;
	
	DiagnosticCommon::DiagElapsedTime_t     minElapsedTimeHost_;	// Read the current Apex2 System Time Register value.
    
	DiagnosticCommon::DiagTimestampTime_t   timerValueStartApex_;
        
    DiagnosticCommon::DiagTimestampTime_t   timerValueStartHost_;
};
};

