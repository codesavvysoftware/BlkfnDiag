////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagIoShutdown.cpp
///
/// @details I/O Shutdown diagnostics implementation.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     16-OCT-2013  Created.
/// -   spolke     10-DEC-2013  Converted to a class.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagIoShutdown.hpp"           // for ApexDiagIoShutdown

// FORWARD REFERENCES
// (none)

// Class variable definitions 
UINT32 ApexDiagIoShutdown::m_LastRunTimeUs = 0;
UINT32 ApexDiagIoShutdown::m_CurrentTestCall = 0;
UINT32 ApexDiagIoShutdown::m_LastFullPathRunTestCall = 0;
UINT32 ApexDiagIoShutdown::m_CallsUntilFullTestRun = 0;

ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::m_TestPhase = POWERUP;

const ApexDiagIoShutdown::PhaseFunction ApexDiagIoShutdown::m_PhaseFunctions[] = 
{
     TestFunctionFirmExceptAdapter,
     WaitingForVms,
     QuickTest,
     FullTestStart,
     FullTestEnd
};

////////////////////////////////////////////////////////////////////////////////
//  Phase functions.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::WaitingForVms
///
/// @memberof ApexDiagIoShutdown
///
/// @details Function checking if VMS signaled it is up by setting fault apex diag line.
///
/// @return TestPhase   Returns state the module should move to.
///     @retval DGN_IOSHUTDOWN_QUICK_TEST       VMS is ready, start normal testing procedure - 
///                                             safety reaction time test.
///     @retval DGN_IOSHUTDOWN_WAITING_FOR_VMS  VMS does not respond yet, keep waiting.
////////////////////////////////////////////////////////////////////////////////
ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::WaitingForVms()
{
    if (IsVmsFaultApexDiagSet())
    {
        ClearTestFaultLine();

        return QUICK_TEST;
    }
    return WAITING_FOR_VMS;
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::QuickTest
///
/// @memberof ApexDiagIoShutdown
///
/// @details Function responsible for running safety time test.
///
/// @return TestPhase                       Returns state the module should move to.
///     @retval DGN_IOSHUTDOWN_QUICK_TEST   Keep testing.
////////////////////////////////////////////////////////////////////////////////
ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::QuickTest()
{
    if (!CheckForNormalTestBits())
    {
        firmExcept();
    }
    return QUICK_TEST;
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::FullTestStart
///
/// @memberof ApexDiagIoShutdown
///
/// @details Function responsible for running steps to start full path test.
///
/// @return TestPhase                           Returns state the module should move to.
///     @retval DGN_IOSHUTDOWN_FULL_TEST_END    Move to final stage of full path test.
////////////////////////////////////////////////////////////////////////////////
ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::FullTestStart()
{
    if (!CheckForNormalTestBits())
    {
        firmExcept();
    }

    SetInvertedTestBitSet();

    if (!CheckForInvertedTestBits())
    {
        firmExcept();
    }

    SetNormalTestBitSet();

    SetTestFaultLine();

    return FULL_TEST_END;
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::FullTestEnd
///
/// @memberof ApexDiagIoShutdown
///
/// @details Function responsible for running steps to finish full path test.
///
/// @return TestPhase                       Returns state the module should move to.
///     @retval DGN_IOSHUTDOWN_QUICK_TEST   Full path test finished, go back to safety 
///                                         reaction time test.
////////////////////////////////////////////////////////////////////////////////
ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::FullTestEnd()
{
    // VMS fault line will cause the bits to be inverted
    if (!CheckForInvertedTestBits())
    {
        firmExcept();
    }
    
    ClearTestFaultLine();
    
    m_LastFullPathRunTestCall = m_CurrentTestCall;

    return QUICK_TEST;
}

////////////////////////////////////////////////////////////////////////////////
//  Interface implementation.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::Init
///
/// @memberof ApexDiagIoShutdown
///
/// @details Function initializing IO Shutdown diagnostics. It should only be called 
///          once during boot up. It initializes GPIO lines and signals VMS Apex is
///          up. Then it changes module state to wait for VMS response. 
///
////////////////////////////////////////////////////////////////////////////////
void ApexDiagIoShutdown::Init()
{
    // Compilation time check for test interval to be valid 
    STATIC_ASSERT(TEST_INTERVAL_MS > 0);

    // Make sure we were called only once
    ASSERT(m_TestPhase == POWERUP);
    
    // Workaround for apex compiler static variables initialization problem
    m_CallsUntilFullTestRun = FULL_TEST_INVERVAL_IN_TEST_CALLS;
    
    // Initialize IO shutdown diagnostic to value in between ordinary diagnostics to prevent 
    // calling both diagnostics in one loop
    m_LastRunTimeUs = TEST_INTERVAL_US / 2;

    // Configure inputs and outputs (bit cleared == input, bit set == output)
    // Clear input bits 
    HI_ApexReg.GpioOutEnbl &= ~GPIO_BITS_CONFIG_INPUT;
    // Set output bits 
    HI_ApexReg.GpioOutEnbl |= GPIO_BITS_CONFIG_OUTPUT; 
    
    // Set first test bit set
    SetNormalTestBitSet();

    // Notify VMS we're ready by setting test fault line
    SetTestFaultLine();
    
    // Wait for VMS to become operational and able to pull the fault line
    m_TestPhase = WAITING_FOR_VMS;
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::RunTest
///
/// @memberof ApexDiagIoShutdown
///
/// @details Main function of the IO shutdown diagnostics. It calls other functions
///          responsible for actions depending on current test step.
///
////////////////////////////////////////////////////////////////////////////////
void ApexDiagIoShutdown::RunTest(const UINT32 uptimeUs)
{
    m_LastRunTimeUs = uptimeUs;
    m_CurrentTestCall++;

    m_CallsUntilFullTestRun--;
    if (0 == m_CallsUntilFullTestRun)
    {
        m_CallsUntilFullTestRun = FULL_TEST_INVERVAL_IN_TEST_CALLS; 

        // if still in waiting for VMS phase it means there was no answer from 
        // VMS for 8 hours
        if (m_TestPhase == WAITING_FOR_VMS)
        {
            firmExcept();
        }

        m_TestPhase = FullTestStart();
    }
    else
    {
        m_TestPhase = (m_PhaseFunctions[m_TestPhase])();
    }
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::CheckCompletionTime
///
/// @memberof ApexDiagIoShutdown
///
/// @details Checks if IO shutdown diagnostic completes in time. 
///
/// @return bool        
///     @retval true    If test completes in time.
///     @retval false   Any other case.
////////////////////////////////////////////////////////////////////////////////
bool ApexDiagIoShutdown::CheckCompletionTime()
{
    // unsigned int math handles roll-over
    if (m_CurrentTestCall - m_LastFullPathRunTestCall > FULL_TEST_TIMEOUT_IN_TEST_CALLS)
    {
        return false;
    }
    return true;
}
