////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagIoShutdown.hpp
///
/// @details Interface for I/O Shutdown diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     16-OCT-2013  Created.
/// -   spolke     10-DEC-2013  Converted to a class.
/// -   dtstalte   11-FEB-2014  Lgx00148711 - poll on GPIO input signal a few
///                             times to allow for propagation delay.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGIOSHUTDOWN_HPP 
#define APEXDIAGIOSHUTDOWN_HPP 

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "Apex.hpp"                          // Hw_HandleException used by firmExcept
#include "ApexRegisters.hpp"                 // for HI_BIT_GPIO_TEST_FAULT
#include "ApexDiagnostic.hpp"                // for DGN_INTERVAL_IOSHUTDOWN_US

// FORWARD REFERENCES
// (none)

class ApexDiagIoShutdown 
{
public:
    
    /// Initialization of IO Shutdown diagnostics.
    static void Init();

    /// Method checks if it's time to run Io shutdown diagnostics.
    inline static bool IsTimeToRun(const UINT32 uptimeUs)
    {
        // unsigned arithmetic handles roll-over
        return (uptimeUs - m_LastRunTimeUs) > TEST_INTERVAL_US;
    }

    /// Performs IO shutdown diagnostics test. 
    static void RunTest(const UINT32 uptimeUs);

    /// Checks if IO shutdown diagnostic completes in time. 
    static bool CheckCompletionTime();
    
private:

    /// Default constructor.
    ApexDiagIoShutdown();
    
    /// Default destructor.
    ~ApexDiagIoShutdown();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagIoShutdown(const ApexDiagIoShutdown &);
    ApexDiagIoShutdown &operator=(const ApexDiagIoShutdown &);

    // CONST VARIABLES 
    static const UINT32 GPIO_BITS_CONFIG_INPUT = 
                    HI_BIT_GPIO_IOSHUTDOWN_VMS_FAULT | HI_MASK_GPIO_IOSHUTDOWN_INPUT; // input config (bits 17, 28, 29) 

    static const UINT32 GPIO_BITS_CONFIG_OUTPUT = 
                    HI_BIT_GPIO_IOSHUTDOWN_TEST_FAULT | HI_MASK_GPIO_IOSHUTDOWN_OUTPUT; // output config (bits 16, 30, 31) 

    static const UINT32 GPIO3130_NORMAL_TEST_BITS       = 0x80000000;
    static const UINT32 GPIO3130_INVERTED_TEST_BITS     = 0x40000000;

    static const UINT32 GPIO2928_NORMAL_EXPECTED_BITS   = 0x20000000;  
    static const UINT32 GPIO2928_INVERTED_EXPECTED_BITS = 0x10000000;
    
    // Used for polling for GPIO value change.  The write and read done
    // by the test can occur as back to back instructions from the compiler.
    // The worst case delay of the signal from output to input is 18.5 ns
    // as confirmed by hardware.  Since the Apex clock is 160 MHz, this means
    // we could read the input before the signal propagates.  Retry a few
    // times just to cover the worst case propagation delay.
    static const INT32 GPIO_PROPAGATION_RETRIES = 5;
    
    // run every 25 milliseconds
    static const UINT32 TEST_INTERVAL_MS = 25;

    static const UINT32 TEST_INTERVAL_US = TEST_INTERVAL_MS * (ApexDiagnostic::MICROSECONDS_IN_MILLISECOND);

    // we want the test to finish in 8 hours so set timeout time to 8 hours
    static const UINT32 FULL_TEST_TIMEOUT_MS  = 8 * ApexDiagnostic::MILLISECONDS_IN_HOUR; 
    // and interval time to 7.5 hours to left some safety margin
    static const UINT32 FULL_TEST_INVERVAL_MS = 7 * ApexDiagnostic::MILLISECONDS_IN_HOUR + 30 * ApexDiagnostic::MILLISECONDS_IN_MINUTE; 

    static const UINT32 FULL_TEST_INVERVAL_IN_TEST_CALLS = FULL_TEST_INVERVAL_MS / TEST_INTERVAL_MS;
    static const UINT32 FULL_TEST_TIMEOUT_IN_TEST_CALLS = FULL_TEST_TIMEOUT_MS / TEST_INTERVAL_MS;

    static UINT32  m_LastRunTimeUs;


    static UINT32 m_CurrentTestCall;
    static UINT32 m_LastFullPathRunTestCall;

    static UINT32 m_CallsUntilFullTestRun;

    enum TestPhase 
    {
        POWERUP = 0,
        WAITING_FOR_VMS,
        QUICK_TEST,
        FULL_TEST_START,
        FULL_TEST_END
    };
    
    static TestPhase m_TestPhase;

    typedef TestPhase (*PhaseFunction)();

    /// Helper inline methods to make code more readable
    inline static void SetNormalTestBitSet();
    inline static void SetInvertedTestBitSet();
    inline static bool CheckForNormalTestBits();
    inline static bool CheckForInvertedTestBits();
    inline static void SetTestFaultLine();
    inline static void ClearTestFaultLine();
    inline static bool IsVmsFaultApexDiagSet();

    /// Phase methods
    static TestPhase TestFunctionFirmExceptAdapter();
    static TestPhase WaitingForVms();
    static TestPhase QuickTest();
    static TestPhase FullTestStart();
    static TestPhase FullTestEnd();

