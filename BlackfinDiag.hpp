#pragma once

#include <time.h>
#include "Defs.h"
#include "Os_iotk.h"


namespace DiagnosticCommon 
{
	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 
    //
    // Var suffix of:
    // 
    // 	MS == milleseconds
    // 	US == microseconds
    //	
	typedef UINT32  DiagElapsedTime;

	typedef clock_t DiagTimestampTime;

    typedef enum _TestState 
    {
        TEST_LOOP_COMPLETE,
        TEST_IN_PROGRESS,
        TEST_FAILURE,
        TEST_IDLE
	} 
	TestState;

	//
	// Requirement:  All Diagnostic Tests Complete in 4 Hours.
	//
    static const DiagElapsedTime PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS     = 4 * 60 * 60 * 1000; // 4 hours, number of milleseconds in 4 hours
    
    static const DiagElapsedTime PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS = 50; // Milleseconds
    
    static const DiagTimestampTime CLOCKS_PER_MS                             = CLOCKS_PER_SEC / 1000;
    
    static const DiagTimestampTime CLOCKS_PER_US                             = CLOCKS_PER_SEC / 1000000;
     
	static const UINT32 DIAG_ERROR_MASK                                      = 0x7fffffff;

	static const UINT32 DIAG_ERROR_TYPE_BIT_POS                              = 26;
	
	static const UINT32 DIVISOR_INITIAL_VALUE                                = 1;

	static const UINT32 ADJ_INITIAL_SHIFT_FACTOR                             = 0;
		
    class cTiming 
	{
		
		public:
		
		    cTiming() : m_DivisorApproxClocksToMS    (DIVISOR_INITIAL_VALUE),
		                m_AdjustmentApproxClocksToMS (ADJ_INITIAL_SHIFT_FACTOR),
		                m_DivisorApproxClocksToUS    (DIVISOR_INITIAL_VALUE),
		                m_AdjustmentApproxClocksToUS (ADJ_INITIAL_SHIFT_FACTOR)
			{			
			    CalcShiftFactorsForCountingMilleseconds( m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS );
			    CalcShiftFactorsForCountingMicroseconds( m_DivisorApproxClocksToUS, m_AdjustmentApproxClocksToUS );
		    }
				
		    DiagTimestampTime GetSystemTimestamp() 
			{
			    return clock();
		    }

		    void ComputeElapsedTimeMS( DiagTimestampTime   current, 
				                       DiagTimestampTime   previous, 
                                       DiagElapsedTime   & rElapsed  ) 
			{
                ComputeElapsedTime( current, previous, m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS, rElapsed );
            }

            void ComputeElapsedTimeUS( DiagTimestampTime   current, 
				                       DiagTimestampTime   previous, 
                                       DiagElapsedTime   & rElapsed  ) 
		    {
                ComputeElapsedTime( current, previous, m_DivisorApproxClocksToUS, m_AdjustmentApproxClocksToUS, rElapsed );
            }
        
		private:
		
		    UINT32   m_DivisorApproxClocksToMS;
			
		    UINT32   m_AdjustmentApproxClocksToMS;
		
		    UINT32   m_DivisorApproxClocksToUS;
			
		    UINT32   m_AdjustmentApproxClocksToUS;
		
		    void CalcShiftFactorsForCountingMilleseconds( UINT32 & rShift_factor, UINT32 & rShift_adjustment ) 
			{
			    CalcShiftFactors( m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS, CLOCKS_PER_MS );
		    }
		
		    void CalcShiftFactorsForCountingMicroseconds( UINT32 & rShift_factor, UINT32 & rShift_adjustment ) 
			{
			    CalcShiftFactors( m_DivisorApproxClocksToUS, m_AdjustmentApproxClocksToUS, CLOCKS_PER_US );
		    }
		
		    void CalcShiftFactors( UINT32 & rShiftFactor, UINT32 & rShiftAdjustment, DiagTimestampTime clockTimeRate ) 
			{
			    //
			    // Find MSB of ClocksPerMillesecond
			    //
			    DiagTimestampTime dtt = clockTimeRate;
	
			    UINT32 bitpos = 0;
	
			    while (dtt != 0 ) 
				{
				    ++bitpos;
		
				    dtt >>= 1;
			    }
	
    		    dtt = CLOCKS_PER_SEC;
    
    		    dtt >>= bitpos-1;
    
    		    UINT32 i = 0;
    
    		    DiagTimestampTime error = 0;
    
    		    DiagTimestampTime prev_error = 0;
    
    		    for ( i = 1; i < bitpos - 1; ++i ) 
				{    	
    			    error = ( dtt - (dtt >> i ) );	
    	
    		    	if ( error > 1000 )	break;
    	
    			    prev_error = error;
    		    }
    
    		    //
    		    // Pick the adjustment that yiels the closest value to 1000
    		    //
    
    		    DiagTimestampTime lower = 1000 - prev_error;
    
    		    DiagTimestampTime upper = error - 1000;
    
    		    if ( upper < lower ) 
				{
    			    rShiftAdjustment = i;
    		    }
    		    else 
				{
    			    rShiftAdjustment = i-1;
    		    }
    
    		    rShiftFactor = bitpos -1;    
		    }    

            void ComputeElapsedTime( DiagTimestampTime   current, 
				                     DiagTimestampTime   previous, 
				                     UINT32              divisor,
				                     UINT32              adjShift,
                                     DiagElapsedTime   & rElapsed  ) 
			{
				DiagTimestampTime diff       = current - previous; // difference in clock cycles;
	
			    // An approximation that is actually very close when CLOCKS_PER_SEC == 600000000
			    // avoiding a constant divide in the background
			    // in the real code probably will be simpler.
			    //
			    // Math for the approximation
			    //  CLOCKS_PER_SEC == 600000000
			    //  CLOCKS_PER_MILLESECOND = CLOCKS_PER_SEC * SECONDS_PER_MILLESECOND = 600000000 / 1000 = 600000
			    //  Elapsed time in milleseconds = difference in clock readings / CLOCKS_PER_MILLESECOND = diff / 600000;
			    //  600000 == 0x927c0
			    //  We're looking for a shift that is less which would be 0x80000 ==  524288.
			    //  600000 ~= 524288 * 1.14441
			    //  Elapsed time in millesconds ~= diff / (524288 * 1.1441) ~=  ( diff / 0x8000 ) * (1/1.1441) 
			    //                                                          ~=  ( diff >> 19 ) * (.8738 )
			    //                                                          ~=  ( diff >> 19 ) * ( 7/8 )
			    //                                                          ~=  ( diff >> 19 ) ( 1 - 1/8 )
			    //                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) * 1/8)
			    //                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) >> 3 );
			    //                                                        substitute  fast for ( diff >> 19 );
	                                                            
			    DiagTimestampTime   fast = diff >> divisor;
	
			    fast -= (fast >> adjShift );

			    rElapsed = fast;  // difference in clock cycles times milleseconds per clock cycle
		    }
	   };
	
	   static cTiming SystemTiming;    
};


