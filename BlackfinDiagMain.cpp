/////////////////////////////////////////////////////////////////////////////
/// @file apxDiagMain.cpp
///
/// Implements the diagnostic services for the APEX ASIC which include
/// powerup and runtime diagnostics, initialization, and scheduling.
///
/// The following routines/methods are included:
///     - DgnSched()
///     - DgnSchedInit()
///     - DgnInit()
///     - DgnPwrUpDiags()
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]     WBC 01-MAR-2005 Initial revision of file.
/// - [1]     DJD 13-Feb-2009 Lgx00094867 Clear the scrub indication in the hardware
/// @endif
///
/// @par Copyright &copy; 2005-2011 Rockwell Automation, Inc. All rights reserved.
/// @ingroup APX
/////////////////////////////////////////////////////////////////////////////
//#include "apx.hpp"
//#include "apxDiag.hpp"
//#include "apxWatchdog.hpp"
#include "BlackfinDiag.h"
#include "BlackfinDiagnostic.h"

using namespace BlackfinDiagnosticGlobals;

//Watchdog Watchdog;


/////////////////////////////////////////////////////////////////////////////
//  FUNCTION NAME: DgnSched
//
/// This function handles the scheduling of the various runtime diagnostic
/// functions.
///
/////////////////////////////////////////////////////////////////////////////
void DgnSched(void)
{

    UINT8        dgnIndex = 0;          // Zero-based index to Diag CB
    UINT32       dgnTemp;               // Temp used in maximum time
                                        // calculation
    UINT32       dgnResult;             // Result returned from
                                        // diagnostic test
    DGN_CTL_BLK *dgnCurDiagCB;          // Pointer to the currently
                                        // referenced Diag CB
    UINT32       dgnEntryTime;          // for recording diag duration
    UINT32       dgnExitTime;


    // Do hardware watchdog handing
    //Watchdog.Service();

 
    // Find either a test scheduled for this slice or the last test
    // in the list
    while ((dgnIndex < (DGN_NUM_RUNTIME_TESTS - 1))
           && (DgnSliceNumber != DgnList[dgnIndex].triggerValueTimeslice))
    {
        dgnIndex++;
    }

    // Assign pointer to found Diag CB
    dgnCurDiagCB = &DgnList[dgnIndex];

    // Schedule next trigger slice for this diag. UINT math handles
    // rollover. In some cases, this trigger value will be overwritten
    // from within the test function that is called.  That's why we do the
    // reschedule before we enter the function.
    dgnCurDiagCB->triggerValueTimeslice += dgnCurDiagCB->stepValueTimeslice;

    // Record the entry time before calling diag
#ifdef HI_APEX_DIAGNOSTICS

    HI_DisableIrq();                    // Disable interrupts
#endif

    dgnEntryTime = GetSystemTime();

    // Call the test function pointed to in Diag CB
    dgnResult = dgnCurDiagCB->diagFunc(dgnCurDiagCB);

    // Record the exit time of the diag
    dgnExitTime = GetSystemTime();

#ifdef HI_APEX_DIAGNOSTICS

    HI_EnableIrq();                    // Enable interrupts
#endif

    switch (dgnResult)
    {
    case TEST_LOOP_COMPLETE:

        // If the test has completed all possible iterations, figure out
        // how long it's been since the last total completion.
        dgnTemp = DgnSliceNumber - dgnCurDiagCB->lastCompleteTimeslice;

        if ((dgnTemp > dgnCurDiagCB->maxTimeslices)
            && (dgnCurDiagCB->oneComplete == TRUE))
        {
            // Store the time if it exceeds the time already stored, and
            // the diagnostic has run at least one previous time
            dgnCurDiagCB->maxTimeslices = dgnTemp;
        }

        dgnCurDiagCB->oneComplete = TRUE;
        dgnCurDiagCB->lastCompleteTimeslice = DgnSliceNumber;

    // (Fall through)

    case TEST_IN_PROGRESS:

        // If the test has successfully run to the timelimit within the
        // timeslice, but has not completed all iterations, don't bother
        // with the maximum time calculations.  However, for debugging purposes,
        // save the amount of time that the diagnostic function took to run
        // during this timeslice into its diag CB.
        dgnCurDiagCB->durationUs = dgnExitTime - dgnEntryTime;

#ifdef HI_APEX_DIAGNOSTICS

        if (dgnCurDiagCB->durationUs > dgnCurDiagCB->maxduration)
            dgnCurDiagCB->maxduration = dgnCurDiagCB->duration;

        if (dgnCurDiagCB->durationUs < dgnCurDiagCB->minduration)
            dgnCurDiagCB->minduration = dgnCurDiagCB->duration;
#endif

        break;

    default:
        // Call HW_HandleException for any other responses
        firmExcept(); //HW_HandleException(__FILE__, __LINE__);
        break;
    }
    ;

    // Reschedule any lower priority diagnostics that were also
    // scheduled for this slice by bumping their trigger values to the
    // next slice number
    dgnIndex++;
    dgnCurDiagCB++;

    while (dgnIndex < DGN_NUM_RUNTIME_TESTS)
    {
        if (DgnSliceNumber == dgnCurDiagCB->triggerValueTimeslice)
        {
            dgnCurDiagCB->triggerValueTimeslice++;
        }
        dgnIndex++;
        dgnCurDiagCB++;
    }

    // Update diag slice number.
    DgnSliceNumber++;
    return;
}

