///////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArmCache.cpp
///
/// @details ARM cache diagnostics.
///
/// @par Full Description
/// Contains ARM cache diagnostics tests code.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - fzembok  21-OCT-2013 ARM Cache diagnostics.
/// - fzembok  05-NOV-2013 Fault injection fixes
/// - mgrad    27-NOV-2013 MISRA fixes
/// - dtstalte 27-JAN-2014 Make completion check interrupt driven.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagArmCache.hpp"     // For class declaration
#include "Apex.hpp"                 // For HI_BIT_INTR_GTMR1

// FORWARD REFERENCES
// (none)

// This pattern set specifically designed to detect individual memory cell shorts,
// opens, and stuck at conditions, data signal line shorts and opens,
// as well as coupling faults between bits in each register.
// This pattern set should be rotated (the order of the patterns changed)
// and inverted (all bits in each word are inverted)
// to provide full diagnostic coverage.
const UINT32 ApexDiagArmCache::m_PatternSet[] =
{
    0x99c966c6,
    0x9c966c69,
    0xc966c699,
    0x966c699c,
};

bool ApexDiagArmCache::m_isTestInProgress = false;

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: PollForBistStatus
//
/// Polls for cache BIST status until status is determined or timeout happens.
///
/// @par Full Description
/// Periodically polls for cache BIST status and interprets it.
/// If status is success then function execution ends.
/// If status is not completed then checks is timeout was not exceeded. 
/// If timeout is exceeded then ASSERTs.
/// If status is fail then ASSERTs.
/// 
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArmCache::PollForBistStatus()
{
    CacheBistStatus cacheBistStatus = SUCCESS;
    UINT32 timeSinceStart = 0;
    
    // capture time when test was started
    UINT32 startTimestamp = HI_ApexReg.SystemTime;

    // poll for status until it is determined or timeout happens
    while ( true )
    {
        // get status
        cacheBistStatus = GetCacheBistStatus();
        
        if (SUCCESS == cacheBistStatus)
        {
            return;
        }
        else if (DATA_CACHE_BIST_NOT_COMPLETED == cacheBistStatus || 
                 INSTRUCTION_CACHE_BIST_NOT_COMPLETED == cacheBistStatus)
        {
            // how long cache BIST is running?
            timeSinceStart = HI_ApexReg.SystemTime - startTimestamp;

            // check if timeout was exceeded
            if ( timeSinceStart > MAX_CACHE_TEST_DURATION_US )
            {
                // cache BIST not finished in allowed time
                ASSERT( 0 );
            }
        }
        else
        {
             ASSERT( 0 );
        }
    } // end: while ( true )          
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ArmCachePowerUp
//
/// Executes powerup stage of caches test.
///
/// @par Full Description
/// Runs BIST for both caches.
/// Runs BIST for all patterns from pattern set.
/// Warning:
/// This function disables caches and interrupts for time it is executed.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArmCache::ArmCachePowerUp()
{
    // disable interrupts and remember their state before disabling
    const bool irqEnabled = HI_DisableIrq();
    const bool fiqEnabled = HI_DisableFiq();

    // disable caches for duration of BIST 
    DisableDataCache();
    DisableInstrCache();
    
    // for each pattern
    for ( UINT32 patternIndex = 0; 
          patternIndex < ( sizeof( m_PatternSet ) / sizeof( *m_PatternSet ) ); 
          ++patternIndex)
    {
        StartCacheBist( m_PatternSet[ patternIndex ] );
        
        // poll for result until it is determined or timeout happens
        PollForBistStatus();
    }
    
    // invalidate and enable caches
    FlushInstrCache();
    EnableInstrCache();
    FlushDataCache();
    EnableDataCache();

    // enable interrupts if they were enabled before
    if ( fiqEnabled )
    {
        HI_EnableFiq();
    }
    if ( irqEnabled )
    {
        HI_EnableIrq();
    }
    
    // Enable and seed timer1 for the run time test
    HI_interruptEnable(HI_BIT_INTR_GTMR1);
    HI_ApexReg.Timer[1].Seed = MAX_CACHE_TEST_DURATION_US;
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: RunTest
//
/// Step 1 of ARM caches test. Starts caches test.
///
/// @par Full Description
/// Starts ARM caches test with one of 4 bit patterns.
/// Both caches are tested in parallel with use of BIST. 
/// Caches test is performed in 2 steps. 
/// Step 1. Disable caches, start BIST with given pattern.
/// Step 2. Check result of BIST, invalidate caches, enable caches.
/// This function realizes step 1.
/// Step 2 is realized by ApexDiagArmCache::ArmCacheCheckResult function. 
/// Warning:
/// This function disables caches.
/// Between steps 1 and 2 both caches are disabled.
///
/// @param      not used
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagArmCache::RunTest( ApexDiagnostic::DiagControlBlock *)
{          
    static UINT32 patternIndex = 0;
        
    // disable interrupts and remember their state before disabling
    const bool irqEnabled = HI_DisableIrq();
    const bool fiqEnabled = HI_DisableFiq();

    // make sure test was not already started
    ASSERT(false == m_isTestInProgress);

    m_isTestInProgress = true;
    
    DisableDataCache();
    DisableInstrCache();
    
    StartCacheBist( m_PatternSet[ patternIndex++ ] );
    
    // If the module is under heavy TX/RX load when this test is run
    // there is a strong possibility we will have to continuously process
    // interrupts since the caches are now off.  This prevents us from
    // returning to the background loop which is where the next diagnostic
    // slice would be invoked from (checking if the BIST is completed).
    // Since we can't disable interrupts for the entire BIST so as to not
    // impact motion, set an interrupt to go off to verify BIST completion
    // and turn the caches back on.
    HI_ApexReg.Timer[1].Value = MAX_CACHE_TEST_DURATION_US;
    HI_ApexReg.Timer[1].Enable = true;
    
    // enable interrupts if they were enabled before
    if ( fiqEnabled )
    {
        HI_EnableFiq();
    }
    if ( irqEnabled )
    {
        HI_EnableIrq();
    }

    // last pattern
    if ( patternIndex == ( sizeof( m_PatternSet ) / sizeof( *m_PatternSet ) ) )
    {
        // reset pattern counter
        patternIndex = 0;

        // all patterns have been tested
        return ApexDiagnostic::TEST_LOOP_COMPLETE;
    }
    
    return ApexDiagnostic::TEST_IN_PROGRESS;
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ArmCacheCheckResult
//
/// Step 2 of ARM caches test. Checks result of test.
///
/// @par Full Description
/// Checks following preconditions: if test was started,  if enough time ( 250 us ) 
/// has passed to expect test is completed ( BIST test takes 178 us ).
/// If above preconditions are met, asserts if test result is success.
/// Caches test is performed in 2 steps. 
/// Step 1. Disable caches, start BIST with given pattern.
/// Step 2. Check result of BIST, invalidate caches, enable caches.
/// This function realizes step 2 and is the registered soft vector for
/// the timer1 interrupt.
/// Warning:
/// This function enables caches if preconditions are met.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArmCache::ArmCacheCheckResult()
{
    // Clear the interrupt, disable timer1
    HI_ApexReg.Timer[1].Intr = 0;
    HI_ApexReg.Timer[1].Enable = false;
        
    // This can now only be entered from the IRQ handler
    // so there is no need to disable interrupts (the mode
    // change will have done that for us).

    // assert if result is success
    ASSERT(SUCCESS == GetCacheBistStatus());

    EndCacheBist();

    FlushInstrCache();
    EnableInstrCache();
    FlushDataCache();
    EnableDataCache();
    
    // set test not in progress
    m_isTestInProgress = false;
}

