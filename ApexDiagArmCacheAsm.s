;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @file ApexDiagArmCacheAsm.s
;;;
;;; @details ARM946E-S Cache tests.
;;;
;;; @par Full Description
;;; Contains ARM946E-S Cache tests functions definitions.
;;;
;;; @if REVISION_HISTORY_INCLUDED
;;; @par Edit History
;;; -   fzembok 21-Oct-2013 ARM Cache diagnostics.
;;; -   fzembok 05-Nov-2013 Fault injection fixes
;;; -   pszramo 04-Nov-2013 Renamed fault injection point to match convention.
;;; -   mgrad   28-NOV-2013 File name changed during MISRA fixes
;;; @endif
;;;
;;; @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Imported symobls
;; (none)

;; Exported symobls
	EXPORT 	DisableDataCache
	EXPORT 	FlushDataCache
	EXPORT	EnableDataCache
	
	EXPORT 	DisableInstrCache
	EXPORT	FlushInstrCache
	EXPORT 	EnableInstrCache
	
	EXPORT	StartCacheBist
	EXPORT	GetCacheBistStatus
	EXPORT  EndCacheBist
    
;; Constants

;; Control Register bits offsets
DATA_CACHE_ENABLE_BIT_OFFSET            EQU 2
INSTRUCTION_CACHE_ENABLE_BIT_OFFSET     EQU 12

;; Cache RAM BIST Control Register bit offsets
DATA_BIST_START_BIT_OFFSET              EQU 0
DATA_BIST_ENABLE_BIT_OFFSET             EQU 2
DATA_BIST_FAIL_BIT_OFFSET               EQU 3
DATA_BIST_COMPLETE_BIT_OFFSET           EQU 4
DATA_BIST_SIZE_BITS_OFFSET              EQU 5
INSTRUCTION_BIST_START_BIT_OFFSET       EQU 16
INSTRUCTION_BIST_ENABLE_BIT_OFFSET      EQU 18
INSTRUCTION_BIST_FAIL_BIT_OFFSET        EQU 19
INSTRUCTION_BIST_COMPLETE_BIT_OFFSET    EQU 20
INSTRUCTION_BIST_SIZE_BITS_OFFSET       EQU 21
  
;; BIST size
BIST_AREA_SIZE_8_KB                     EQU 11

;; Cache structure contants
DATA_CACHE_LINE_COUNTER_STEP            EQU 0x20
DATA_CACHE_SEGMENT_SIZE                 EQU 0x800
DATA_CACHE_SEGMENT_COUNTER_STEP         EQU 0x40000000     

;; BIST status
SUCCESS 								EQU 0
DATA_CACHE_BIST_NOT_COMPLETED 			EQU 1
INSTRUCTION_CACHE_BIST_NOT_COMPLETED 	EQU 2
DATA_CACHE_BIST_FAILED 					EQU 3
INSTRUCTION_CACHE_BIST_FAILED 			EQU 4

;; Code area start
    AREA DGN_ARM_CACHE_TEST, CODE, READONLY

;; Macros

;; Control register read
    MACRO
    READ_CONTROL_REGISTER $reg
        MRC     p15, 0, $reg, c1, c0, 0
    MEND
    
;; Control register write
    MACRO
    WRITE_CONTROL_REGISTER $reg
        MCR     p15, 0, $reg, c1, c0, 0;
    MEND

;; Cache RAM BIST Control Register read
    MACRO
    READ_CACHE_RAM_BIST_CONTROL_REGISTER $reg
        MRC     p15, 2, $reg, c15, c0, 1
    MEND
    
;; Cache RAM BIST Control Register write
    MACRO
    WRITE_CACHE_RAM_BIST_CONTROL_REGISTER $reg
        MCR     p15, 2, $reg, c15, c0, 1
    MEND

;; Instruction Cache RAM BIST Address Register write
    MACRO
    WRITE_INSTRUCTION_CACHE_RAM_BIST_ADDRESS_REGISTER $reg
        MCR     p15, 2, $reg, c15, c0, 2
    MEND
    
 ;; Instruction Cache RAM BIST General Register write
    MACRO
    WRITE_INSTRUCTION_CACHE_RAM_BIST_GENERAL_REGISTER $reg
        MCR     p15, 2, $reg, c15, c0, 3
    MEND 

;; Disable instruction cache
    MACRO
    DISABLE_INSTR_CACHE $reg
        READ_CONTROL_REGISTER $reg
        BIC     $reg, $reg, #( 1 << INSTRUCTION_CACHE_ENABLE_BIT_OFFSET )
        WRITE_CONTROL_REGISTER $reg
    MEND
  
;; Enable instruction cache
    MACRO
    ENABLE_INSTR_CACHE $reg
        READ_CONTROL_REGISTER $reg
        ORR     $reg, $reg, #( 1 << INSTRUCTION_CACHE_ENABLE_BIT_OFFSET )
        WRITE_CONTROL_REGISTER $reg
    MEND