/////////////////////////////////////////////////////////////////////////////
//  FUNCTION NAME: DgnSchedInit
//
/// This function initializes runtime diagnostic scheduler variables and
/// structures.
///
/////////////////////////////////////////////////////////////////////////////
void DgnSchedInit(void)
{

#ifdef HI_APEX_DIAGNOSTICS
    {
        INT32 nn;

        for (nn = 0; nn < DGN_NUM_RUNTIME_TESTS; ++nn)
        {
            DgnList[nn].maxduration = 0;
            DgnList[nn].minduration = 0xffffffff;
        }
    }
#endif

    DgnSliceNumber = 0;


    DgnList[DGN_SAFE_RAM_NUM].diagFunc = DgnChkSafeRAM;
    DgnList[DGN_SAFE_RAM_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_SAFE_RAM_NUM].stepValueTimeslice = DGN_SAFE_RAM_STEP;
    DgnList[DGN_SAFE_RAM_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_SAFE_RAM_NUM].durationUs = 0;
    DgnList[DGN_SAFE_RAM_NUM].maxTimeslices = 0;
    DgnList[DGN_SAFE_RAM_NUM].oneComplete = FALSE;

    DgnList[DGN_EXE_CHECKSUM_NUM].diagFunc = DgnRAMExecTest;
    DgnList[DGN_EXE_CHECKSUM_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_EXE_CHECKSUM_NUM].stepValueTimeslice = DGN_EXE_CHECKSUM_STEP;
    DgnList[DGN_EXE_CHECKSUM_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_EXE_CHECKSUM_NUM].durationUs = 0;
    DgnList[DGN_EXE_CHECKSUM_NUM].maxTimeslices = 0;
    DgnList[DGN_EXE_CHECKSUM_NUM].oneComplete = FALSE;

    DgnList[DGN_RAM_DATA_NUM].diagFunc = DgnRAMData;
    DgnList[DGN_RAM_DATA_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].stepValueTimeslice = DGN_RAM_DATA_STEP;
    DgnList[DGN_RAM_DATA_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].durationUs = 0;
    DgnList[DGN_RAM_DATA_NUM].maxTimeslices = 0;
    DgnList[DGN_RAM_DATA_NUM].oneComplete = FALSE;

    return;

}

/////////////////////////////////////////////////////////////////////////////
//  FUNCTION NAME: DgnInit
//
/// This function initializes variables and structures for runtime diagnostics.
///
/////////////////////////////////////////////////////////////////////////////
void DgnInit(void)
{
    DgnInitSafeRAM();

    DgnRAMDataInit();

    DgnRAMExecInit();

    DgnIntRAMAddrInit();
}

/////////////////////////////////////////////////////////////////////////////
//  FUNCTION NAME: DgnPwrUpDiags
//
//  This function performs diagnostics that are done once at powerup.
//
/////////////////////////////////////////////////////////////////////////////
void DgnPwrUpDiags(void)
{
    DgnRAMExecPwrupTst(); //Checksum test of the executable RAM area

    DgnRamECCPwrUpTst(); // Verify the ECC can detect and correct memory errors.
}
