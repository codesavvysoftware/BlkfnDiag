#pragma once


#define AllOnesPattern             0xffff
#define AllZerosPattern            0
#define AlternatingOnesZeros       0xaaaa
#define AlternatingZerosOnes       0x5555

#define R0R1SanityCheckFail        1
#define R0R2SanityCheckFail        2
#define R1R2SanityCheckFail        3
#define P0SanityCheckFail          4

#define SanityCheckFailure         1
#define DataRegFailure             2
#define PointerRegFailure          3
#define AccumFailure               4
#define IndexRegFailure            5
#define ModifyRegFailure           6
#define LengthRegFailure           7
#define BaseRegFailure             8

#define R7Failure                  8
#define R6Failure                  7
#define R5Failure                  6
#define R4Failure                  5
#define R3Failure                  4

#define P5Failure                  6    
#define P4Failure                  5    
#define P3Failure                  4    
#define P2Failure                  3    
#define P0Failure                  1    

#define Modify3Failure             4
#define Modify2Failure             3
#define Modify1Failure             2
#define Modify0Failure             1

#define Length3Failure             4
#define Length2Failure             3
#define Length1Failure             2
#define Length0Failure             1

#define Idx3Failure                4
#define Idx2Failure                3
#define Idx1Failure                2
#define Idx0Failure                1

#define Base3Failure               4
#define Base2Failure               3
#define Base1Failure               2
#define Base0Failure               1

#define A0Failure32                1
#define A0Failure8                 2
#define A1Failure32                3
#define A1Failure8                 4