;; Flush instruction cache
    MACRO
    FLUSH_INSTR_CACHE $reg
        MOV     $reg, #0
        MCR     p15, 0, $reg, c7, c5, 0
    MEND

;; Data Cache RAM BIST Address Register write
    MACRO
    WRITE_DATA_CACHE_RAM_BIST_ADDRESS_REGISTER $reg
        MCR     p15, 2, $reg, c15, c0, 6
    MEND

 ;; Data Cache RAM BIST General Register write
    MACRO
    WRITE_DATA_CACHE_RAM_BIST_GENERAL_REGISTER $reg
        MCR     p15, 2, $reg, c15, c0, 7
    MEND 

;; Disable data cache
    MACRO
    DISABLE_DATA_CACHE $reg
        READ_CONTROL_REGISTER $reg
        BIC     $reg, $reg, #( 1 << DATA_CACHE_ENABLE_BIT_OFFSET )
        WRITE_CONTROL_REGISTER $reg
    MEND
    
;; Enable data cache
    MACRO
    ENABLE_DATA_CACHE $reg
        READ_CONTROL_REGISTER $reg
        ORR     $reg, $reg, #( 1 << DATA_CACHE_ENABLE_BIT_OFFSET )
        WRITE_CONTROL_REGISTER $reg
    MEND  
    
;; Clean and flush data cache entry
    MACRO
    CLEAN_AND_FLUSH_DATA_CACHE_ENTRY $reg
        MCR     p15, 0, $reg, c7, c14, 2
    MEND
    
;; Clean data cache entry
    MACRO
    CLEAN_DATA_CACHE_ENTRY $reg
        MCR     p15, 0, $reg, c7, c10, 2
    MEND
    
;; Flush data cache
    MACRO
    FLUSH_DATA_CACHE $reg
        MOV     $reg, #0
        MCR     p15, 0, $reg, c7, c6, 0
    MEND
    
;; Drain write buffer
    MACRO
    DRAIN_WRITE_BUFFER $reg
        MOV     $reg, #0
        MCR     p15, 0, $reg, c7, c10, 4
    MEND
    
;; Functions
        
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DisableDataCache
;;
;; Disables data cache and synchronizes it with main memory.
;;
;; @par Full Description
;; Performs following steps
;; 1. Disable data cache
;; 2. Clean data cache ( write data from cache to main memory )
;; 3. Drain write buffer
;; Warning:
;; This function should be executed in one piece. Interrupts should
;; be disabled for the time this function is executed.
;; Before data cache is reenabled it must be flushed (invalidated).  
;; Folowing sequence of calls should happen:
;; 1. DisableDataCache
;; 2. Do things with data cache disabled
;; 3. FlushDataCache
;; 4. EnableDataCache
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
DisableDataCache

;; 1. Disable data cache
    DISABLE_DATA_CACHE r0
    
;; 2. Clean data cache ( write data from cache to main memory )
;; r3 - segment counter
    MOV     r3, #0   
       
;; segment loop            
DisableDataCacheNextSegment

;; r2 - line counter
    MOV     r2, #0     
  
;; line loop               
DisableDataCacheNextLine

;; r1 - word address
    ORR     r1, r3, r2  
               
;; clean data cache entry
    CLEAN_DATA_CACHE_ENTRY r1
        
;; next line
    ADD     r2, r2, #( DATA_CACHE_LINE_COUNTER_STEP )    
           
;; all lines in segment done?
    CMP     r2, #( DATA_CACHE_SEGMENT_SIZE )              
    BNE     DisableDataCacheNextLine  
                      
;; next segment
    ADD     r3, r3, #( DATA_CACHE_SEGMENT_COUNTER_STEP )   

;; all segments done?
    CMP     r3, #0               
    BNE     DisableDataCacheNextSegment    

;; 3. Drain write buffer
    DRAIN_WRITE_BUFFER r2
    
;; return
    BX      lr
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DisableInstrCache
;;
;; Disables instruction cache.
;;
;; @par Full Description
;; Disables instruction cache.
;; Warning:
;; Before instruction cache is reenabled it must be flushed (invalidated).  
;; Folowing sequence of calls should happen:
;; 1. DisableInstrCache
;; 2. Do things with instruction cache disabled
;; 3. FlushInstrCache
;; 4. EnableInstrCache
;; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
DisableInstrCache
    DISABLE_INSTR_CACHE r0
    BX      lr     

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; EnableDataCache
;;
;; Enables data cache.
;;
;; @par Full Description
;; Enables data cache.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
EnableDataCache
    ENABLE_DATA_CACHE r0
    BX      lr
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; EnableInstrCache
;;
;; Enables instruction cache.
;;
;; @par Full Description
;; Enables instruction cache.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
EnableInstrCache
    ENABLE_INSTR_CACHE r0
    BX      lr     
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FlushDataCache
;;
;; Flushes data cache.
;;
;; @par Full Description
;; Flushes data cache. Inalidates all lines in data cache.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
FlushDataCache
    FLUSH_DATA_CACHE r0
    BX      lr
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FlushInstrCache
;;
;; Flushes data cache.
;;
;; @par Full Description
;; Flushes instruction cache. Inalidates all lines in instruction cache.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
FlushInstrCache
    FLUSH_INSTR_CACHE r0
    BX      lr  
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; StartCacheBist
;;
;; Starts BIST for both caches.
;;
;; @par Full Description
;; Performs following steps
;; 1. Setup BIST
;; 2. Start BIST
;; Warning: 
;; Before calling this function both caches should be disabled.
;; Before calling this function interrupts should be disabled. 
;; 
;; @param   UINT32  pattern     Pattern to be used by BIST
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   
StartCacheBist

