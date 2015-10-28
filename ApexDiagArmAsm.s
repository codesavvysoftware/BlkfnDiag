;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @file ApexDiagArmAsm.s
;;;
;;; @details ARM946E-S instructions and registers tests.
;;;
;;; @par Full Description
;;; Contains ARM946E-S instructions and registers diagnostic functions definitions.
;;;
;;; @if REVISION_HISTORY_INCLUDED
;;; @par Edit History
;;; -   fzembok  20-Aug-2013 ARM instructions and registers diagnostics
;;; -   dtstalte 21-Oct-2013 Keep interrupts off when switching modes.
;;; -   fzembok  05-Nov-2013 Fault injection fixes
;;; -   mgrad    28-NOV-2013 File name changed during MISRA fixes
;;; @endif
;;;
;;; @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Imported symobls
;; (none)

;; Start Fault Injection Point 3
;; Allow fault injection flag
;;	IMPORT	InjectFaultFlag
;; End Fault Injection Point 3

;; Exported symobls
	EXPORT	ArmInstrAdd
	EXPORT	ArmInstrAdc
	EXPORT	ArmInstrQadd
	EXPORT	ArmInstrQdadd
    EXPORT	ArmInstrSub
    EXPORT	ArmInstrRsb
	EXPORT	ArmInstrSbc
	EXPORT	ArmInstrRsc
	EXPORT	ArmInstrQsub
	EXPORT	ArmInstrQdsub
	EXPORT	ArmInstrMul
	EXPORT  ArmInstrMla
	EXPORT	ArmInstrUmull
	EXPORT	ArmInstrUmlal
	EXPORT  ArmInstrSmull
	EXPORT	ArmInstrSmlal
	EXPORT	ArmInstrMov
	EXPORT	ArmInstrMvn
	EXPORT	ArmInstrAnd
	EXPORT	ArmInstrOrr
	EXPORT	ArmInstrEor
	EXPORT	ArmInstrBic
	EXPORT	ArmInstrTeq
	EXPORT	ArmInstrTeqCarry
	EXPORT	ArmInstrTst
	EXPORT	ArmInstrTstCarry
	EXPORT	ArmInstrCmp
	EXPORT	ArmInstrCmpCarry
	EXPORT	ArmInstrCmn
	EXPORT	ArmInstrConditionEq
	EXPORT	ArmInstrConditionNe
	EXPORT	ArmInstrConditionHs
	EXPORT	ArmInstrConditionLo
	EXPORT	ArmInstrConditionMi
	EXPORT	ArmInstrConditionPl
	EXPORT	ArmInstrConditionVs
	EXPORT	ArmInstrConditionVc
	EXPORT	ArmInstrConditionHi
	EXPORT	ArmInstrConditionLs
	EXPORT	ArmInstrConditionGe
	EXPORT	ArmInstrConditionLt
	EXPORT	ArmInstrConditionGt
	EXPORT	ArmInstrConditionLe
	EXPORT	ArmInstrSmulbb
	EXPORT	ArmInstrSmulbt
	EXPORT	ArmInstrSmultb
	EXPORT	ArmInstrSmultt
	EXPORT	ArmInstrSmlabb
	EXPORT	ArmInstrSmlabt
	EXPORT	ArmInstrSmlatb
	EXPORT	ArmInstrSmlatt
	EXPORT	ArmInstrSmulwb
	EXPORT	ArmInstrSmulwt
	EXPORT	ArmInstrSmlawb
	EXPORT	ArmInstrSmlawt
	EXPORT	ArmInstrLdr
	EXPORT	ArmInstrLdrb
	EXPORT	ArmInstrLdrh
	EXPORT	ArmInstrLdrsb
	EXPORT	ArmInstrLdrsh
	EXPORT	ArmInstrStr
	EXPORT	ArmInstrStrb
	EXPORT	ArmInstrStrh
	EXPORT	ArmInstrClz
	EXPORT	ArmInstrStmfdLdmfd
	EXPORT	ArmOperand2Immediate
	EXPORT	ArmOperand2LslImmediate
	EXPORT	ArmOperand2LsrImmediate
	EXPORT	ArmOperand2AsrImmediate
	EXPORT	ArmOperand2Register
	EXPORT	ArmOperand2LslRegister
	EXPORT	ArmOperand2LsrRegister
	EXPORT	ArmOperand2AsrRegister
	EXPORT	ArmOperand2RorRegister
	EXPORT	ArmOperand2Rxx
	EXPORT	ArmAddrModeNoOffset
	EXPORT	ArmAddrModeImmediateOffsetPositive
	EXPORT	ArmAddrModeImmediateOffsetNegative
	EXPORT	ArmAddrModeOffsetRegister
	EXPORT	ArmAddrModeOffsetRegisterShift
	EXPORT	ArmAddrModePreInxedImmediateOffset
	EXPORT	ArmAddrModePreInxedOffsetRegister
	EXPORT	ArmAddrModePreInxedOffsetRegisterShift
	EXPORT	ArmAddrModePostInxedImmediateOffset
	EXPORT	ArmAddrModePostInxedOffsetRegister
	EXPORT	ArmAddrModePostInxedOffsetRegisterShift
	EXPORT	ArmRegisterR0
	EXPORT	ArmRegisterR1
	EXPORT	ArmRegisterR2
	EXPORT	ArmRegisterR3
	EXPORT	ArmRegisterR4
	EXPORT	ArmRegisterR5
	EXPORT	ArmRegisterR6
	EXPORT	ArmRegisterR7
	EXPORT	ArmRegisterR8
	EXPORT	ArmRegisterR9
	EXPORT	ArmRegisterR10
	EXPORT	ArmRegisterR11
	EXPORT	ArmRegisterR12
	EXPORT	ArmRegisterSpSystem
	EXPORT	ArmRegisterLrSystem
	EXPORT	ArmRegisterCpsr
	EXPORT	ArmRegisterSpSupervisor
	EXPORT	ArmRegisterLrSupervisor
	EXPORT	ArmRegisterSpAbort
	EXPORT	ArmRegisterLrAbort
	EXPORT	ArmRegisterSpUndefined
	EXPORT	ArmRegisterLrUndefined
	EXPORT	ArmRegisterSpIrq
	EXPORT	ArmRegisterLrIrq
	EXPORT	ArmRegisterSpFiq
	EXPORT	ArmRegisterLrFiq
	EXPORT	ArmRegisterR8Fiq
	EXPORT	ArmRegisterR9Fiq
	EXPORT	ArmRegisterR10Fiq
	EXPORT	ArmRegisterR11Fiq
	EXPORT	ArmRegisterR12Fiq

;; Constants
;; Condition flags from CPSR register.
CPSR_N 			EQU		0x80000000;
CPSR_Z 			EQU		0x40000000;
CPSR_C 			EQU		0x20000000;
CPSR_V 			EQU		0x10000000;
CPSR_CLEAR		EQU 	0x00000000;

;; ARM operating modes from CPSR register.
;; This diagnostic should be entered with interrupts disabled.
;; Changing modes should not re-enable them.
MODE_USER 		EQU		0x000000D0
MODE_FIQ 		EQU		0x000000D1
MODE_IRQ 		EQU 	0x000000D2
MODE_SUPERVISOR	EQU 	0x000000D3
MODE_ABORT		EQU 	0x000000D7
MODE_UNDEFINED	EQU 	0x000000DB
MODE_SYSTEM		EQU 	0x000000DF

