#include "Defs.h"
#include "DiagnosticDefs.h"

namespace DiagnosticTiming 
{
    //*
    //* The compiler generates much more effecient code both memory and execution speed wise by using constants here.
    //* Originally I had a class that would compute the divisor and adjShift values based on the CLOCKS_PER_SEC 
    //* constant during construction.  Here is how the algorithm works:
    //* Find the most significant bit of the CLOCKS_PER_SECOND rate.  The divisor is most significant bit minus 1.
    //* then use a known value like 1000 milleseconds per second to determine the closest adjustment factor. The 
    //* algoriothm below works well.  I don't use it because a lot of code is generated for the Blackfin as a result.
    //* Just computing the constants by hand and using them results in much better code but is not as generic of
    //* a solution.  It is much better than doing divides though.
    //* 
    //*     void CalcShiftFactors( UINT32 & rShiftFactor, UINT32 & rShiftAdjustment ) 
    //* 		{
    //* 	    //
    //* 	    // Find MSB of ClocksPerMillesecond
	//*	        //
	//* 	    UINT64 dtt = (CLOCKS_PER_SEC / 1000 );
    //* 
    //* 	    UINT32 bitpos = 0;
    //*
    //* 	    while (dtt != 0 ) 
    //* 		{
    //* 		    ++bitpos;
    //*
    //* 		    dtt >>= 1;
    //* 	    }
    //*
    //* 	    // Scale to the seoond
    //* 	    dtt = CLOCKS_PER_SEC;
    //* 
    //* 	    dtt >>= bitpos-1;
    //* 
    //* 	    UINT32 i = 1;
    //* 
    //* 	    UINT64 error = 0;
    //* 
    //* 	    //
    //*         // Determine the best adjustment shift factor that yields the closest value to one second.
    //*         //
    //*         UINT64 prev_error = 0;
    //* 
    //* 	    for ( ; i < bitpos - 1; ++i ) 
	//* 		{    	
    //* 		    error = ( dtt - (dtt >> i ) );	
    //*
    //*             //
    //*             // dtt is scaled to the second but we know it will be greater than one second because we've divided
    //*             // by a value less than CLOCKS_PER_SECOND.  When we start off with i == 1 we're dividing by one half.
    //*             // and subtracting that from the CLOCKS_PER_SECOND scaled to the second approximation.  That number
    //*             // almost always will be less than 1000 milleseconds or a second.  We want to find the value of i 
    //*             // where error is greater than 1 second.  Then by keeping track of the pervious error we know
    //*             // the value of i where the threshold is crossed between less than a second and greater than a 
    //*             // second for the adjmustment shift factor.  Then we pick the value that yields the lowest error IE
    //*             // the value that comes closes to approximating one second.
    //*             if ( error > 1000 )	break;
    //*      
    //*             prev_error = error;
    //*  	    }
    //* 
    //* 	    //
    //* 	    // Pick the adjustment that yiels the closest value to 1000 milleseconds/ 1 second
    //* 	    //
    //* 
    //* 	    UINT64 lower = 1000 - prev_error;
    //* 
    //* 	    UINT64 upper = error - 1000;
    //* 
    //*		    if ( upper < lower ) 
	//* 		{
    //* 		    rShiftAdjustment = i;
   	//* 	    }
   	//* 	    else 
	//* 		{
   	//* 		    rShiftAdjustment = i-1;
   	//* 	    }
    //* 
   	//* 	    rShiftFactor = bitpos -1;   
	//*     }    
        
        
    #define divisor 19
    #define adjShift 3
        
    static UINT32 ComputeElapsedTimeMS( UINT64 current, UINT64 previous ) 
	{
        DiagTimestampTime diff       = current - previous; // difference in clock cycles;
	
        // An approximation that is actually very close when CLOCKS_PER_SEC == 600000000
        // avoiding a constant divide in the background
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
	    UINT64   fast = diff >> divisor;
			    
        fast -= (fast >> adjShift );

		return fast;  // difference in clock cycles times milleseconds per clock cycle
    }
	   
	static UINT64 ReadTimestamp () 
	{
	    UINT64  timestamp = 0;
	       
	    _GET_CYCLE_COUNT( timestamp );
	        
	    return timestamp;
	}
	
	static UINT64 (*GetTimestamp)() = &ReadTimestamp;  
	   
	static UINT32  (*CalcElapsedTimeMS)( UINT64 current, UINT64 previous ) = &ComputeElapsedTimeMS; 
       
};


