#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */

	
namespace BlackfinDiagTests {
class BlackfinDiagDataRam : public BlackfinDiagTest {

public:

	BlackfinDiagDataRam( BlackfinDataRamTestSuite * dataRamTestSuite,
	                     const UINT8 *              testPatternsForRamTesting,
	                     UINT32                     nmbrRamTestingPatterns,
	                     UINT32                     nmberBytesToTestPerIteration,
		    		     BlackfinExecTestData &     testData )    
						 :  BlackfinDiagTest             ( testData ),
                            critical_                    ( 0 ),                    
	                       	dataRamTestSuite_            ( dataRamTestSuite ),
	                       	nmbrBytesToTestPerIteration_ ( nmberBytesToTestPerIteration ),
							nmbrTestPatterns_            ( nmbrRamTestingPatterns ),
							testPatternsRAM_             ( testPatternsForRamTesting )
	{}

	virtual ~BlackfinDiagDataRam() {}

	virtual TestState RunTest( UINT32 &                     ErrorCode, 
	                           DiagnosticCommon::DiagTime_t TimeTestStarted_milleseconds  );

 	typedef struct {
		      UINT8  * ptrByteToTest;
		      UINT8  * ptrPatternThatFailed;
		const UINT8  * ptrTestPatterns;
		      UINT32   nmbrTestPatterns;
	} ByteTestParameters;
	
protected:

	virtual void ConfigureForNextTestCycle();
	
private:

	//
	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	// If using C++ 11 and later use the delete keyword to do this.
	//
	BlackfinDiagDataRam(const BlackfinDiagDataRam & other);
	
	const BlackfinDiagDataRam & operator = (const BlackfinDiagDataRam & );
	

	typedef enum { BankA = 1, BankB = 2, BankC = 3 } DataRamMemoryBanks;
		
	//
	// Which memory bank failed
	//
	static const UINT32        memoryBankFailureBitPos_ = 30;
	
	static const UINT32        testPatternErrorBitPos_ = 16;




	INT                        critical_;                    

	BlackfinDataRamTestSuite * dataRamTestSuite_;

	UINT32                     nmbrTestPatterns_;
	
	UINT32                     nmbrBytesToTestPerIteration_;

	const UINT8 *              testPatternsRAM_;

	void DisableInterrupts() {
		critical_ = cli();
	}

	void EnableInterrupts() {
		sti(critical_);
	}

    void EncodeErrorInfo( UINT32 &             errorInfo, 
                          DataRamMemoryBanks   memoryBank, 
                          UINT32               offsetFromBankStart, 
                          UINT32 FailurePattern );
                          
	BOOL RunRamTest( DataRamTestDescriptor * ramDescriptor, 
                     UINT32 &                offsetFromBankStart, 
                     UINT32 &                failurePattern );
                      
	BOOL TestAByte(ByteTestParameters * pbtp);
};
};