;; Code area start
	AREA DGN_ARM_TEST, CODE, READONLY

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrAdd
;;
;; ADD instruction.
;;
;; @par Full Description
;; rd := rn + Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn + operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrAdd   
		ADD		r0, r0, r1
;; Start Fault Injection Point 1
;;    	ADD		r0, r0, #1
;; If above line of code is uncommented then APEX ARM instructions and 
;; registers diagnostics will report fault at the next run of this stage of test
;; End Fault Injection Point 1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrAdc
;;
;; ADC instruction. Add with carry.
;;
;; @par Full Description
;; rd := rn + Operand2 + Carry
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn + operand2 + 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrAdc
;; set carry flag
		MSR     CPSR_f, #CPSR_C
		ADC		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrQadd
;;
;; QADD instruction. Signed Add with saturation.
;;
;; @par Full Description
;; rd := SAT(rm + rn)
;; Saturation here means that when the result crosses the extreme limit of
;; INT32 the value should be maintained at the respective maximum / minimum.
;;
;; @param	INT32	rm	First argument.
;; @param   INT32	rn	Second argument.
;;
;; @return	INT32	SAT(rm + rn)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrQadd
		QADD	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrQdadd
;;
;; QDADD instruction. Signed double and add with saturation.
;;
;; @par Full Description
;; rd := SAT(rm + SAT(rn * 2))
;; Saturation means that when the result crosses the extreme limit of INT32 the
;; value should be maintained at the respective maximum / minimum.
;;
;; @param	INT32	rm	First argument.
;; @param   INT32	rn	Second argument.
;;
;; @return	INT32	SAT(rm + SAT(rn * 2))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrQdadd
		QDADD	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSub
;;
;; SUB instruction.
;;
;; @par Full Description
;; rd := rn - Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn - operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSub
		SUB		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSbc
;;
;; SBC instruction. Subtract with carry.
;;
;; @par Full Description
;; rd := rn � Operand2 � NOT(Carry)
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn - operand2 - NOT(carry)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSbc
;; clear all flags
		MSR     CPSR_f, #CPSR_CLEAR
		SBC		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrRsb
;;
;; RSB instruction. Reverse subtract.
;;
;; @par Full Description
;; rd := Operand2 � rn
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	operand2 - rn
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrRsb
		RSB		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrRsc
;;
;; RSC instruction. Reverse subtract with carry.
;;
;; @par Full Description
;; rd := Operand2 � rn � NOT(Carry)
;;
;; @param	UINT32	rn			First argument.
;; @param   UINT32	operand2	Second argument.
;;
;; @return	UINT32	operand2 - rn - NOT(carry)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrRsc
;; clear all flags
		MSR     CPSR_f, #CPSR_CLEAR
		RSC		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrQsub
;;
;; QSUB instruction. Signed subtract with saturation.
;;
;; @par Full Description
;; rd := SAT(rm � rn)
;; Saturation means that when the result crosses the extreme limit of INT32 the
;; value should be maintained at the respective maximum / minimum.
;;
;; @param	INT32	rm	First argument.
;; @param   INT32	rn	Second argument.
;;
;; @return	INT32	SAT(rm - rn)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrQsub
		QSUB	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrQdsub
;;
;; QDSUB instruction. Signed souble and subtract with saturation.
;;
;; @par Full Description
;; rd := SAT(rm � SAT(rn * 2))
;; Saturation means that when the result crosses the extreme limit of INT32 the
;; value should be maintained at the respective maximum / minimum.
;;
;; @param	INT32	rm	First argument.
;; @param   INT32	rn	Second argument.
;;
;; @return	INT32	SAT(rm � SAT(rn * 2))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrQdsub
		QDSUB	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrMul
;;
;; MUL instruction. Multiply.
;;
;; @par Full Description
;; rd := (rm * rs)[31:0]
;; Truncates result to 32 least significant bits.
;;
;; @param	UINT32	rm	First argument.
;; @param   UINT32	rs	Second argument.
;;
;; @return	UINT32	rm * rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrMul
		MUL		r0, r1, r0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrMla
;;
;; MLA instruction. Multiply accumulate.
;;
;; @par Full Description
;; rd := ((rm * rs) + rn)[31:0]
;; Truncates result to 32 least significant bits.
;;
;; @param	UINT32	rm	First argument.
;; @param   UINT32	rs	Second argument.
;; @param   UINT32	rn	Third argument.
;;
;; @return	UINT32	rm * rs + rn
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrMla
		MLA		r0, r1, r0, r2
		MOV 	pc, lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrUmull
;;
;; UMULL instruction. Unsigned long multiply. 64 bit result.
;;
;; @par Full Description
;; RdHi,RdLo := unsigned(rm * rs)
;;
;; @param	UINT32	rm	First argument.
;; @param   UINT32	rs	Second argument.
;;
;; @return	UINT64	rm * rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrUmull
		UMULL	r2, r3, r0, r1
		MOV		r0, r2
		MOV		r1, r3
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrUmlal
;;
;; UMLAL instruction. Unsigned long multiply, with accumulate. 64 bit result.
;;
;; @par Full Description
;; RdHi,RdLo := unsigned(RdHi,RdLo + rm * rs)
;;
;; @param	UINT64	rd	First argument.
;; @param	UINT32	rm	Second argument.
;; @param   UINT32	rs	Third argument.
;;
;; @return	UINT64	rm * rs + rd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrUmlal
		UMLAL	r0, r1, r2, r3
		MOV 	pc, lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmull
;;
;; SMULL instruction. Signed long multiply. 64 bit result.
;;
;; @par Full Description
;; RdHi,RdLo := signed(rm * rs)
;;
;; @param	INT32	rm	First argument.
;; @param   INT32	rs	Second argument.
;;
;; @return	INT64	rm * rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmull
		SMULL	r2, r3, r0, r1
		MOV		r0, r2
		MOV		r1, r3
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlal
;;
;; SMLAL instruction. Unsigned long multiply, with accumulate. 64 bit result.
;;
;; @par Full Description
;; RdHi,RdLo := signed(RdHi,RdLo + rm * rs)
;;
;; @param	INT64	rd	First argument.
;; @param	INT32	rm	Second argument.
;; @param   INT32	rs	Third argument.
;;
;; @return	INT64	rm * rs + rd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlal
		SMLAL	r0, r1, r2, r3
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrMov
;;
;; MOV instruction.
;;
;; @par Full Description
;; rd := Operand2
;; First and second parameters shall have different values.
;;
;; @param	UINT32	dummy		First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrMov
		MOV		r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrMvn
;;
;; MVN instruction. Move negtive.
;;
;; @par Full Description
;; rd := 0xFFFFFFFF EOR Operand2
;; First and second parameters shall have different values.
;;
;; @param	UINT32	dummy		First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	operand2 EOR 0xFFFFFFFF
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrMvn
		MVN		r0, r1
		MOV 	pc, lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrAnd
;;
;; AND instruction. Bitwise and.
;;
;; @par Full Description
;; rd := rn AND Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn & operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrAnd
		AND		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrOrr
