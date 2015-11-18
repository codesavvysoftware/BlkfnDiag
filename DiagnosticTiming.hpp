#include "Defs.h"
#include "DiagnosticDefs.h"

namespace DiagnosticTiming 
{
	#define DIVISOR_INITIAL_VALUE    19

	#define ADJ_INITIAL_SHIFT_FACTOR  3
	
	class cTiming 
	{
		
		public:
		
		    cTiming() : m_DivisorApproxClocksToMS    (DIVISOR_INITIAL_VALUE),
		                m_AdjustmentApproxClocksToMS (ADJ_INITIAL_SHIFT_FACTOR)
			{			
			    CalcShiftFactorsForCountingMilleseconds( m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS );
		    }
				
		    DiagTimestampTime GetSystemTimestamp() 
			{
			    return clock();
		    }

		    UINT32 ComputeElapsedTimeMS( clock_t  current, 
			                             clock_t  previous ) 
			{
                return ComputeElapsedTime( current, previous, m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS );
            }

        
		private:
		
		    UINT32   m_DivisorApproxClocksToMS;
			
		    UINT32   m_AdjustmentApproxClocksToMS;
		
		
		    void CalcShiftFactorsForCountingMilleseconds( UINT32 & rShift_factor, UINT32 & rShift_adjustment ) 
			{
			    CalcShiftFactors( m_DivisorApproxClocksToMS, m_AdjustmentApproxClocksToMS, (CLOCKS_PER_SEC / 1000 ));
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
	
    		    // Scale to the seoond
    		    dtt = CLOCKS_PER_SEC;
    
    		    dtt >>= bitpos-1;
    
    		    UINT32 i = 1;
    
    		    DiagTimestampTime error = 0;
    
    		    DiagTimestampTime prev_error = 0;
    
    		    for ( ; i < bitpos - 1; ++i ) 
				{    	
    			    error = ( dtt - (dtt >> i ) );	
    	
    		    	if ( error > 1000 )	break;
    	
    			    prev_error = error;
    		    }
    
    		    //
    		    // Pick the adjustment that yiels the closest value to 1000 milleseconds/ 1 second
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

            UINT32 ComputeElapsedTime( clock_t   current, 
			                           clock_t   previous, 
			                           UINT32    divisor,
			                           UINT32    adjShift ) 
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

			    return fast;  // difference in clock cycles times milleseconds per clock cycle
		    }
	   };
	
	   static cTiming SystemTiming; 
	   

	   static UINT32 ComputeElapsedTimeMS( clock_t current, clock_t previous ) 
	   {
	        return SystemTiming.ComputeElapsedTimeMS( current, previous );
	   }
	   
	   static clock_t (*GetTimestamp)() = &clock;  
	   
	   static UINT32  (*CalcElapsedTimeMS)( clock_t current, clock_t previous ) = &ComputeElapsedTimeMS; 
       
};


