#pragma once
#include "BlackfinDiagTest.hpp"
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */
	
namespace BlackfinDiagTesting 
{
    class BlackfinDiagDataRam : public BlackfinDiagTest 
    {
        public:
        
            

            typedef struct 
            {
    	        UINT8 * m_pDataRamAddressStart;
    	        UINT32  m_NmbrContiguousBytesToTest;
    	        UINT32  m_NmbrBytesTested;
    	        BOOL    m_TestCompleted;
            } 
            DataRamTestDescriptor;
            
    
	        BlackfinDiagDataRam( DataRamTestDescriptor      bankA,
	                             DataRamTestDescriptor      bankB,
	                             DataRamTestDescriptor      bankC,
	                             const UINT8 *              pTestPatternsForRamTesting,
	                             UINT32                     nmbrRamTestingPatterns,
	                             UINT32                     nmbrBytesToTestPerIteration,
	                             UINT32                     memoryBankFailureBitPos,
	                             UINT32                     testPatternsFailureBitPos,
		    		             BlackfinExecTestData &     rTestData )    
						         :  BlackfinDiagTest              ( rTestData ),
                                    m_Critical                    ( 0 ),                    
	                       	        m_BankA                       ( bankA ),
	                       	        m_BankB                       ( bankB ),
	                       	        m_BankC                       ( bankC ),
	                       	        m_MemoryBankFailureBitPos     ( memoryBankFailureBitPos ),
	                       	        m_NmbrBytesToTestPerIteration ( nmbrBytesToTestPerIteration ),
							        m_NmbrTestPatterns            ( nmbrRamTestingPatterns ),
							        m_pTestPatternsRAM            ( pTestPatternsForRamTesting ),
							        m_TestPatternsFailureBitPos   ( testPatternsFailureBitPos )
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
            INT                        m_Critical; 
            
            DataRamTestDescriptor  m_BankA;
            
            DataRamTestDescriptor  m_BankB;
            
            DataRamTestDescriptor  m_BankC;                   

            UINT32                     m_MemoryBankFailureBitPos;

	        UINT32                     m_NmbrTestPatterns;
	
	        UINT32                     m_NmbrBytesToTestPerIteration;

	        const UINT8 *              m_pTestPatternsRAM;

            UINT32                     m_TestPatternsFailureBitPos;

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

