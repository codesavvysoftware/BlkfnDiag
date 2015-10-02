/////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagSafeRam.cpp
///
/// Implementation of the Blackfin safe ram diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// @endif
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "BlackfinDiagSafeRam.h"                  // for BlackfinDiagSafeRam
//#include "Apex.hpp"                             // for firmExcept

using namespace BlackfinDiagnosticGlobals;

// FORWARD REFERENCES
// (none)

DGN_SAFE_RAM DgnSafeRam = {0};

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagSafeRam::Init
//
/// This routine initializes the special doubly-checked 'Safe RAM' that is
/// used to store various static variables used in the RAM diagnostic itself.
/// Each of the paired values is a complement of the other.
///
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagSafeRam::Init(void)
{
    // Start Fault Injection Point 2
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 2

    // Initialize the values
    DgnSafeRam.intDataramSize        =   (UINT32)DGN_INT_DATA_RAM_END - (UINT32)DGN_INT_DATA_RAM_START;                                
    DgnSafeRam.intDataramSizePrime   = ~DgnSafeRam.intDataramSize;    
    DgnSafeRam.intShramSize          =    (UINT32)DGN_INT_SHR_RAM_END - (UINT32)DGN_INT_SHR_RAM_START;                                
    DgnSafeRam.intShramSizePrime     = ~DgnSafeRam.intShramSize;
    DgnSafeRam.extShramSize          =    (UINT32)DGN_EXT_SHR_RAM_END - (UINT32)DGN_EXT_SHR_RAM_START;
    DgnSafeRam.extShramSizePrime     = ~DgnSafeRam.extShramSize;
    DgnSafeRam.curDataTestAddr       = (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curDataTestAddrPrime  = ~DgnSafeRam.curDataTestAddr;
    DgnSafeRam.curAddrTestAddr1      = 0;
    DgnSafeRam.curAddrTestAddr1Prime = ~DgnSafeRam.curAddrTestAddr1;
    DgnSafeRam.curAddrTestAddr2      = 0;
    DgnSafeRam.curAddrTestAddr2Prime = ~DgnSafeRam.curAddrTestAddr2;
    DgnSafeRam.dataIterations        = 0;
    DgnSafeRam.dataIterationsPrime   = ~DgnSafeRam.dataIterations;
    DgnSafeRam.addrIterations        = 0;
    DgnSafeRam.addrIterationsPrime   = ~DgnSafeRam.addrIterations;
    DgnSafeRam.dataRamSize           = (UINT32)DGN_INT_DATA_RAM_END - (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.dataRamSizePrime      = ~DgnSafeRam.dataRamSize;
    DgnSafeRam.curShadowRamAddr      = (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curShadowRamAddrPrime = ~DgnSafeRam.curShadowRamAddr;

    // Do an initial check of them
    (void)Check(NULL);
}

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagSafeRam::IsDataSafeRamInvalid

/// Check if any data test-related variable doesn't match its complement.
///
/// @return bool
///     @retval true if data was correctly validated
///     @retval false otherwise
/////////////////////////////////////////////////////////////////////////////
bool BlackfinDiagSafeRam::IsDataSafeRamInvalid()
{
    return  (DgnSafeRam.intDataramSizePrime   != ~DgnSafeRam.intDataramSize)
        ||  (DgnSafeRam.curDataTestAddrPrime  != ~DgnSafeRam.curDataTestAddr)
        ||  (DgnSafeRam.dataIterationsPrime   != ~DgnSafeRam.dataIterations)
        ||  (DgnSafeRam.dataRamSizePrime      != ~DgnSafeRam.dataRamSize)
        ||  (DgnSafeRam.curShadowRamAddrPrime != ~DgnSafeRam.curShadowRamAddr);
}

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagSafeRam::IsAddrSafeRamInvalid
///
/// Check if any address test-related variable doesn't match its complement.
///
/// @return bool
///     @retval true if data was correctly validated
///     @retval false otherwise
/////////////////////////////////////////////////////////////////////////////
bool BlackfinDiagSafeRam::IsAddrSafeRamInvalid()
{
    return  (DgnSafeRam.intShramSizePrime     != ~DgnSafeRam.intShramSize)
        ||  (DgnSafeRam.extShramSizePrime     != ~DgnSafeRam.extShramSize)
        ||  (DgnSafeRam.curAddrTestAddr1Prime != ~DgnSafeRam.curAddrTestAddr1)
        ||  (DgnSafeRam.curAddrTestAddr2Prime != ~DgnSafeRam.curAddrTestAddr2)
        ||  (DgnSafeRam.addrIterationsPrime   != ~DgnSafeRam.addrIterations);
}

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagSafeRam::Check
//
/// This routine checks the special doubly-checked 'Safe RAM' that is used to
/// store various static variables used in the RAM diagnostic itself.  The
/// diagnostic verifies that each of the paired values is a complement of the
/// other.
///
/// @param                  (not used)
/// @return         BlackfinDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
TestState BlackfinDiagSafeRam::Check(DiagControlBlock * )
{
    // Start Fault Injection Point 1
    // Code which inverts cell complimentary to DgnSafeRam.intDataramSize
    // variable when InjectFaultFlag is set at run-time will be injected here.
    // End Fault Injection Point 1

    if (IsDataSafeRamInvalid() || IsAddrSafeRamInvalid())
    {
        firmExcept();
    }

    return (TEST_LOOP_COMPLETE);
}
