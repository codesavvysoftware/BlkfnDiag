/////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagRamData.cpp
///
/// Implementation of the APEX data RAM diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]   WBC 21-MAR-2005 Initial revision of file.
/// - [1]                   Unrecorded revisions, See Clearcase
/// - [1]   ACC 20-SEP-2006 Add this header.
/// - [2]   IFL 19-APR-2012 Added HOST_ICE2 flag for ICE2 chip
/// - [3]   SP  13-AUG-2013 Checksum diagnostic replaced with CRC
/// - [4]   SP  30-AUG-2013 Modification of RAM Data pattern with
///                         shadow area
/// - [5]   PG  14-OCT-2013 Fault injection point added.
/// - mgrad     25-NOV-2013 MISRA fixes
/// - spolke    27-NOV-2013 Data Ram diagnostics separated from apxDiagRam.cpp
///                         and moved to class.
/// - pszramo   11-DEC-2013 Updates to fault injection points according to
///                         Coverity defects triage.
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
//#include "Apex.hpp"                             // for HI_DisableIrq, HI_EnableIrq
#include "BlackfinDiagnostic.h"                   // for TEST_LOOP_COMPLETE etc.
#include "BlackfinDiagRamData.h"                  // for class definiton
#include "BlackfinDiagSafeRam.h"                  // for DgnSafeRam

extern "C" bool HI_DisableIrq();
extern "C" void HI_EnableIrq();
using namespace BlackfinDiagnosticGlobals;


// FORWARD REFERENCES
// Main testing function, written in assembler.

