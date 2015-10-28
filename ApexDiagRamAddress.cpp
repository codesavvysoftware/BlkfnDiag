/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagRamAddress.cpp
///
/// Implementation of the APEX Address RAM diagnostics.
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
/// - spolke    27-NOV-2013 Renamed from apxDiagRAM.cpp to ApexDiagRamAddress.
///                         Binary CRC, data and address diagnostics moved to separate files.
///                         Address diagnostics moved to class.
/// - pszramo   11-DEC-2013 Updates to fault injection points according to
///                         Coverity defects triage.
/// - mstasia   16-DEC-2013 Removed External RAM support, algorithm corrections
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
#include "Apex.hpp"                             // for HI_DisableIrq, HI_EnableIrq
#include "ApexDiagRamAddress.hpp"               // for ApexDiagRamAddress
#include "ApexDiagSafeRam.hpp"                  // for DgnSafeRam

// FORWARD REFERENCES
// Main testing function, written in assember.
extern "C" UINT32 ApexDiagRamAddressRunTestAsm(void);

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagRamAddress::Init
//
/// This function initializes variables for runtime RAM address diagnostics.
/// Since the internal data RAM address space includes all of the address
/// lines that the internal shared RAM uses, we're covered for both cases.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagRamAddress::Init()
{
    UINT32 sigbits;             // Number of significant bits (not MSB)
    UINT32 lastAddr;            // The last avaliable address (32-bit boundary)
    UINT32 sigAddr;             // The last address with cleared bits which are not significant
    UINT32 allowedIterations;   // Number of iterations allowed 
    UINT32 overheadTime;        // Used in timing measurements
    UINT32 measureDelta;        // Used in timing measurements
    UINT32 startTime;           // Used in timing measurements

    // Can't test external ram on xscale yet.  So just do the internal ram.
    DgnSafeRam.curAddrTestAddr1 = (UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr1Prime = ~(UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr2 = ((UINT32) DGN_INT_DATA_RAM_START + 4);
    DgnSafeRam.curAddrTestAddr2Prime = ~((UINT32) DGN_INT_DATA_RAM_START + 4);

    // Start Fault Injection Point 1
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 1

    // Calculate number of significant address bits in internal data RAM by
    // finding the bit position of the leading one of the size-1

    lastAddr = (UINT32)DGN_INT_DATA_RAM_END - 4;
    sigAddr = lastAddr & (~(UINT32) DGN_INT_DATA_RAM_START);
    
    sigbits = 32;
    while ((sigbits > 0) && ((sigAddr & 0x80000000) == 0)) {
        sigbits -= 1;
        sigAddr <<= 1;
    }

    // Subtract 2, since we won't check bits zero or one
    sigbits -= 2;

    // Put a zero iteration size in 'safe' RAM so that we just measure the
    // overhead
    DgnSafeRam.addrIterations = 0;
    DgnSafeRam.addrIterationsPrime = ~((UINT32) 0);

     // Calculate how many address pairs can be checked per timeslice
     // by timing how long it takes to get through internal data RAM.

    // Measure time to check 0 pairs, to determine overhead time

    bool irqEnabled = HI_DisableIrq();

    // Measure the time to execute the function
    startTime = HI_ApexReg.SystemTime;
    (void)RunTest(NULL);
    measureDelta = HI_ApexReg.SystemTime - startTime;
    if (irqEnabled)
    {
        HI_EnableIrq();
    }

    overheadTime = measureDelta;

    // Put an iteration size in 'safe' RAM so that we check the whole
    // internal data RAM

    DgnSafeRam.addrIterations = (UINT32) (2 * sigbits);
    DgnSafeRam.addrIterationsPrime = ~((UINT32) (2 * sigbits));

    // Measure time to check RAM
    irqEnabled = HI_DisableIrq();
    startTime = HI_ApexReg.SystemTime;
    (void)RunTest(NULL);
    measureDelta = HI_ApexReg.SystemTime - startTime;
    if (irqEnabled)
    {
        HI_EnableIrq();
    }
    if (((INT32)ApexDiagnostic::SLICE_TIME_LIMIT_US - (INT32)overheadTime) <= 0)
    {
        // If it's impossible to meet the time limit, at least allow one
        // iteration so something gets checked

        allowedIterations = 1;
    }
    else if (( measureDelta - overheadTime) == 0)
    {
        // If the time to do RAM checks is so short that it's negligible
        // compared to the overhead (not likely), then set the iterations
        // to 2 * sigbits since we know it will work.  This avoids a
        // divide-by-zero in the scaling calculation

        allowedIterations = 2 * sigbits;
    }
    else
    {
        // Scale the iteration count up or down, based on the
        // measurements

        allowedIterations =
                (UINT32) (
                ((INT32) 2 * (INT32) sigbits)
                * ((INT32) ApexDiagnostic::SLICE_TIME_LIMIT_US - (INT32) overheadTime)
                / ((INT32) measureDelta - (INT32) overheadTime));

        if (allowedIterations < 1)  /* Guarantee that at least one pair */
            allowedIterations = 1;  /* will be checked during a slice */
    }

    // Store iteration value in 'safe' RAM (with prime value, too)

    DgnSafeRam.addrIterations = allowedIterations;
    DgnSafeRam.addrIterationsPrime = ~allowedIterations;

    // Initialize Addr1 and Addr2 to internal RAM start
    DgnSafeRam.curAddrTestAddr1 = (UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr1Prime = ~(UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr2 = ((UINT32) DGN_INT_DATA_RAM_START + 4);
    DgnSafeRam.curAddrTestAddr2Prime = ~((UINT32) DGN_INT_DATA_RAM_START + 4);

    return;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagRamAddress::RunTest
//
/// This function performs runtime internal RAM address line
/// tests. Since the internal data RAM address space includes all
/// of the address lines that the internal shared RAM uses,
/// we're covered for both internal cases.
///
/// @param      not used
/// @return     ApexDiagnostic::TestState
///     @retval TEST_LOOP_COMPLETE if the test has successfully completed all iterations.
///     @retval TEST_IN_PROGRESS   if the test has successfully completed iterations for the
///             timeslice, but has not completed all iterations. (Fails in function with
///             firmExcept() if not successful).
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagRamAddress::RunTest(ApexDiagnostic::DiagControlBlock *)
{
    UINT32 addrReturnVal;

    // Perform checks (assembly code)
    // (Function includes address bounds checking)
    addrReturnVal = ApexDiagRamAddressRunTestAsm();

    if (addrReturnVal == 0)
    {
        // If not done with all iterations for the particular RAM, the
        // assembly function will put updated addresses into 'safe' RAM.  We
        // just need to return status
        return (ApexDiagnostic::TEST_IN_PROGRESS);
    }

    // Can't test external ram on xscale yet.  So just do the internal ram.
    DgnSafeRam.curAddrTestAddr1 = (UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr1Prime = ~(UINT32) DGN_INT_DATA_RAM_START;
    DgnSafeRam.curAddrTestAddr2 = ((UINT32) DGN_INT_DATA_RAM_START + 4);
    DgnSafeRam.curAddrTestAddr2Prime = ~((UINT32) DGN_INT_DATA_RAM_START + 4);

    return (ApexDiagnostic::TEST_LOOP_COMPLETE);
}