    static const PhaseFunction m_PhaseFunctions[];
};

////////////////////////////////////////////////////////////////////////////////
//  Helper inline methods to make code more readable
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  SetNormalTestBitSet
///
/// @memberof ApexDiagIoShutdown
///
/// @details Sets GPIO pins 30:31 to bit pattern '10'.
///
////////////////////////////////////////////////////////////////////////////////
inline void ApexDiagIoShutdown::SetNormalTestBitSet()
{
    // Clear output bits
    HI_ApexReg.GpioOutput &= ~HI_MASK_GPIO_IOSHUTDOWN_OUTPUT;
    // Set to new value
    HI_ApexReg.GpioOutput |= GPIO3130_NORMAL_TEST_BITS;
}

////////////////////////////////////////////////////////////////////////////////
//  SetInvertedTestBitSet
///
/// @memberof ApexDiagIoShutdown
///
/// @details Sets GPIO pins 30:31 to bit pattern '01'.
///
////////////////////////////////////////////////////////////////////////////////
inline void ApexDiagIoShutdown::SetInvertedTestBitSet()
{
    // Clear output bits
    HI_ApexReg.GpioOutput &= ~HI_MASK_GPIO_IOSHUTDOWN_OUTPUT;
    // Set to new value
    HI_ApexReg.GpioOutput |= GPIO3130_INVERTED_TEST_BITS;
}

////////////////////////////////////////////////////////////////////////////////
//  CheckForNormalTestBits
///
/// @memberof ApexDiagIoShutdown
///
/// @details Check GPIO pins 28:29 for bit pattern '10'.
///
/// @return bool        
///     @retval true    If GPIO bits 28:29 are set to '10'.
///     @retval false   In all other cases. 
////////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagIoShutdown::CheckForNormalTestBits()
{ 
    for (INT32 tries = 0; tries < GPIO_PROPAGATION_RETRIES; tries++)
    {
        if ((HI_ApexReg.GpioInput & HI_MASK_GPIO_IOSHUTDOWN_INPUT) == GPIO2928_NORMAL_EXPECTED_BITS)
        {
            return true;
        }
    }
    
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//  CheckForInvertedTestBits
///
/// @memberof ApexDiagIoShutdown
///
/// @details Check GPIO pins 28:29 for bit pattern '01'.
///
/// @return bool        
///     @retval true    If GPIO bits 28:29 are set to '01'.
///     @retval false   In all other cases. 
////////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagIoShutdown::CheckForInvertedTestBits()
{
    for (INT32 tries = 0; tries < GPIO_PROPAGATION_RETRIES; tries++)
    {
        if ((HI_ApexReg.GpioInput & HI_MASK_GPIO_IOSHUTDOWN_INPUT) == GPIO2928_INVERTED_EXPECTED_BITS)
        {
            return true;
        }
    }
    
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//  SetTestFaultLine
///
/// @memberof ApexDiagIoShutdown
///
/// @details Sets TEST_FAULT_DIAG line.
///
////////////////////////////////////////////////////////////////////////////////
inline void ApexDiagIoShutdown::SetTestFaultLine()
{
    HI_ApexReg.GpioOutput |= HI_BIT_GPIO_IOSHUTDOWN_TEST_FAULT;
}

////////////////////////////////////////////////////////////////////////////////
//  ClearTestFaultLine
///
/// @memberof ApexDiagIoShutdown
///
/// @details Clears TEST_FAULT_DIAG pin.
///
////////////////////////////////////////////////////////////////////////////////
inline void ApexDiagIoShutdown::ClearTestFaultLine()
{
    HI_ApexReg.GpioOutput &= ~HI_BIT_GPIO_IOSHUTDOWN_TEST_FAULT;
}

////////////////////////////////////////////////////////////////////////////////
//  IsVmsFaultApexDiagSet
///
/// @memberof ApexDiagIoShutdown
///
/// @details Check is VMS FAULT_APEX_DIAG is set.
///
/// @return bool        
///     @retval true    If VMS FAULT_APEX_DIAG line is set.
///     @retval false   If VMS FAULT_APEX_DIAG line is not set. 
////////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagIoShutdown::IsVmsFaultApexDiagSet()
{
    return (0 != (HI_ApexReg.GpioInput & HI_BIT_GPIO_IOSHUTDOWN_VMS_FAULT));
}

////////////////////////////////////////////////////////////////////////////////
//  ApexDiagIoShutdown::TestFunctionFirmExceptAdapter
///
/// @memberof ApexDiagIoShutdown
///
/// @details Wrapper adapting firmExcept to TestFunction. This function is just a sanity
///          check for reentering power up state (which should never happen).
///
/// @return TestPhase                  Returns state the module should move to.
///     @retval DGN_IOSHUTDOWN_POWERUP Dummy value, this function should never return. 
////////////////////////////////////////////////////////////////////////////////
inline ApexDiagIoShutdown::TestPhase ApexDiagIoShutdown::TestFunctionFirmExceptAdapter()
{
    firmExcept();
    // should never get here
    return POWERUP;
}

#endif // #if !defined(APEXDIAGIOSHUTDOWN_HPP) 
