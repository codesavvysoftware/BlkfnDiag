////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagEcc.hpp
///
/// @details ApexDiagEcc class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     25-NOV-2013  Created. 
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGECC_HPP
#define APEXDIAGECC_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
// (none)
#include "ApexStd.hpp"                       // for UINT32 etc.
#include "ApexParameters.hpp"                // for DiagControlBlock
#include "ApexDiagnostic.hpp"                // TEST_LOOP_COMPLETE

// FORWARD REFERENCES
// (none)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///@begin_claim_coding_exception
///
/// Rule 3.3.12 Const Variables, 3.3.16 Macro and #define Names, MISRA 16-0-4
///
/// It's easier and more readable to use #define and a macro to check address validity
/// on compile time than static const variable.
///
//@end_claim_coding_exception
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///@begin_code_exception
// Macro checking if address is in Apex2 memory area designated for ecc tests
// with bit 15 set and aligned to 4 - useful for comparing to ecc error address
// form SramEccError register
#define DGN_IS_VALID_ECC_TEST_ADDR(_addr) \
                ( (_addr) >= (DGN_ECC_TEST_RAM_START) \
                && (_addr) < (DGN_ECC_TEST_RAM_END) \
                && (((_addr) & 0x8000) > 0) \
                && (((_addr) % 4) == 0))

#define DGN_ECC_TEST_PTR1 (DGN_ECC_TEST_RAM_START)
#define DGN_ECC_TEST_PTR2 (DGN_ECC_TEST_PTR1 + 4)
#define DGN_ECC_TEST_PTR3 (DGN_ECC_TEST_PTR2 + 4)
#define DGN_ECC_TEST_PTR4 (DGN_ECC_TEST_PTR3 + 4)
#define DGN_ECC_TEST_PTR5 (DGN_ECC_TEST_PTR4 + 4)
#define DGN_ECC_TEST_PTR6 (DGN_ECC_TEST_PTR5 + 4)

#if ( !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR1) \
    || !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR2) \
    || !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR3) \
    || !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR4) \
    || !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR5) \
    || !DGN_IS_VALID_ECC_TEST_ADDR (DGN_ECC_TEST_PTR6))
#error DGN_ECC_TEST_PTR outside ecc test memory area or not correctly aligned!
#endif
///@end_code_exception

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagEcc
///
/// @ingroup 
///
/// @brief Apex ecc circuit diagnotic class.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagEcc
{
public:
    // PUBLIC METHODS
    
    //  DgnEccTest
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);

    //  DgnInitECCTest
    static void Init();

    //  DgnPowerupECCTest
    static void PowerupTest();

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    struct EccTestData 
    {
        UINT32 *ptr;                // pointer to the memory cell the test uses
        UINT32  seedValue;          // value to seed ecc error with
        UINT32  expectedValue;      // value that should be read (corrected by ecc circuit)
        bool    singleBitError;     // single or multibit error seeded
    };

    // number of tests to fail before faulting - this is used to prevent random
    // errors in seeded areas to cause diagnostics to assert
    static const UINT32 ECC_ERRORS_THRESHOLD    = 2;

    static const UINT32 ECC_TEST_ARRAY_SIZE     = 3;

    // single ecc error test data
    static const EccTestData SINGLE_ERROR_DATA[ECC_TEST_ARRAY_SIZE];

    // double ecc error test data
    static const EccTestData DOUBLE_ERROR_DATA[ECC_TEST_ARRAY_SIZE];

    static bool m_IrqEnabledBeforeTestMode;

    /// Default constructor.
    ApexDiagEcc();
    
    /// Default destructor.
    ~ApexDiagEcc();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagEcc(const ApexDiagEcc &);
    ApexDiagEcc &operator=(const ApexDiagEcc &);
    
    static bool VerifyEccErrors(const EccTestData* testData);

    /// This method scrubs memory cell where ecc error occured.
    static void ScrubECCError(void);

    /// This method enters ecc test mode.
    static void EnterECCTestMode(void);

    /// This method exits ecc test mode on Apex2.
    static void ExitECCTestMode(void);

    /// This method plants all ECC errors in defined memory cells.
    static void PlantECCErrors(void);

    /// This method re-plants ECC error in given memory cell.
    static void ReplantEccError(const EccTestData* testData);

    /// This method test single memory cell.
    static bool TestCell(const EccTestData* testData);

    /// This method checks if ecc errors are correctly detected (and corrected if required). 
    static void CheckErrors(EccTestData const (&testDataArray)[ECC_TEST_ARRAY_SIZE]);

    /// This method checks the ECC circuitry on the internal RAM.
    static void EccTestLocations(void);
    
    /// This methods asserts if given bit is not set in AsicCtrl register.
    static inline void AssertBitSet(const UINT32 bit);
    
    /// This methods asserts if given bit is not clear in AsicCtrl register.
    static inline void AssertBitClear(const UINT32 bit);

    // MEMBER VARIABLES

};

/// This methods asserts if given bit is not set in AsicCtrl register.
inline void ApexDiagEcc::AssertBitSet(const UINT32 bit)
{
    UINT32 bitState = HI_ApexReg.AsicCtrl & bit;
    ASSERT(0 != bitState);
}

/// This methods asserts if given bit is not clear in AsicCtrl register.
inline void ApexDiagEcc::AssertBitClear(const UINT32 bit)
{
    UINT32 bitState = HI_ApexReg.AsicCtrl & bit;
    ASSERT(0 == bitState);
}

#endif // #if !defined(APEXDIAGECC_HPP) 