;;
;; ORR instruction. Bitwise or.
;;
;; @par Full Description
;; rd := rn OR Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn | operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrOrr
		ORR		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrEor
;;
;; EOR instruction. Exclusive or.
;;
;; @par Full Description
;; rd := rn EOR Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn ^ operand2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrEor
		EOR		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrBic
;;
;; BIC instruction. Bit clear.
;;
;; @par Full Description
;; rd := rn AND NOT Operand2
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	rn & (~operand2)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrBic
		BIC		r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrTeq
;;
;; TEQ instruction.
;;
;; @par Full Description
;; Performs rn EOR Operand2 operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrTeq
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		TEQ		r0, r1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrTeqCarry
;;
;; TEQ instruction. Set carry bit.
;;
;; @par Full Description
;; Performs rn EOR (Operand2 shifted 1 bit left) operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrTeqCarry
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		TEQ		r0, r1, LSL #1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrTst
;;
;; TST instruction.
;;
;; @par Full Description
;; Performs rn AND Operand2 operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrTst
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		TST		r0, r1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrTstCarry
;;
;; TST instruction. Set carry bit.
;;
;; @par Full Description
;; Performs rn AND (Operand2 shifted 1 bit left) operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrTstCarry
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		TST		r0, r1, LSL #1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrCmp
;;
;; CMP instruction.
;;
;; @par Full Description
;; Performs rn SUB sOperand2 operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C, V
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrCmp
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		CMP		r0, r1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrCmpCarry
;;
;; CMP instruction.
;;
;; @par Full Description
;; Performs rn SUB (Operand2 shifted 1 bit left) operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C, V
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrCmpCarry
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		CMP		r0, r1, LSL #1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrCmn
;;
;; CMN instruction.
;;
;; @par Full Description
;; Performs rn ADD Operand2 operation.
;; Does not store result.
;; Updates CPSR flags N, Z, C, V
;;
;; @param	UINT32	rn			First argument.
;; @param	UINT32	operand2	Second argument.
;;
;; @return	UINT32	cpsr comparison flags
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrCmn
;; clear all comparison result flags
  		MSR     CPSR_f, #CPSR_CLEAR
		CMN		r0, r1
		MRS		r0, CPSR
;; clear all bits except comparison result flags
		AND		r0, r0, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionEq
;;
;; Conditional execution EQ.
;;
;; @par Full Description
;; Tests if condition field EQ (equal) works.
;; Equal is when Z set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionEq
;; initialize result with false
		MOV		r0, #0
;; set all flags but not Z
		MSR     CPSR_f, #( CPSR_N | CPSR_C | CPSR_V )
;; if equal return false
		MOVEQ 	pc, lr
;; set only Z flag
		MSR     CPSR_f, #CPSR_Z
;; if equal return true
		MOVEQ	r0, #1
		MOV 	pc, lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionNe
;;
;; Conditional execution NE.
;;
;; @par Full Description
;; Tests if condition field NE (not equal) works.
;; Not equal is when Z is not set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionNe
;; initialize result with false
		MOV		r0, #0
;; set only Z flag
		MSR     CPSR_f, #CPSR_Z
;; if not equal return false
		MOVNE 	pc, lr
;; set all flags but not Z
		MSR     CPSR_f, #( CPSR_N | CPSR_C | CPSR_V )
;; if not equal return true
		MOVNE	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionHs
;;
;; Conditional execution HS.
;;
;; @par Full Description
;; Tests if condition field HS (unsigned higher or same) works.
;; Unsigned higher or same is when C set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionHs
;; initialize result with false
		MOV		r0, #0
;; set all flags but not C
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_V )
;; if unsigned higher or same return false
		MOVHS 	pc, lr
;; set only C flag
		MSR     CPSR_f, #CPSR_C
;; if unsigned higher or same return true
		MOVHS	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionLo
;;
;; Conditional execution LO.
;;
;; @par Full Description
;; Tests if condition field LO (unsigned lower) works.
;; Unsigned lower is when C is not set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionLo
;; initialize result with false
		MOV		r0, #0
;; set only C flag
		MSR     CPSR_f, #CPSR_C
;; if unsigned lower return false
		MOVLO 	pc, lr
;; set all flags but not C
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_V )
;; if unsigned lower return true
		MOVLO	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionMi
;;
;; Conditional execution MI.
;;
;; @par Full Description
;; Tests if condition field MI (negative) works.
;; Negative is when N is set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionMi
;; initialize result with false
		MOV		r0, #0
;; set all flags but not N
		MSR     CPSR_f, #( CPSR_Z | CPSR_C | CPSR_V )
;; if negative return false
		MOVMI 	pc, lr
;; set only N flag
		MSR     CPSR_f, #CPSR_N
;; if negative return true
		MOVMI	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionPl
;;
;; Conditional execution PL.
;;
;; @par Full Description
;; Tests if condition field PL (positive or zero) works.
;; Positive or zero is when N is not set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; initialize result with false
ArmInstrConditionPl
;; initialize result with false
		MOV		r0, #0
;; set only N flag
		MSR     CPSR_f, #CPSR_N
;; if positive or zero return false
		MOVPL 	pc, lr
;; set all flags but not N
		MSR     CPSR_f, #( CPSR_Z | CPSR_C | CPSR_V )
;; if positive or zero return true
		MOVPL	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionVs
;;
;; Conditional execution VS.
;;
;; @par Full Description
;; Tests if condition field VS (overflow) works.
;; Overflow is when V set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionVs
;; initialize result with false
		MOV		r0, #0
;; set all flags but not V
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_C )
;; if overflow return false
		MOVVS 	pc, lr
;; set only V flag
		MSR     CPSR_f, #CPSR_V
;; if overflow return true
		MOVVS	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionVc
;;
;; Conditional execution VC.
;;
;; @par Full Description
;; Tests if condition field VC (no overflow) works.
;; No overflow is when V is not set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionVc
;; initialize result with false
		MOV		r0, #0
;; set only V flag
		MSR     CPSR_f, #CPSR_V
;; if no overflow return false
		MOVVC 	pc, lr
;; set all flags but not V
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_C )
;; if no overflow return true
		MOVVC	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionHi
;;
;; Conditional execution HI.
;;
;; @par Full Description
;; Tests if condition field HI (unsigned higher) works.
;; Unsigned higher is when C is set and Z clear.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionHi
;; initialize result with false
		MOV		r0, #0
;; set C and Z flags
		MSR     CPSR_f, #( CPSR_Z | CPSR_C )
;; if unsigned higher return false
		MOVHI 	pc, lr
;; set C flag only
		MSR     CPSR_f, #CPSR_C
;; if unsigned higher return true
		MOVHI	r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionLs
;;
;; Conditional execution LS.
;;
;; @par Full Description
;; Tests if condition field LS (unsigned lower or same) works.
;; Unsigned lower or same is when C is clear or Z is set.
;;
;; @return	UINT32	Test result
;;  @retval 3	  	Success
;;  @retval < 3		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionLs
;; initialize result with 0
		MOV		r0, #0
;; C = 1, Z = 0
		MSR     CPSR_f, #CPSR_C
;; if unsigned lower or same return
		MOVLS 	pc, lr
;; C = 0, Z = 0
		MSR     CPSR_f, #CPSR_CLEAR
		ADDLS	r0, r0, #1
