/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagEcc.cpp
///
/// Implementation of the ECC internal RAM diagnostics for APEX.
///
/// @see ApexDiagECC.hpp for additional information.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - spolke  29-APR-2013 Initial revision of file.
/// - spolke  10-AUG-2013 ECC test fixes.
/// - pgrzywn 14-OCT-2013 Fault injection point added.
/// - pszramo 12-NOV-2013 Fault injection point added.
/// - spolke  27-NOV-2013 ECC diagnostic moved to a class.
/// - pszramo 11-DEC-2013 Updates to fault injection points according to
///                       Coverity defects triage.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
/// @ingroup APX
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "Apex.hpp"                     // for HI_CleanCacheAddr etc.
#include "ApexDiagEcc.hpp"              // for ApexDiagEcc class

// FORWARD REFERENCES
// (none)

//******************************************************************************
// STATIC VARIABLES
//******************************************************************************

// single ECC error test data
ApexDiagEcc::EccTestData const ApexDiagEcc::SINGLE_ERROR_DATA[ECC_TEST_ARRAY_SIZE] =
{
        // ptr                       seedValue   expected    singleBitError
        {(UINT32*)DGN_ECC_TEST_PTR1, 0x00000001, 0x00000000, true},
        {(UINT32*)DGN_ECC_TEST_PTR2, 0x00000100, 0x00000000, true},
        {(UINT32*)DGN_ECC_TEST_PTR3, 0x00010000, 0x00000000, true},
};

// double ECC error test data
ApexDiagEcc::EccTestData const ApexDiagEcc::DOUBLE_ERROR_DATA[ECC_TEST_ARRAY_SIZE] =
{
        // ptr                       seedValue   expected    singleBitError
        {(UINT32*)DGN_ECC_TEST_PTR4, 0x00000003, 0x00000000, false},
        {(UINT32*)DGN_ECC_TEST_PTR5, 0x00000300, 0x00000000, false},
        {(UINT32*)DGN_ECC_TEST_PTR6, 0x00030000, 0x00000000, false},
};

bool ApexDiagEcc::m_IrqEnabledBeforeTestMode = false;

//******************************************************************************
// PRIVATE METHODS
//******************************************************************************