;; 1. Setup BIST
;; enable BIST and set its size
    MOV     r1, #( ( 1 << DATA_BIST_ENABLE_BIT_OFFSET ) | ( BIST_AREA_SIZE_8_KB << DATA_BIST_SIZE_BITS_OFFSET ) )
    MOV     r2, #( ( 1 << INSTRUCTION_BIST_ENABLE_BIT_OFFSET ) | ( BIST_AREA_SIZE_8_KB << INSTRUCTION_BIST_SIZE_BITS_OFFSET) )
    ORR     r1, r1, r2
         
    WRITE_CACHE_RAM_BIST_CONTROL_REGISTER r1

;; set BIST start address
    MOV     r2, #0
    WRITE_DATA_CACHE_RAM_BIST_ADDRESS_REGISTER r2
    WRITE_INSTRUCTION_CACHE_RAM_BIST_ADDRESS_REGISTER r2
     
;; set BIST pattern
    WRITE_DATA_CACHE_RAM_BIST_GENERAL_REGISTER r0
    WRITE_INSTRUCTION_CACHE_RAM_BIST_GENERAL_REGISTER r0

;; 2. Start BIST
    ORR     r1, r1, #( 1 << DATA_BIST_START_BIT_OFFSET )
    ORR     r1, r1, #( 1 << INSTRUCTION_BIST_START_BIT_OFFSET )
    WRITE_CACHE_RAM_BIST_CONTROL_REGISTER r1
    
;; return 
	BX		lr    
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; GetCacheBistStatus
;;
;; Returns status of BIST started by StartCacheBist.
;;
;; @par Full Description
;; Performs following steps
;; 1. Read BIST status register
;; 2. Check if data cache BIST is completed ( return 1 if not )
;; 3. Check if instruction cache BIST is completed ( return 2 if not )
;; 4. Check if data cache BIST fail flag is set ( return 3 if it is )
;; 5. Check if instruction cache BIST fail flag is set ( return 4 if it is )
;; 6. Return success ( 0 )
;; 
;; @return  UINT32	Test result
;;  @retval 0    	Success
;;  @retval 1   	Data cache BIST not completed
;;  @retval 2   	Instruction cache BIST not completed
;;  @retval 3   	Data cache BIST failed
;;  @retval 4   	Instruction cache BIST failed
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
GetCacheBistStatus        
;; 1. Read BIST status register
    READ_CACHE_RAM_BIST_CONTROL_REGISTER r1
    
;; 2. Check if data cache BIST is completed ( return 1 if not )
    TST     r1, #( 1 << DATA_BIST_COMPLETE_BIT_OFFSET )    
    MOVEQ   r0, #DATA_CACHE_BIST_NOT_COMPLETED
    BXEQ    lr
    
;; 3. Check if instruction cache BIST is completed ( return 2 if not )
    TST     r1, #( 1 << INSTRUCTION_BIST_COMPLETE_BIT_OFFSET )    
    MOVEQ   r0, #INSTRUCTION_CACHE_BIST_NOT_COMPLETED
    BXEQ    lr   

;; 4. Check if data cache BIST fail flag is set ( return 3 if it is )
    MOV     r2, #( 1 << DATA_BIST_FAIL_BIT_OFFSET )
    TST		r1, r2
    MOVNE	r0, #DATA_CACHE_BIST_FAILED
    BXNE    lr 
    
;; 5. Check if instruction cache BIST fail flag is set ( return 4 if it is )
    MOV     r2, #( 1 << INSTRUCTION_BIST_FAIL_BIT_OFFSET )
    TST     r1, r2
    MOVNE	r0, #INSTRUCTION_CACHE_BIST_FAILED
    BXNE    lr 
 
;; 6. Return success ( 0 )   
    MOV		r0, #SUCCESS
    
;; Start Fault Injection Point 1
;;	MOV		r0, #DATA_CACHE_BIST_FAILED
;; End Fault Injection Point 1
   
;; Start Fault Injection Point 2
;;	MOV		r0, #INSTRUCTION_CACHE_BIST_FAILED
;; End Fault Injection Point 2
    
    BX		lr
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; EndCacheBist
;;
;; Ends BIST for both caches.
;;
;; @par Full Description
;; Disables BIST for both caches.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   
EndCacheBist  
    MOV     r0, #0
    WRITE_CACHE_RAM_BIST_CONTROL_REGISTER r0
    BX		lr
    
;; END directive
	END