;; C = 0, Z = 1
		MSR     CPSR_f, #CPSR_Z
		ADDLS	r0, r0, #1
;; C = 1, Z = 1
		MSR     CPSR_f, #( CPSR_Z | CPSR_C )
		ADDLS	r0, r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionGe
;;
;; Conditional execution GE.
;;
;; @par Full Description
;; Tests if condition field GE (signed greater than or equal) works.
;; Signed greater than or equal  is when (N == V).
;;
;; @return	UINT32	Test result
;;  @retval 2	  	Success
;;  @retval < 2		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionGe
;; initialize result with 0
		MOV		r0, #0
;; N = 1, V = 0
		MSR     CPSR_f, #CPSR_N
;; if signed greater than or equal return
		MOVGE 	pc, lr
;; N = 0, V = 1
		MSR     CPSR_f, #CPSR_V
;; if signed greater than or equal return
		MOVGE 	pc, lr
;; N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_V )
		ADDGE	r0, r0, #1
;; N = 0, V = 0
		MSR     CPSR_f, #CPSR_CLEAR
		ADDGE	r0, r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionLt
;;
;; Conditional execution LT.
;;
;; @par Full Description
;; Tests if condition field LT (signed less than) works.
;; Signed less than is when (N != V).
;;
;; @return	UINT32	Test result
;;  @retval 2	  	Success
;;  @retval < 2		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionLt
;; initialize result with 0
		MOV		r0, #0
;; N = 0, V = 0
		MSR     CPSR_f, #CPSR_CLEAR
;; if signed less than return
		MOVLT 	pc, lr
;; N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_V )
;; if signed less than return
		MOVLT 	pc, lr
;; N = 0, V = 1
		MSR     CPSR_f, #CPSR_V
		ADDLT	r0, r0, #1
;; N = 1, V = 0
		MSR     CPSR_f, #CPSR_N
		ADDLT	r0, r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionGt
;;
;; Conditional execution GT.
;;
;; @par Full Description
;; Tests if condition field GT (signed greater than) works.
;; Signed greater than  is when (Z == 0, N == V).
;;
;; @return	UINT32	Test result
;;  @retval 2	  	Success
;;  @retval < 2		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionGt
;; initialize result with 0
		MOV		r0, #0
;; Z = 1, N = 0, V = 0
		MSR     CPSR_f, #CPSR_Z
;; if signed greater than return
		MOVGT 	pc, lr
;; Z = 1, N = 0, V = 1
		MSR     CPSR_f, #( CPSR_Z | CPSR_V )
;; if signed greater than return
		MOVGT 	pc, lr
;; Z = 1, N = 1, V = 0
		MSR     CPSR_f, #( CPSR_N | CPSR_Z )
;; if signed greater than return
		MOVGT 	pc, lr
;; Z = 1, N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_V )
;; if signed greater than return
		MOVGT 	pc, lr
;; Z = 0, N = 0, V = 0
		MSR     CPSR_f, #CPSR_CLEAR
		ADDGT	r0, r0, #1
;; Z = 0, N = 0, V = 1
		MSR     CPSR_f, #CPSR_V
		MOVGT 	pc, lr
;; Z = 0, N = 1, V = 0
		MSR     CPSR_f, #CPSR_N
		MOVGT 	pc, lr
;; Z = 0, N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_V )
		ADDGT	r0, r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrConditionLe
;;
;; Conditional execution LE.
;;
;; @par Full Description
;; Tests if condition field LE (signed less than or equal) works.
;; Signed less than or equal is when (Z == 1 or N != V).
;;
;; @return	UINT32	Test result
;;  @retval 6	  	Success
;;  @retval < 6		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrConditionLe
;; initialize result with 0
		MOV		r0, #0
;; Z = 1, N = 0, V = 0
		MSR     CPSR_f, #CPSR_Z
		ADDLE	r0, r0, #1
;; Z = 1, N = 0, V = 1
		MSR     CPSR_f, #( CPSR_Z | CPSR_V )
		ADDLE	r0, r0, #1
;; Z = 1, N = 1, V = 0
		MSR     CPSR_f, #( CPSR_N | CPSR_Z )
		ADDLE	r0, r0, #1
;; Z = 1, N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_V )
		ADDLE	r0, r0, #1
;; Z = 0, N = 0, V = 0
		MSR     CPSR_f, #CPSR_CLEAR
;; if signed less than or equal return
		MOVLE 	pc, lr
;; Z = 0, N = 1, V = 1
		MSR     CPSR_f, #( CPSR_N | CPSR_V )
;; if signed less than or equal return
		MOVLE 	pc, lr
;; Z = 0, N = 0, V = 1
		MSR     CPSR_f, #CPSR_V
		ADDLE	r0, r0, #1
;; Z = 0, N = 1, V = 0
		MSR     CPSR_f, #CPSR_N
		ADDLE	r0, r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmulbb
;;
;; SMULBB instruction. Signed Multiply (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y]
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If Y is B, then the bottom halfword of rm is used
;; If Y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	rn[15:0] * rm[15:0]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmulbb
		SMULBB	r0, r0, r1
