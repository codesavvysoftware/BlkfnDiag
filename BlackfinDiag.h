#pragma once

#include <time.h>
#include "Defs.h"
#include "Os_iotk.h"


namespace DiagnosticCommon {

	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 
	typedef UINT32  DiagElapsedTime_t;
	typedef clock_t DiagTimestampTime_t;

	//
	// Requirement:  All Diagnostic Tests Complete in 4 Hours.
	//
    static const DiagElapsedTime_t PeriodForAllDiagnosticsCompleted_milleseconds = 4 * 60 * 60 * 1000; // 4 hours
    
    static const DiagElapsedTime_t PeriodForOneDiagnosticTestIteration_milleseconds = 50;
    
    static const DiagTimestampTime_t ClocksPerMillesecond = CLOCKS_PER_SEC / 1000;
    
    static const DiagTimestampTime_t ClocksPerMicrosecond = CLOCKS_PER_SEC / 1000000;
     
	static const UINT32 DiagnosticErrorNumberMask = 0x7fffffff;

	static const UINT32 DiagnosticErrorTestTypeBitPos = 26;
	
	class cTiming {
		
		public:
		
		cTiming() : divisorApproxClocksToMS    ( 0 ), 
		            adjustmentApproxClocksToMS ( 0 ),
		            divisorApproxClocksToUS    ( 0 ),
		            adjustmentApproxClocksToUS ( 0 ) {			
			
			CalcShiftFactorsForCountingMilleseconds( divisorApproxClocksToMS, adjustmentApproxClocksToMS );
			CalcShiftFactorsForCountingMicroseconds( divisorApproxClocksToUS, adjustmentApproxClocksToUS );
		}
				
		DiagTimestampTime_t GetSystemTimestamp() {
			return clock();
		}

		void ComputeElapsedTimeMS( DiagTimestampTime_t   current, 
				                   DiagTimestampTime_t   previous, 
                                   DiagElapsedTime_t   & elapsed  ) {
            ComputeElapsedTime( current, previous, divisorApproxClocksToMS, adjustmentApproxClocksToMS, elapsed );
        }

        void ComputeElapsedTimeUS( DiagTimestampTime_t   current, 
				                   DiagTimestampTime_t   previous, 
                                   DiagElapsedTime_t   & elapsed  ) {
            ComputeElapsedTime( current, previous, divisorApproxClocksToUS, adjustmentApproxClocksToUS, elapsed );
        }
        
		private:
		
		UINT32   divisorApproxClocksToMS;
			
		UINT32   adjustmentApproxClocksToMS;
		
		UINT32   divisorApproxClocksToUS;
			
		UINT32   adjustmentApproxClocksToUS;
		
		void CalcShiftFactorsForCountingMilleseconds( UINT32 & shift_factor, UINT32 & shift_adjustment ) {
			
			CalcShiftFactors( divisorApproxClocksToMS, adjustmentApproxClocksToMS, ClocksPerMillesecond );
		}
		
		void CalcShiftFactorsForCountingMicroseconds( UINT32 & shift_factor, UINT32 & shift_adjustment ) {
			
			CalcShiftFactors( divisorApproxClocksToUS, adjustmentApproxClocksToUS, ClocksPerMicrosecond );
		}
		
		void CalcShiftFactors( UINT32 & shift_factor, UINT32 & shift_adjustment, DiagTimestampTime_t ClockTimeRate ) {
			//
			// Find MSB of ClocksPerMillesecond
			//
			DiagTimestampTime_t dtt = ClockTimeRate;
	
			UINT32 bitpos = 0;
	
			while (dtt != 0 ) {
				++bitpos;
		
				dtt >>= 1;
			}
	
    		dtt = CLOCKS_PER_SEC;
    
    		dtt >>= bitpos-1;
    
    		UINT32 i = 0;
    
    		DiagTimestampTime_t error = 0;
    
    		DiagTimestampTime_t prev_error = 0;
    
    		for ( i = 1; i < bitpos - 1; ++i ) {
    	
    			error = ( dtt - (dtt >> i ) );	
    	
    			if ( error > 1000 )	break;
    	
    			prev_error = error;
    		}
    
    		//
    		// Pick the adjustment that yiels the closest value to 1000
    		//
    
    		DiagTimestampTime_t lower = 1000 - prev_error;
    
    		DiagTimestampTime_t upper = error - 1000;
    
    		if ( upper < lower ) {
    			shift_adjustment = i;
    		}
    		else {
    			shift_adjustment = i-1;
    		}
    
    		shift_factor = bitpos -1;
    
		}

        void ComputeElapsedTime( DiagTimestampTime_t   current, 
				                 DiagTimestampTime_t   previous, 
				                 UINT32                divisor,
				                 UINT32                adjShift,
                                 DiagElapsedTime_t   & elapsed  ) {
                                   
             DiagTimestampTime_t diff       = current - previous; // difference in clock cycles;
	
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
	                                                            
			DiagTimestampTime_t   fast = diff >> divisor;
	
			fast -= (fast >> adjShift );

			elapsed = fast;  // difference in clock cycles times milleseconds per clock cycle
	                                        // yields elapsed time in milleseconds
			//	DiagTimestampTime_t rate = CLOCKS_PER_SEC;
	
			//	diff *= 1000;
	
			//	diff /= rate;     // milleseconds per clock cycle is what ends up in multiplier
		}
	};
	
	static cTiming SystemTiming;    
};


