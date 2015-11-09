#pragma once
#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */
	
namespace BlackfinDiagTesting 
{
    class BlackfinDiagDataRam : public BlackfinDiagTest 
    {
        public:

	        BlackfinDiagDataRam( BlackfinDataRamTestSuite * pDataRamTestSuite,
	                             const UINT8 *              pTestPatternsForRamTesting,
	                             UINT32                     nmbrRamTestingPatterns,
	                             UINT32                     nmbrBytesToTestPerIteration,
		    		             BlackfinExecTestData &     rTestData )    
						         :  BlackfinDiagTest              ( rTestData ),
                                    m_Critical                    ( 0 ),                    
	                       	        m_pDataRamTestSuite           ( pDataRamTestSuite ),
	                       	        m_NmbrBytesToTestPerIteration ( nmbrBytesToTestPerIteration ),
							        m_NmbrTestPatterns            ( nmbrRamTestingPatterns ),
							        m_pTestPatternsRAM            ( pTestPatternsForRamTesting )
	        {
	        }

	        virtual ~BlackfinDiagDataRam() 
	        {
	        }

	        virtual TestState RunTest( UINT32 & ErrorCode  );

 	        typedef struct 
 	        {
		              UINT8  * m_pByteToTest;
		              UINT8  * m_pPatternThatFailed;
		        const UINT8  * m_pTestPatterns;
		              UINT32   m_NmbrTestPatterns;
	        } 
	        ByteTestParameters;
	
        protected:

	        virtual void ConfigureForNextTestCycle();
	
        private:

	        //
	        // Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	        // If using C++ 11 and later use the delete keyword to do this.
	        //
	        BlackfinDiagDataRam(const BlackfinDiagDataRam &);
	
	        const BlackfinDiagDataRam & operator = (const BlackfinDiagDataRam & );
	

	        typedef enum 
	        { 
	        	BANK_A = 1, 
	        	BANK_B = 2, 
	        	BANK_C = 3 
	        } 
	        DataRamMemoryBanks;
		
	        //
	        // Which memory bank failed
	        //
	        static const UINT32        MEMORY_BANK_FAILURE_BIT_POS = 30;
	
	        static const UINT32        TEST_PATTERNS_ERROR_BIT_POS = 16;

            INT                        m_Critical;                    

	        BlackfinDataRamTestSuite * m_pDataRamTestSuite;

	        UINT32                     m_NmbrTestPatterns;
	
	        UINT32                     m_NmbrBytesToTestPerIteration;

	        const UINT8 *              m_pTestPatternsRAM;

	        void DisableInterrupts(); 
	
	        void EnableInterrupts();
	
            void EncodeErrorInfo( UINT32 &             rErrorInfo, 
                                  DataRamMemoryBanks   memoryBank, 
                                  UINT32               offsetFromBankStart, 
                                  UINT32               failurePattern );
                          
	        BOOL RunRamTest( DataRamTestDescriptor * pRamDescriptor, 
                             UINT32 &                rOffsetFromBankStart, 
                             UINT32 &                rFailurePattern );
                      
	       BOOL TestAByte(ByteTestParameters * pbtp);
    };
};