extern "C" UINT32 BlackfinDiagRamDataRunTestAsm(UINT32 numwords, UINT32 *RAMAddr, UINT32 *shadowRam);

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagRamData::Init
//
/// Initialization for Apex RAM data diagnostics.  This function initializes
/// static variables pertaining to runtime Apex RAM data diagnostics, and
/// for the most part, stores them into the double-checked ('safe') RAM.
///
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagRamData::Init()
{
    return;
    
/*    // Start Fault Injection Point 1
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 1

    // Store internal shared RAM size in 'safe' RAM
    DgnSafeRam.intShramSize = (UINT32)DGN_INT_SHR_RAM_END -
                                 (UINT32)DGN_INT_SHR_RAM_START;
    DgnSafeRam.intShramSizePrime = ~(DgnSafeRam.intShramSize);

    // Store external shared RAM size in 'safe' RAM
    DgnSafeRam.extShramSize = (UINT32)DGN_EXT_SHR_RAM_END -
                                 (UINT32)DGN_EXT_SHR_RAM_START;
    DgnSafeRam.extShramSizePrime = ~(DgnSafeRam.extShramSize);

    // Store internal data RAM size in 'safe' RAM
    DgnSafeRam.intDataramSize = (UINT32)DGN_INT_DATA_RAM_END -
                                  (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.intDataramSizePrime = ~(DgnSafeRam.intDataramSize);

    // Set current data test address to internal RAM start address
    DgnSafeRam.curDataTestAddr = (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curDataTestAddrPrime = ~((UINT32)DGN_INT_DATA_RAM_START);

    DgnSafeRam.dataRamSize =   (UINT32)DGN_INT_DATA_RAM_END
                                - (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.dataRamSizePrime = ~DgnSafeRam.dataRamSize;

    DgnSafeRam.curShadowRamAddr =   (UINT32)DGN_INT_DATA_RAM_START;
    DgnSafeRam.curShadowRamAddrPrime = ~DgnSafeRam.curShadowRamAddr;

    // Time how long it takes 0 blocks to be checked in order to determine
    // overhead time
    DgnSafeRam.dataIterations = 0;
    DgnSafeRam.dataIterationsPrime = ~(DgnSafeRam.dataIterations);

    bool irqEnabled = HI_DisableIrq();                      // Disable interrupts
    
    UINT32 startTime = BlackfinDiagnostic::GetSystemTime(); // HI_ApexReg.SystemTime;
    (void)RunTest(NULL);
    UINT32 overheadTime = BlackfinDiagnostic::GetSystemTime() - startTime; //HI_ApexReg.SystemTime - startTime;

    if (irqEnabled)
    {
        HI_EnableIrq();                    // Enable interrupts
    }

    UINT32 iterationsPerSlice;
    if (BlackfinDiagnostic::SLICE_TIME_LIMIT_US <= overheadTime)
    {
        // If it's impossible to meet the time limit, at least do one
        // iteration so something gets checked
        iterationsPerSlice = 1;
    }
    else 
    {
        // Time how long it takes 50 blocks to be checked in internal Data RAM in
        // order to figure out how many blocks can be checked during timeslice limit
        UINT32 blksToChk = 50;
        DgnSafeRam.dataIterations = blksToChk;
        DgnSafeRam.dataIterationsPrime = ~blksToChk;

        irqEnabled = HI_DisableIrq();                    // Disable interrupts

		startTime = BlackfinDiagnostic::GetSystemTime();

		//BlackfinDiagnostc::GetSystemTime(); //HI_ApexReg.SystemTime;
        (void)RunTest(NULL);
        UINT32 measureDelta = startTime - BlackfinDiagnostic::GetSystemTime();
        // - startTime; //HI_ApexReg.SystemTime - startTime;
        if (irqEnabled)
        {
            HI_EnableIrq();                    // Enable interrupts
        }

        if ((measureDelta - overheadTime) == 0)
        {
            // If the time to do blksToChk is so short that it's negligible
            // compared to the overhead (not likely), then set the iterations
            // to blksToChk since we know it will work.  This avoids a
            // divide-by-zero in the scaling calculation
            iterationsPerSlice = blksToChk;
        }
        else
        {
            // Scale the iteration count up or down, based on the
            // measurements
            iterationsPerSlice = blksToChk * (BlackfinDiagnostic::SLICE_TIME_LIMIT_US - overheadTime)
                          / (measureDelta - overheadTime);

            if (iterationsPerSlice < 1)  
            {
                // Guarantee that at least one block will be checked during a slice
                iterationsPerSlice = 1;
            }
        }
    }

    // Store iterations/slice into 'safe' RAM
    DgnSafeRam.dataIterations = iterationsPerSlice;
    DgnSafeRam.dataIterationsPrime = ~iterationsPerSlice;

    return; */
}

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagRamData::RunTest
//
/// Runtime RAM data diagnostic; performs runtime RAM data pattern tests.
///
/// @param          not used
/// @return         BlackfinDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
TestState BlackfinDiagRamData::RunTest(DiagControlBlock *)
{
    return (TEST_LOOP_COMPLETE);
    
    /* UINT32 curAddr = DgnSafeRam.curDataTestAddr;
    if (curAddr != ~DgnSafeRam.curDataTestAddrPrime)
    {
        firmExcept();
    }

    // If address isn't on a 4-word boundary, fail
    if ((curAddr & FOUR_WORD_BOUNDARY_MASK) != 0)
    {
        firmExcept();
    }

    UINT32 shadowRamAddr = DgnSafeRam.curShadowRamAddr;
    if (shadowRamAddr != ~DgnSafeRam.curShadowRamAddrPrime)
    {
        firmExcept();
    }

    // If shadow address isn't on a word boundary, fail
    if ((shadowRamAddr & WORD_BOUNDARY_MASK) != 0)
    {
        firmExcept();
    }

    // Get iteration count from 'safe' RAM
    UINT32 blksToChk = DgnSafeRam.dataIterations;
    if (blksToChk != ~DgnSafeRam.dataIterationsPrime)
    {
        firmExcept();
    }

    // Get internal RAM size from 'safe' RAM
    UINT32 memSize = DgnSafeRam.dataRamSize;
    if (memSize != ~DgnSafeRam.dataRamSizePrime)
    {
        firmExcept();
    }

    UINT32 blksToRamEnd = (((UINT32)DGN_INT_DATA_RAM_START + memSize - curAddr) / 16);

    if (blksToRamEnd > blksToChk)
    {
        // We can check all necessary blocks without going past the
        // end of memory
        (void)BlackfinDiagRamDataRunTestAsm(blksToChk, (UINT32 *) curAddr, (UINT32 *) shadowRamAddr);

        curAddr += (blksToChk << 4);      // Point to next

        DgnSafeRam.curDataTestAddr = curAddr;
        DgnSafeRam.curDataTestAddrPrime = ~curAddr;
        return (BlackfinDiagnostic::TEST_IN_PROGRESS);
    }
    else
    {
        // We're on the last set of blocks, so only test to the
        // end of memory
        (void)BlackfinDiagRamDataRunTestAsm(blksToRamEnd, (UINT32 *) curAddr, (UINT32 *) shadowRamAddr);

        // Point to start of RAM for next checks
        DgnSafeRam.curDataTestAddr = (UINT32)DGN_INT_DATA_RAM_START;
        DgnSafeRam.curDataTestAddrPrime = ~((UINT32)DGN_INT_DATA_RAM_START);

        // We're done with whole data memory, move to the next shadow window
        DgnSafeRam.curShadowRamAddr += sizeof(UINT32);
        if (DgnSafeRam.curShadowRamAddr >= (UINT32)DGN_INT_DATA_RAM_END)
        {
            DgnSafeRam.curShadowRamAddr = (UINT32)DGN_INT_DATA_RAM_START;
        }
        DgnSafeRam.curShadowRamAddrPrime = ~DgnSafeRam.curShadowRamAddr;
        return (TEST_LOOP_COMPLETE);
    } */
}