;; Start Fault Injection Point 2
;;		LDR		r2, =InjectFaultFlag
;;		LDR		r2, [r2]
;;		CMP		r2, #1
;; Add 1 only if InjectFaultFlag is set
;;    	ADDEQ	r0, r0, #1
;; If above lines of code are uncommented then APEX ARM instructions and 
;; registers diagnostics will report fault at the next run of this stage of test
;; End Fault Injection Point 2
		MOV 	pc, lr


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmulbt
;;
;; SMULBT instruction. Signed Multiply (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y]
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If Y is B, then the bottom halfword of rm is used
;; If Y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	rn[15:0] * rm[31:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmulbt
		SMULBT	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmultb
;;
;; SMULTB instruction. Signed Multiply (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y]
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If Y is B, then the bottom halfword of rm is used
;; If Y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	rn[31:16] * rm[15:0]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmultb
		SMULTB	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmultt
;;
;; SMULTT instruction. Signed Multiply (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y]
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If Y is B, then the bottom halfword of rm is used
;; If Y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	rn[31:16] * rm[31:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmultt
		SMULTT	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlabb
;;
;; SMLABB instruction. Signed Multiply Accumulate (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y] + rs
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	rn[15:0] * rm[15:0] + rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlabb
		SMLABB	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlabt
;;
;; SMLABT instruction. Signed Multiply Accumulate (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y] + rs
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	rn[15:0] * rm[31:16] + rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlabt
		SMLABT	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlatb
;;
;; SMLATB instruction. Signed Multiply Accumulate (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y] + rs
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	rn[31:16] * rm[15:0] + rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlatb
		SMLATB	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlatt
;;
;; SMLATT instruction. Signed Multiply Accumulate (halfwords).
;;
;; @par Full Description
;; rd := rn[x] * rm[y] + rs
;; x and y specify which halfword
;; If x is B, then the bottom halfword of rn is used
;; If x is T, then the top halfword of rn is used
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	rn[31:16] * rm[31:16] + rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlatt
		SMLATT	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmulwb
;;
;; SMULWB instruction. Signed Multiply (word by halfword).
;;
;; @par Full Description
;; rd := (rm * rm[y])[47:16]
;; y specify which halfword
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;; Returns bits [47:16] of result
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	(rn * rm[15:0])[47:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmulwb
		SMULWB	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmulwt
;;
;; SMULWT instruction. Signed Multiply (word by halfword).
;;
;; @par Full Description
;; rd := (rm * rm[y])[47:16]
;; y specify which halfword
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;; Returns bits [47:16] of result
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;;
;; @return	INT32	(rn * rm[31:16])[47:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmulwt
		SMULWT	r0, r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlawb
;;
;; SMLAWB instruction. Signed Multiply Accumulate (word by halfword).
;;
;; @par Full Description
;; rd := (rn[x] * rm[y] + rs)[47:16]
;; y specify which halfword
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;; Returns bits [47:16] of result
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	(rn * rm[15:0] + rs)[47:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlawb
		SMLAWB	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrSmlawt
;;
;; SMLAWT instruction. Signed Multiply Accumulate (word by halfword).
;;
;; @par Full Description
;; rd := (rn[x] * rm[y] + rs)[47:16]
;; y specify which halfword
;; If y is B, then the bottom halfword of rm is used
;; If y is T, then the top halfword of rm is used
;; Returns bits [47:16] of result
;;
;; @param	INT32	rn	First argument.
;; @param	INT32	rm	Second argument.
;; @param	INT32	rs	Third argument.
;;
;; @return	INT32	(rn * rm[31:16] + rs)[47:16]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrSmlawt
		SMLAWT	r0, r0, r1, r2
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrLdr
;;
;; LDR instruction.
;;
;; @par Full Description
;; rd := [address]
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	[address]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrLdr
		LDR		r0, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrLdrb
;;
;; LDRB instruction.
;;
;; @par Full Description
;; rd := ZeroExtend[byte from address]
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	ZeroExtend[byte from address]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrLdrb
		LDRB	r0, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrLdrh
;;
;; LDRH instruction.
;;
;; @par Full Description
;; rd := ZeroExtent[halfword from address]
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	ZeroExtent[halfword from address]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrLdrh
		LDRH	r0, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrLdrsb
;;
;; LDRSB instruction.
;;
;; @par Full Description
;; rd := SignExtend[byte from address]
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	SignExtend[byte from address]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrLdrsb
		LDRSB	r0, [r0]
		MOV 	pc, lr	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrLdrsh
;;
;; LDRSH instruction.
;;
;; @par Full Description
;; rd := SignExtend[halfword from address]
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	SignExtend[halfword from address]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrLdrsh
		LDRSH	r0, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrStr
;;
;; STR instruction.
;;
;; @par Full Description
;; [address] := rd
;; Writes rd value to address.
;;
;; @param	UINT32*	address	First argument.
;; @param	UINT32	rd		Second argument.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrStr
		STR		r1, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrStrb
;;
;; STRB instruction.
;;
;; @par Full Description
;; [address][7:0] := rd[7:0]
;; Writes rd[7:0] value to address[7:0].
;;
;; @param	UINT32*	address	First argument.
;; @param	UINT32	rd		Second argument.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrStrb
		STRB	r1, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrStrh
;;
;; STRH instruction.
;;
;; @par Full Description
;; [address][15:0] := rd[15:0]
;; Writes rd[15:0] value to address[15:0].
;;
;; @param	UINT32*	address	First argument.
;; @param	UINT32	rd		Second argument.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrStrh
		STRH	r1, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrClz
;;
;; CLZ instruction. Count leading zeros.
;;
;; @par Full Description
;; rd := number of leading zeroes in rm
;;
;; @param	UINT32	rm	First argument.
;;
;; @return	UINT32	number of leading zeroes in rm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrClz
		CLZ		r0, r0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmInstrStmLdm
;;
;; STM and LDM instructions.
;;
;; @par Full Description
;; Pushes registers on stack with STMFD. Checks if sp was modified, and if correct
;; values were stored on stack. Restores registers from stack using LDMFD. Checks
;; if sp was modified and if correct values were restored from stack.
;;
;; @return	UINT32	Test result
;;  @retval 7	  	Success
;;  @retval < 7		Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmInstrStmfdLdmfd
;; test stmfd
;; prepare r0 for result
		MOV		r0, #0
;; remember sp in r1
		MOV 	r1, sp
;; store r4, r5 and lr on stack
		STMFD	sp !, { r4, r5, lr }
;; check if sp was modified
		SUB		r2,	r1, sp
		TEQ		r2, #12
		ADDEQ	r0, r0, #1
;; check if r4 was stored
		LDR		r2, [sp]
		TEQ		r2, r4
		ADDEQ	r0, r0, #1
;; check if r5 was stored
		LDR		r2, [sp, #4]
		TEQ		r2, r5
		ADDEQ	r0, r0, #1
;; check if lr was stored
		LDR		r2, [sp, #8]
		TEQ		r2, lr
		ADDEQ	r0, r0, #1
;; test ldmfd
;; copy r4, r5 to r2, r3
		MOV		r2, r4
		MOV		r3, r5
;; modify r4 and r5
		MVN		r4, r4
		MVN		r5, r5
;; load r4 and r5 from stack
		LDMFD   sp !, { r4, r5 }
;; check if r4 was loaded
		TEQ		r2, r4
		ADDEQ	r0, r0, #1
;; check if r5 was loaded
		TEQ		r3, r5
		ADDEQ	r0, r0, #1
;; check if sp was modified
		SUB		r2,	r1, sp
		TEQ		r2, #4
		ADDEQ	r0, r0, #1
;; restore lr and return
		LDMFD   sp !, { pc }

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2Immediate
;;
;; Operand2 immediate.
;;
;; @par Full Description
;; Uses Operand2 immediate version on MOV instruction.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2Immediate
		MOV		r0, #0
		MOV		r0, #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2LslImmediate
;;
;; Operand2 LSL immediate.
;;
;; @par Full Description
;; Uses Operand2 LSL immediate version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;;
;; @return	UINT32	rm << 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2LslImmediate
		MOV		r0, r0, LSL #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2LsrImmediate
;;
;; Operand2 LSR immediate.
;;
;; @par Full Description
;; Uses Operand2 LSR immediate version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;;
;; @return	UINT32	rm >> 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2LsrImmediate
		MOV		r0, r0, LSR #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2AsrImmediate
;;
;; Operand2 ASR immediate.
;;
;; @par Full Description
;; Uses Operand2 ASR immediate version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;;
;; @return	UINT32	(signed) rm / 2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2AsrImmediate
		MOV		r0, r0, ASR #1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2Register
;;
;; Operand2 Register immediate.
;;
;; @par Full Description
;; Uses Operand2 Register version on MOV instruction.
;; First and second parameters should differ.
;;
;;
;; @param	UINT32	dummy	First argument.
;; @param	UINT32	rm		Second argument.
;;
;; @return	UINT32	rm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2Register
		MOV		r0, r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2LslRegister
;;
;; Operand2 LSL Register.
;;
;; @par Full Description
;; Uses Operand2 LSL Register version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;; @param	UINT32	rs	Second argument.
;;
;; @return	UINT32	rm << rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2LslRegister
		MOV		r0, r0, LSL r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2LsrRegister
;;
;; Operand2 LSR Register.
;;
;; @par Full Description
;; Uses Operand2 LSR Register version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;; @param	UINT32	rs	Second argument.
;;
;; @return	UINT32	rm >> rs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2LsrRegister
		MOV		r0, r0, LSR r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2AsrRegister
;;
;; Operand2 ASR Register.
;;
;; @par Full Description
;; Uses Operand2 ASR Register version on MOV instruction.
;;
;; @param	UINT32	rm	First argument.
;; @param	UINT32	rs	Second argument.
;;
;; @return	UINT32	(signed) rm / pow( 2, rs )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2AsrRegister
		MOV		r0, r0, ASR r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2RorRegister
;;
;; Operand2 ROR Register.
;;
;; @par Full Description
;; Uses Operand2 ROR Register version on MOV instruction.
;; Bits moved out of the right-hand end of the register are rotated
;; back into the left-hand end
;;
;; @param	UINT32	rm	First argument.
;; @param	UINT32	rs	Second argument.
;;
;; @return	UINT32	result of ROR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2RorRegister
		MOV		r0, r0, ROR r1
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmOperand2Rxx
;;
;; Operand2 RXX.
;;
;; @par Full Description
;; Uses Operand2 RXX version on MOV instruction.
;; Rotates right 1 bit. If carry bit is set bit 31 is set to 1.
;;
;; @param	UINT32	rm	First argument.
;;
;; @return	UINT32	result of RRX on rm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmOperand2Rxx
;; set C flag
		MSR     CPSR_f, #CPSR_C
		MOV		r0, r0, RRX
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModeNoOffset
;;
;; Addressing mode no offset.
;;
;; @par Full Description
;; Loads value form address in r0 to r0.
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	*address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModeNoOffset
		LDR		r0, [r0]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModeImmediateOffsetPositive
;;
;; Addressing mode immediate positive.
;;
;; @par Full Description
;; Loads value form address in (r0 + 4 bytes) to r0.
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	*(address + 4 bytes)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModeImmediateOffsetPositive
		LDR		r0, [r0, #4]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModeImmediateOffsetNegative
;;
;; Addressing mode immediate negative.
;;
;; @par Full Description
;; Loads value form address in (r0 - 4 bytes) to r0.
;;
;; @param	UINT32*	address	First argument.
;;
;; @return	UINT32	*(address - 4 bytes)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModeImmediateOffsetNegative
		LDR		r0, [r0, #-4]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModeOffsetRegister
;;
;; Addressing mode immediate negative.
;;
;; @par Full Description
;; Loads value form address in (r0 + r1) to r0.
;;
;; @param	UINT32*	address	First argument.
;; @param	UINT32	offset	Second argument.
;;
;; @return	UINT32	*(address + offset)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModeOffsetRegister
		LDR		r0, [r0, r1]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModeOffsetRegisterShift
;;
;; Addressing mode immediate negative.
;;
;; @par Full Description
;; Loads value form address in (r0 + (r1 << 2)) to r0.
;;
;; @param	UINT32*	address	First argument.
;; @param	UINT32	offset	Second argument.
;;
;; @return	UINT32	*(address + (offset << 2))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModeOffsetRegisterShift
		LDR		r0, [r0, r1, LSL #2]
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePreInxedImmediateOffset
;;
;; Addressing mode pre indexed immediate.
;;
;; @par Full Description
;; Uses pre indexing immediate addressing mode. Address shall be calulated first,
;; then used, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address			First argument.
;; @param	UINT32	expectedValue	Second argument.
;;
;; @return	UINT32*					Modified address
;;  @retval address + 4 bytes  		Success
;;  @retval 0						Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePreInxedImmediateOffset
		LDR		r2, [r0, #4]!
;; test if loaded value is as expected
		TEQ		r1, r2
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePreInxedOffsetRegister
;;
;; Addressing mode pre indexed register.
;;
;; @par Full Description
;; Uses pre indexing register addressing mode. Address shall be calulated first,
;; then used, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address			First argument.
;; @param	UINT32	offset			Second argument.
;; @param	UINT32	expectedValue	Third argument.
;;
;; @return	UINT32*					Modified address
;;  @retval	address + offset bytes  Success
;;  @retval 0						Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePreInxedOffsetRegister
		LDR		r3, [r0, r1]!
;; test if loaded value is as expected
		TEQ		r2, r3
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePreInxedOffsetRegisterShift
;;
;; Addressing mode pre indexed register shift.
;;
;; @par Full Description
;; Uses pre indexing register shift addressing mode.
;; Address shall be calulated first, then used, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address				First argument.
;; @param	UINT32	offset				Second argument.
;; @param	UINT32	expectedValue		Third argument.
;;
;; @return	UINT32*						Modified address
;;  @retval address + offset * 4 bytes 	Success
;;  @retval 0							Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePreInxedOffsetRegisterShift
		LDR		r3, [r0, r1, LSL #2]!
;; test if loaded value is as expected
		TEQ		r2, r3
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePostInxedImmediateOffset
;;
;; Addressing mode post indexed immediate.
;;
;; @par Full Description
;; Uses post indexing immediate addressing mode. Address shall be used first,
;; then calculated, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address			First argument.
;; @param	UINT32	expectedValue	Second argument.
;;
;; @return	UINT32*					Modified address
;;  @retval address + 4 bytes  		Success
;;  @retval 0						Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePostInxedImmediateOffset
		LDR		r2, [r0], #4
;; test if loaded value is as expected
		TEQ		r1, r2
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePostInxedOffsetRegister
;;
;; Addressing mode post indexed register.
;;
;; @par Full Description
;; Uses post indexing register addressing mode. Address shall be used first,
;; then calculated, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address			First argument.
;; @param	UINT32	offset			Second argument.
;; @param	UINT32	expectedValue	Third argument.
;;
;; @return	UINT32*					Modified address
;;  @retval address + offset bytes  Success
;;  @retval 0						Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePostInxedOffsetRegister
		LDR		r3, [r0], r1
;; test if loaded value is as expected
		TEQ		r2, r3
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmAddrModePostInxedOffsetRegisterShift
;;
;; Addressing mode post indexed register shift.
;;
;; @par Full Description
;; Uses post indexing register shift addressing mode.
;; Address shall be used first, then calculated, then write back to its register.
;; Checks if loaded value is as expected. Returns modified address.
;;
;; @param	UINT32*	address				First argument.
;; @param	UINT32	offset				Second argument.
;; @param	UINT32	expectedValue		Third argument.
;;
;; @return	UINT32*						Modified address
;;  @retval address + offset * 4 bytes 	Success
;;  @retval 0							Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmAddrModePostInxedOffsetRegisterShift
		LDR		r3, [r0], r1, LSL #2
;; test if loaded value is as expected
		TEQ		r2, r3
;; return 0 if different than expected
		MOVNE	r0, #0
		MOV 	pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR0
;;
;; Register r0 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r0. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR0
;; move pattern 1 from r0 to r2
		MOV		r2, r0
;; set pattern 1
		MOV		r0, r1
		TEQ		r0, r1
;; Start Fault Injection Point 4
;;    	BEQ		ArmRegisterR0ReturnFalse
;; If above line of code is uncommented then APEX ARM instructions and 
;; registers diagnostics will report fault at the next run of this stage of test
;; End Fault Injection Point 4
		BNE		ArmRegisterR0ReturnFalse
;; set pattern 2
		MOV		r0, r2
		TEQ		r0, r2
		BNE		ArmRegisterR0ReturnFalse
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR0ReturnFalse
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR1
;;
;; Register r1 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r1. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR1
;; move pattern 2 from r1 to r2
		MOV		r2, r1
;; set pattern 1
		MOV		r1, r0
;; Start Fault Injection Point 5
;;		LDR		r3, =InjectFaultFlag
;;		LDR		r3, [r3]
;;		CMP		r3, #1
;; Jump to return false only if InjectFaultFlag is set
;;		BEQ		ArmRegisterR1ReturnFalse
;; If above lines of code are uncommented then APEX ARM instructions and 
;; registers diagnostics will report fault at the next run of this stage of test
;; End Fault Injection Point 5
		TEQ		r1, r0
		BNE		ArmRegisterR1ReturnFalse
;; set pattern 2
		MOV		r1, r2
		TEQ		r1, r2
		BNE		ArmRegisterR1ReturnFalse
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR1ReturnFalse
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR2
;;
;; Register r2 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r2. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR2
;; set pattern 1
		MOV		r2, r0
		TEQ		r2, r0
		BNE		ArmRegisterR2ReturnFalse
;; set pattern 2
		MOV		r2, r1
		TEQ		r2, r1
		BNE		ArmRegisterR2ReturnFalse
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR2ReturnFalse
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR3
;;
;; Register r3 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r3. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR3
;; set pattern 1
		MOV		r3, r0
		TEQ		r3, r0
		BNE		ArmRegisterR3ReturnFalse
;; set pattern 2
		MOV		r3, r1
		TEQ		r3, r1
		BNE		ArmRegisterR3ReturnFalse
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR3ReturnFalse
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR4
;;
;; Register r4 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r4. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR4
;; store r4 in r2
		MOV		r2, r4
;; set pattern 1
		MOV		r4, r0
		TEQ		r4, r0
		BNE		ArmRegisterR4ReturnFalse
;; set pattern 2
		MOV		r4, r1
		TEQ		r4, r1
		BNE		ArmRegisterR4ReturnFalse
;; restore r4
		MOV		r4, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR4ReturnFalse
;; restore r4
		MOV		r4, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR5
;;
;; Register r5 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r5. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR5
;; store r5 in r2
		MOV		r2, r5
;; set pattern 1
		MOV		r5, r0
		TEQ		r5, r0
		BNE		ArmRegisterR5ReturnFalse
;; set pattern 2
		MOV		r5, r1
		TEQ		r5, r1
		BNE		ArmRegisterR5ReturnFalse
;; restore r5
		MOV		r5, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR5ReturnFalse
;; restore r5
		MOV		r5, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR6
;;
;; Register r6 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r6. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR6
;; store r6 in r2
		MOV		r2, r6
;; set pattern 1
		MOV		r6, r0
		TEQ		r6, r0
		BNE		ArmRegisterR6ReturnFalse
;; set pattern 2
		MOV		r6, r1
		TEQ		r6, r1
		BNE		ArmRegisterR6ReturnFalse
;; restore r6
		MOV		r6, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR6ReturnFalse
;; restore r6
		MOV		r6, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR7
;;
;; Register r7 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r7. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR7
;; store r7 in r2
		MOV		r2, r7
;; set pattern 1
		MOV		r7, r0
		TEQ		r7, r0
		BNE		ArmRegisterR7ReturnFalse
;; set pattern 2
		MOV		r7, r1
		TEQ		r7, r1
		BNE		ArmRegisterR7ReturnFalse
;; restore r7
		MOV		r7, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR7ReturnFalse
;; restore r7
		MOV		r7, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR8
;;
;; Register r8 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r8. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR8
;; store r8 in r2
		MOV		r2, r8
;; set pattern 1
		MOV		r8, r0
		TEQ		r8, r0
		BNE		ArmRegisterR8ReturnFalse
;; set pattern 2
		MOV		r8, r1
		TEQ		r8, r1
		BNE		ArmRegisterR8ReturnFalse
;; restore r8
		MOV		r8, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR8ReturnFalse
;; restore r8
		MOV		r8, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR9
;;
;; Register r9 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r9. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR9
;; store r9 in r2
		MOV		r2, r9
;; set pattern 1
		MOV		r9, r0
		TEQ		r9, r0
		BNE		ArmRegisterR9ReturnFalse
;; set pattern 2
		MOV		r9, r1
		TEQ		r9, r1
		BNE		ArmRegisterR9ReturnFalse
;; restore r9
		MOV		r9, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR9ReturnFalse
;; restore r9
		MOV		r9, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR10
;;
;; Register r10 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r10. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR10
;; store r10 in r2
		MOV		r2, r10
;; set pattern 1
		MOV		r10, r0
		TEQ		r10, r0
		BNE		ArmRegisterR10ReturnFalse
;; set pattern 2
		MOV		r10, r1
		TEQ		r10, r1
		BNE		ArmRegisterR10ReturnFalse
;; restore r10
		MOV		r10, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR10ReturnFalse
;; restore r10
		MOV		r10, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR11
;;
;; Register r11 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r11. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR11
;; store r11 in r2
		MOV		r2, r11
;; set pattern 1
		MOV		r11, r0
		TEQ		r11, r0
		BNE		ArmRegisterR11ReturnFalse
;; set pattern 2
		MOV		r11, r1
		TEQ		r11, r1
		BNE		ArmRegisterR11ReturnFalse
;; restore r11
		MOV		r11, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR11ReturnFalse
;; restore r11
		MOV		r11, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR12
;;
;; Register r12 test.
;;
;; @par Full Description
;; Sets 2 patterns in register r12. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR12
;; store r12 in r2
		MOV		r2, r12
;; set pattern 1
		MOV		r12, r0
		TEQ		r12, r0
		BNE		ArmRegisterR12ReturnFalse
;; set pattern 2
		MOV		r12, r1
		TEQ		r12, r1
		BNE		ArmRegisterR12ReturnFalse
;; restore r12
		MOV		r12, r2
;; return true
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR12ReturnFalse
;; restore r12
		MOV		r12, r2
;; return false
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpSystem
;;
;; Register sp in system mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in system mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpSystem
;; remember current mode in r2
		MRS		r2, CPSR
;; goto system mode
		MSR     CPSR_c, #MODE_SYSTEM
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpSystemReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpSystemReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpSystemReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrSystem
;;
;; Register lr in system mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in system mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrSystem
;; remember current mode in r2
		MRS		r2, CPSR
;; goto system mode
		MSR     CPSR_c, #MODE_SYSTEM
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrSystemReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrSystemReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrSystemReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpSupervisor
;;
;; Register sp in supervisor mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in supervisor mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpSupervisor
;; remember current mode in r2
		MRS		r2, CPSR
;; goto supervisor mode
		MSR     CPSR_c, #MODE_SUPERVISOR
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpSupervisorReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpSupervisorReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpSupervisorReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrSupervisor
;;
;; Register lr in supervisor mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in supervisor mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrSupervisor
;; remember current mode in r2
		MRS		r2, CPSR
;; goto supervisor mode
		MSR     CPSR_c, #MODE_SUPERVISOR
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrSupervisorReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrSupervisorReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrSupervisorReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpAbort
;;
;; Register sp in abort mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in abort mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpAbort
;; remember current mode in r0
		MRS		r2, CPSR
;; goto abort mode
		MSR     CPSR_c, #MODE_ABORT
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpAbortReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpAbortReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpAbortReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrAbort
;;
;; Register lr in abort mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in abort mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrAbort
;; remember current mode in r2
		MRS		r2, CPSR
;; goto abort mode
		MSR     CPSR_c, #MODE_ABORT
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrAbortReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrAbortReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrAbortReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpUndefined
;;
;; Register sp in undefined mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in undefined mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpUndefined
;; remember current mode in r2
		MRS		r2, CPSR
;; goto undefined mode
		MSR     CPSR_c, #MODE_UNDEFINED
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpUndefinedReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpUndefinedReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpUndefinedReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrUndefined
;;
;; Register lr in undefined test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in undefined mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrUndefined
;; remember current mode in r2
		MRS		r2, CPSR
;; goto undefined mode
		MSR     CPSR_c, #MODE_UNDEFINED
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrUndefinedReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrUndefinedReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrUndefinedReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpIrq
;;
;; Register sp in irq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in irq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpIrq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto irq mode
		MSR     CPSR_c, #MODE_IRQ
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpIrqReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpIrqReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpIrqReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrIrq
;;
;; Register lr in irq undefined test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in irq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrIrq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto irq mode
		MSR     CPSR_c, #MODE_IRQ
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrIrqReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrIrqReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrIrqReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterSpFiq
;;
;; Register sp in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register sp in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterSpFiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store sp in r3
		MOV		r3, sp
;; set pattern 1
		MOV		sp, r0
		TEQ		sp, r0
		BNE		ArmRegisterSpFiqReturnFalse
;; set pattern 2
		MOV		sp, r1
		TEQ		sp, r1
		BNE		ArmRegisterSpFiqReturnFalse
;; restore remembered sp and mode, return true
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterSpFiqReturnFalse
;; restore remembered sp and mode, return false
		MOV		sp, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterLrFiq
;;
;; Register lr in fiq undefined test.
;;
;; @par Full Description
;; Sets 2 patterns in register lr in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterLrFiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store lr in r3
		MOV		r3, lr
;; set pattern 1
		MOV		lr, r0
		TEQ		lr, r0
		BNE		ArmRegisterLrFiqReturnFalse
;; set pattern 2
		MOV		lr, r1
		TEQ		lr, r1
		BNE		ArmRegisterLrFiqReturnFalse
;; restore remembered lr and mode, return true
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterLrFiqReturnFalse
;; restore remembered lr and mode, return false
		MOV		lr, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR8Fiq
;;
;; Register r8 in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register r8 in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR8Fiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store r8 in r3
		MOV		r3, r8
;; set pattern 1
		MOV		r8, r0
		TEQ		r8, r0
		BNE		ArmRegisterR8FiqReturnFalse
;; set pattern 2
		MOV		r8, r1
		TEQ		r8, r1
		BNE		ArmRegisterR8FiqReturnFalse
;; restore remembered r8 and mode, return true
		MOV		r8, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR8FiqReturnFalse
;; restore remembered r8 and mode, return false
		MOV		r8, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR9Fiq
;;
;; Register r9 in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register r9 in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR9Fiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store r9 in r3
		MOV		r3, r9
;; set pattern 1
		MOV		r9, r0
		TEQ		r9, r0
		BNE		ArmRegisterR9FiqReturnFalse
;; set pattern 2
		MOV		r9, r1
		TEQ		r9, r1
		BNE		ArmRegisterR9FiqReturnFalse
;; restore remembered r9 and mode, return true
		MOV		r9, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR9FiqReturnFalse
;; restore remembered r9 and mode, return false
		MOV		r9, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR10Fiq
;;
;; Register r10 in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register r10 in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR10Fiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store r10 in r3
		MOV		r3, r10
;; set pattern 1
		MOV		r10, r0
		TEQ		r10, r0
		BNE		ArmRegisterR10FiqReturnFalse
;; set pattern 2
		MOV		r10, r1
		TEQ		r10, r1
		BNE		ArmRegisterR10FiqReturnFalse
;; restore remembered r10 and mode, return true
		MOV		r10, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR10FiqReturnFalse
;; restore remembered r10 and mode, return false
		MOV		r10, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR11Fiq
;;
;; Register r11 in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register r11 in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR11Fiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store r11 in r3
		MOV		r3, r11
;; set pattern 1
		MOV		r11, r0
		TEQ		r11, r0
		BNE		ArmRegisterR11FiqReturnFalse
;; set pattern 2
		MOV		r11, r1
		TEQ		r11, r1
		BNE		ArmRegisterR11FiqReturnFalse
;; restore remembered r11 and mode, return true
		MOV		r11, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR11FiqReturnFalse
;; restore remembered r11 and mode, return false
		MOV		r11, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterR12Fiq
;;
;; Register r12 in fiq mode test.
;;
;; @par Full Description
;; Sets 2 patterns in register r12 in fiq mode. Checks if patterns were set.
;;
;; @param	UINT32	pattern1	First pattern to set.
;; @param	UINT32	pattern2	Second pattern to set.
;;
;; @return	bool	Test result
;;  @retval true  	Success
;;  @retval false	Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterR12Fiq
;; remember current mode in r2
		MRS		r2, CPSR
;; goto fiq mode
		MSR     CPSR_c, #MODE_FIQ
;; store r12 in r3
		MOV		r3, r12
;; set pattern 1
		MOV		r12, r0
		TEQ		r12, r0
		BNE		ArmRegisterR12FiqReturnFalse
;; set pattern 2
		MOV		r12, r1
		TEQ		r12, r1
		BNE		ArmRegisterR12FiqReturnFalse
;; restore remembered r12 and mode, return true
		MOV		r12, r3
		MSR     CPSR_c, r2
		MOV		r0, #1
		MOV		pc, lr
ArmRegisterR12FiqReturnFalse
;; restore remembered r12 and mode, return false
		MOV		r12, r3
		MSR     CPSR_c, r2
		MOV		r0, #0
		MOV		pc, lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ArmRegisterCpsr
;;
;; Register CPSR test.
;;
;; @par Full Description
;; Sets 2 patterns of flags in CPSR register. Tests if patterns were set.
;;
;; @return	UINT32				Test result
;;  @retval 2	  				Success
;;  @retval < 2					Failure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArmRegisterCpsr
;; prepare r0 as result
		MOV		r0, #0
;; store CPSR in r3
		MRS		r3, CPSR
;; use r1 as reference value
		MOV		r1, r3
;; set all flags in CPSR
		MSR     CPSR_f, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		MRS		r2, CPSR
;; preapre value to compare
		ORR		r1, r1, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		TEQ		r2, r1
		ADDEQ	r0, r0, #1
;; clear all flags in CPSR
		MSR     CPSR_f, #CPSR_CLEAR
		MRS		r2, CPSR
;; preapre value to compare
		BIC		r1, r1, #( CPSR_N | CPSR_Z | CPSR_C | CPSR_V )
		TEQ		r2, r1
		ADDEQ	r0, r0, #1
;; restore CPSR and return
		MSR     CPSR_f, r3
		MOV		pc, lr
		
	; END directive
	END
