/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagSafeRam.cpp
///
/// Implementation of the APEX safe ram diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  WBC       21-MAR-2005 Initial revision of file.
/// - [1]                        Unrecorded revisions, See Clearcase.
/// - [1]  ACC       20-SEP-2006 Add this header.
/// - [2]  IFL       19-APR-2012 Added HOST_ICE2 flag for ICE2 chip.
/// - [3]  SP        13-AUG-2013 Checksum diagnostic replaced with CRC.
/// - [4]  SP        30-AUG-2013 Modification of RAM Data pattern with
///                              shadow area.
/// - [5]  PG        14-OCT-2013 Fault injection point added.
/// - [6]  mgrad     25-NOV-2013 MISRA fixes.
/// - [7]  spolke    27-NOV-2013 SafeRam diagnostics separated from apxDiagRam.cpp.
/// - [8]  pszramo   27-NOV-2013 Refactoring to reduce DgnChkSafeRAM() complexity.
/// - [9]  spolke    28-NOV-2013 Converted to class.
/// - [10] pszramo   11-DEC-2013 Updates to fault injection points according to
///                              Coverity defects triage.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
/// @ingroup APX
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "ApexDiagSafeRam.hpp"                  // for ApexDiagSafeRam
#include "Apex.hpp"                             // for firmExcept

// FORWARD REFERENCES
// (none)

DGN_SAFE_RAM DgnSafeRam = {0};

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagSafeRam::Init
//
/// This routine initializes the special doubly-checked 'Safe RAM' that is
/// used to store various static variables used in the RAM diagnostic itself.
/// Each of the paired values is a complement of the other.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagSafeRam::Init(void)
{
    // Start Fault Injection Point 2
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 2

    // Initialize the values
    DgnSafeRam.intDataramSize =   (UINT32)DGN_INT_DATA_RAM_END
                                - (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.intDataramSizePrime = ~DgnSafeRam.intDataramSize;
    DgnSafeRam.intShramSize =    (UINT32)DGN_INT_SHR_RAM_END
                                - (UINT32)DGN_INT_SHR_RAM_START;
    DgnSafeRam.intShramSizePrime = ~DgnSafeRam.intShramSize;
    DgnSafeRam.extShramSize =    (UINT32)DGN_EXT_SHR_RAM_END
                                - (UINT32)DGN_EXT_SHR_RAM_START;
    DgnSafeRam.extShramSizePrime = ~DgnSafeRam.extShramSize;
    DgnSafeRam.curDataTestAddr = (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curDataTestAddrPrime = ~DgnSafeRam.curDataTestAddr;
    DgnSafeRam.curAddrTestAddr1 = 0;
    DgnSafeRam.curAddrTestAddr1Prime = ~DgnSafeRam.curAddrTestAddr1;
    DgnSafeRam.curAddrTestAddr2 = 0;
    DgnSafeRam.curAddrTestAddr2Prime = ~DgnSafeRam.curAddrTestAddr2;
    DgnSafeRam.dataIterations = 0;
    DgnSafeRam.dataIterationsPrime = ~DgnSafeRam.dataIterations;
    DgnSafeRam.addrIterations = 0;
    DgnSafeRam.addrIterationsPrime = ~DgnSafeRam.addrIterations;

    DgnSafeRam.dataRamSize =   (UINT32)DGN_INT_DATA_RAM_END
                                - (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.dataRamSizePrime = ~DgnSafeRam.dataRamSize;
    DgnSafeRam.curShadowRamAddr =   (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curShadowRamAddrPrime = ~DgnSafeRam.curShadowRamAddr;

    // Do an initial check of them
    (void)Check(NULL);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagSafeRam::IsDataSafeRamInvalid

/// Check if any data test-related variable doesn't match its complement.
///
/// @return bool
///     @retval true if data was correctly validated
///     @retval false otherwise
/////////////////////////////////////////////////////////////////////////////
bool ApexDiagSafeRam::IsDataSafeRamInvalid()
{
    return  (DgnSafeRam.intDataramSizePrime != ~DgnSafeRam.intDataramSize)
        ||  (DgnSafeRam.curDataTestAddrPrime != ~DgnSafeRam.curDataTestAddr)
        ||  (DgnSafeRam.dataIterationsPrime != ~DgnSafeRam.dataIterations)
        ||  (DgnSafeRam.dataRamSizePrime != ~DgnSafeRam.dataRamSize)
        ||  (DgnSafeRam.curShadowRamAddrPrime != ~DgnSafeRam.curShadowRamAddr);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagSafeRam::IsAddrSafeRamInvalid
///
/// Check if any address test-related variable doesn't match its complement.
///
/// @return bool
///     @retval true if data was correctly validated
///     @retval false otherwise
/////////////////////////////////////////////////////////////////////////////
bool ApexDiagSafeRam::IsAddrSafeRamInvalid()
{
    return  (DgnSafeRam.intShramSizePrime != ~DgnSafeRam.intShramSize)
        ||  (DgnSafeRam.extShramSizePrime != ~DgnSafeRam.extShramSize)
        ||  (DgnSafeRam.curAddrTestAddr1Prime != ~DgnSafeRam.curAddrTestAddr1)
        ||  (DgnSafeRam.curAddrTestAddr2Prime != ~DgnSafeRam.curAddrTestAddr2)
        ||  (DgnSafeRam.addrIterationsPrime != ~DgnSafeRam.addrIterations);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagSafeRam::Check
//
/// This routine checks the special doubly-checked 'Safe RAM' that is used to
/// store various static variables used in the RAM diagnostic itself.  The
/// diagnostic verifies that each of the paired values is a complement of the
/// other.
///
/// @param                  (not used)
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagSafeRam::Check(ApexDiagnostic::DiagControlBlock * )
{
    // Start Fault Injection Point 1
    // Code which inverts cell complimentary to DgnSafeRam.intDataramSize
    // variable when InjectFaultFlag is set at run-time will be injected here.
    // End Fault Injection Point 1

    if (IsDataSafeRamInvalid() || IsAddrSafeRamInvalid())
    {
        firmExcept();
    }

    return (ApexDiagnostic::TEST_LOOP_COMPLETE);
}
