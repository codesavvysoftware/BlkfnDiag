#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */


namespace BlackfinDiagTests {
	
class BlackfinDiagRegistersTest : public BlackfinDiagTest {

public:
	BlackfinDiagRegistersTest( 	const RegisterTestDescriptor      registerTestSuite[],
	                            UINT32                            numberOfDescriptorsInTestSuite, 
								const UINT32                      testPatterns[], 
								UINT32                            numberOfPatterns,
		    		     		BlackfinExecTestData &     testData ) 
		    		     		:	BlackfinDiagTest             	( testData ),
									corruptedRegisterTestSuite_  	( 0xff ),
									numberOfRegisterPatterns_       ( numberOfPatterns ), 
									testPatternsForRegisterTesting_ ( testPatterns ),
									numberOfRegisterTests_          ( numberOfDescriptorsInTestSuite),
									registerTestSuite_              ( registerTestSuite ) 
	{}

	virtual ~BlackfinDiagRegistersTest() {}

	virtual TestState RunTest( UINT32 & ErrorCode );

protected:

	virtual void ConfigureForNextTestCycle();
	
private:
	//
	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	// If using C++ 11 and later use the delete keyword to do this.
	//
	BlackfinDiagRegistersTest(const BlackfinDiagRegistersTest & other);
	
	const BlackfinDiagRegistersTest & operator = (const BlackfinDiagRegistersTest & );
	

	const UINT32                      corruptedRegisterTestSuite_;

    const UINT32                      numberOfRegisterPatterns_;

	const UINT32                      numberOfRegisterTests_;
	
	const RegisterTestDescriptor *    registerTestSuite_;
	
    const UINT32 *                    testPatternsForRegisterTesting_;
    
	INT                               critical_;                    // Temp to allow disabling interrupts around critical sections 

	BOOL FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & rtdTests );
	
	void DisableInterrupts() {
		critical_ = cli();
	}

	void EnableInterrupts() {
		sti(critical_);
	}
    
    BOOL RunRegisterTests( RegisterTestDescriptor  * rtdTests, UINT32 & FailureInfo );
};


};