/////////////////////////////////////////////////////////////////////////////
//  VerifyEccErrors
///
/// This routine checks SramEccError register to verify that it has
/// correctly logged a ECC error.
///
/// @param  testData Test parameters.
/// @return bool     true if verification successful, false if not.
/////////////////////////////////////////////////////////////////////////////
bool ApexDiagEcc::VerifyEccErrors(const EccTestData* testData)
{
    UINT32 eccErrAddress = DGN_RAM_SPACE_START + (HI_ApexReg.SramEccError & HI_MASK_SRAMECC_OFST);
    UINT32 sramEccError  = HI_ApexReg.SramEccError;

    // Start Fault Injection Point 1
    // Code which modifies sramEccError to simulate that a seeded double-bit error 
    // is not detected at power-up will be injected here.
    // End Fault Injection Point 1

    // Start Fault Injection Point 2
    // Code which modifies sramEccError to simulate that a seeded single-bit error 
    // is not detected when InjectFaultFlag is set at run-time will be injected here.
    // End Fault Injection Point 2

    if ((sramEccError & HI_BIT_SRAMECC_PER)                 // if ECC error logged
            && (((UINT32)testData->ptr) == eccErrAddress)   // and at expected address
            && (testData->singleBitError ?
                !(sramEccError & HI_BIT_SRAMECC_DBL) :      // single-bit error
                 (sramEccError & HI_BIT_SRAMECC_DBL)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ScrubECCError
///
/// This routine scrubs memory cell where ECC error occurs and clears error in
/// SramEccError register.
///
/// @note To clear error in code space unlocking code space before calling is required.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::ScrubECCError(void)
{
    if (HI_ApexReg.SramEccError & HI_BIT_SRAMECC_PER)
    {
        // Get the address to scrub, and scrub it
        UINT32 offset = (HI_ApexReg.SramEccError & HI_MASK_SRAMECC_OFST);
        UINT32 volatile *ptr = (UINT32 volatile *) (DGN_RAM_SPACE_START + offset);

        // Scrub it
        *ptr = *ptr;

        // Clear the indication in the hardware by doing a write with any value
        HI_ApexReg.SramEccError = 0;

        // Clean the data cache at that address
        //  to guarantee RAM is written
        HI_CleanCacheAddr(ptr);
    }
}

/////////////////////////////////////////////////////////////////////////////
//  EnterECCTestMode
///
/// This routine enters ECC test mode on Apex2.
///
/// @full  From APEX specification:
///        The test condition requires cbmc to be set
///        to 0, ipen set to 1, and writes to a memory region with
///        address bit 15 set
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::EnterECCTestMode(void)
{
    // disable interrupts
    m_IrqEnabledBeforeTestMode = HI_DisableIrq();

    // Assure test mode is not enabled - this is the only place when it should be
    // entered - CMBC bit should be set
    AssertBitSet( HI_BIT_ASIC_CBMC );

    // enable test mode and turn off fault generation on double bit error
    HI_ApexReg.AsicCtrl &= ~(HI_BIT_ASIC_CBMC | HI_BIT_ASIC_PFLT);
}

/////////////////////////////////////////////////////////////////////////////
//  ExitECCTestMode
///
/// This exits ECC test mode on Apex2.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::ExitECCTestMode(void)
{
    // Test mode should be enabled - CMBC bit should be clear
    AssertBitClear( HI_BIT_ASIC_CBMC );

    // Restore original state
    HI_ApexReg.AsicCtrl |= HI_BIT_ASIC_PFLT | HI_BIT_ASIC_CBMC;

    // Clear error that happen during write
    HI_ApexReg.SramEccError = 0;

    if (m_IrqEnabledBeforeTestMode)
    {
        HI_EnableIrq();
    }
}

/////////////////////////////////////////////////////////////////////////////
//  PlantECCErrors
///
/// This routine plants all ECC errors in defined memory cells.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::PlantECCErrors(void)
{
    EnterECCTestMode();

    for (INT32 i = 0; i < ECC_TEST_ARRAY_SIZE; i++)
    {
        (*(SINGLE_ERROR_DATA[i].ptr)) = SINGLE_ERROR_DATA[i].seedValue;
        (*(DOUBLE_ERROR_DATA[i].ptr)) = DOUBLE_ERROR_DATA[i].seedValue;
    }

    ExitECCTestMode();
}

/////////////////////////////////////////////////////////////////////////////
//  ReplantEccError
///
/// This routine re-plants ECC error in given memory cell.
///
/// @param testData Test data that contains pointer and value to seed.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::ReplantEccError(const EccTestData* testData)
{
    EnterECCTestMode();
    (*(testData->ptr)) = testData->seedValue;
    ExitECCTestMode();
}

/////////////////////////////////////////////////////////////////////////////
//  TestCell
///
/// This function performs single memory cell ECC test. For single bit errors
/// it checks if value was corrected and appropriate error signaled. For double
/// bit only checks correct error was signaled (since it is not correctable).
///
/// @param testData Test to perform.
/// @return true if test was successful, false otherwise.
///
/////////////////////////////////////////////////////////////////////////////
bool ApexDiagEcc::TestCell(const EccTestData* testData)
{
    UINT32 testRead = (*(testData->ptr));

    // Verify error was correctly signaled
    if (!VerifyEccErrors(testData))
    {
        return false;
    }

    // For single bit error check if it was corrected
    if (testData->singleBitError)
    {
        if (testRead == testData->expectedValue)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    // For multi-bit test passed
    else
    {
        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  CheckErrors
///
/// This function iterates thorough test array and performs a test for every
/// element. If test fails, error is replanted and test is repeated to prevent
/// random, not planned, errors to cause the test to fail. If test fails the
/// second time it means there is a permanent error in ECC circuit or memory
/// cell and firmExcept is called.
///
/// @param testDataArray Array of tests to perform.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::CheckErrors(EccTestData const (&testDataArray)[ECC_TEST_ARRAY_SIZE])
{
    for (INT32 i = 0; i < ECC_TEST_ARRAY_SIZE; i++)
    {
        if (!TestCell(&testDataArray[i]))
        {
            ReplantEccError(&testDataArray[i]);

            // if test fails again it means there is a permanent error
            if (!TestCell(&testDataArray[i]))
            {
                firmExcept();
            }
        }
        // Clear error that was signaled during test
        HI_ApexReg.SramEccError = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  EccTestLocations
///
/// This routine checks the ECC circuitry on the internal RAM.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::EccTestLocations(void)
{
    bool irqEnabled = HI_DisableIrq();

    // Clear error that may occurred before
    if (HI_ApexReg.SramEccError & HI_BIT_SRAMECC_PER)
    {
        HI_UnlockCodeSpace();   // Unlock the code space (may or may not be in code space)
        ScrubECCError();
        HI_LockCodeSpace();     // Lock the code space
    }

    CheckErrors(SINGLE_ERROR_DATA);

    // Turn off fault generation on multi-bit ECC error
    UINT32 asicPfltBitSaved = HI_ApexReg.AsicCtrl & HI_BIT_ASIC_PFLT;
    HI_ApexReg.AsicCtrl &= ~HI_BIT_ASIC_PFLT;

    CheckErrors(DOUBLE_ERROR_DATA);

    // Restore original state
    HI_ApexReg.AsicCtrl |= asicPfltBitSaved;

    if (irqEnabled)
    {
        HI_EnableIrq();
    }
}

//******************************************************************************
// PUBLIC METHODS
//******************************************************************************

/////////////////////////////////////////////////////////////////////////////
//  DgnEccTest
///
/// @param                          (unused)
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagEcc::RunTest(ApexDiagnostic::DiagControlBlock *)
{
    EccTestLocations();
    return (ApexDiagnostic::TEST_LOOP_COMPLETE);
}

/////////////////////////////////////////////////////////////////////////////
//  DgnInitECCTest
//
/// This routine initializes the ECC diagnostic.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::Init(void)
{
    // Start Fault Injection Point 3
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 3

    // Check if ECC is enabled, it does not make sense to run ECC circuit diagnostic
    // test when it's not used - in that case this test should be not be compiled at
    // all
    AssertBitSet( HI_BIT_ASIC_IPEN );

    PlantECCErrors();
}

/////////////////////////////////////////////////////////////////////////////
//  DgnPowerupECCTest
///
/// Function runs power up diagnostic tests.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagEcc::PowerupTest(void)
{
    EccTestLocations();
}
