////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagmCrcLoopback.hpp
///
/// @details Apex binary CRC diagnostic class diagnostic definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - dtstalte  04-DEC-2013  Created.
/// - dtstalte  12-DEC-2013  Moved in mCRC specific functions from global files.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGMCRCLOOPBACK_HPP
#define APEXDIAGMCRCLOOPBACK_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagnostic.hpp"           // for DiagControlBlock
#include "ApexStdType.hpp"              // for UINT types
#include "ApexParameters.hpp"           // for HI_ApexParam

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagmCrcLoopback
///
/// @ingroup Diagnostics
///
/// @brief This class implements the APEX mCRC loopback diagnostic including
/// initialization, power-up, and runtime components.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagmCrcLoopback
{
public:
    
    enum mCrcLoopbackState
    {
        MCRC_INIT = 1,
        MCRC_NOT_READY,
        MCRC_WAITING_FOR_RX_AVAILABLE,
        MCRC_WAITING_FOR_MCRC_ERROR,
        MCRC_SUCCESS,
        MCRC_FAIL
    };
    
    // PUBLIC METHODS
    
    /// Initialize the diagnostic.  Called at power up.
    static void Init();
    
    /// The primary body of the run time diagnostic test.
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);
    
    static void ParallelCrc32(UINT32* pData, UINT16 size, UINT32* pCrc);
    static void ParallelCrc8(UINT8* pData, UINT16 size, UINT32* pCrc);
    static UINT16 ComputeParallelCrc(void* pSrc, UINT16 size);
    static void ContinueParallelCrc(void* pSrc, UINT16 size, UINT32* pCrc);
    static void VerifyParallelCrc(void* pSrc, UINT32 size, UINT32 HostCrc);

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS
    inline static bool IsmCRCErrorReported();
    inline static bool IsmCRCErrorSeqMatch();
    inline static bool IsResponseAvailable();
    inline static bool IsRxAvailReported();
    inline static void SetTxReqstReady();
    
    static void TestNotSetUp(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr);
    static void PreparePacket(UINT32 LpBkSize, void* LpBkTxAddr);
    static void PreparePattern(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr);
    static void TestNotWaiting(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr);
    static void CheckResponse(UINT32 LpBkSize, void* LpBkRxAddr);
    static void TestResponseAvailable(UINT32 LpBkSize, void* LpBkRxAddr);
    static void CheckState(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr);
    static ApexDiagnostic::TestState HandleState();
    
    /// Default constructor.
    ApexDiagmCrcLoopback();
    
    /// Default destructor.
    ~ApexDiagmCrcLoopback();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagmCrcLoopback(const ApexDiagmCrcLoopback &);
    ApexDiagmCrcLoopback &operator=(const ApexDiagmCrcLoopback &);

    // MEMBER VARIABLES
    
    // Used to record start time for the test iteration
    static UINT32   m_IterationStartTime;
    
    // Sequence number for the loopback packet data payload
    static UINT16   m_LoopbackSequeneNumber;
    
    // Flag for selecting a WAITING state
    static bool     m_WaitingStateSelect;
    
    // mCRC for TxBuffer
    static UINT32   m_TxmCrc;
    
    // Intermediate mCRC representing the fixed test pattern
    static UINT32   m_TestPatternmCrc;

    // Flag for tracking the state of the mCRC loopback test
    static mCrcLoopbackState m_mCrcLoopbackState;
        
    static const UINT32 MCRC_LOOPBACK_SHORT_STEP_MS = 1000;
    
    // Corresponds to Max Unconnected Frame Size for CNet
    static const UINT32 MAX_LPBK_BUF_SIZE   = 512;

    // These constants represent the bits positions for handshaking flags
    // in the mCRCLpBkCtrlSts ApexParameter
    static const UINT32 LPBK_RX_AVAILABLE         = 0x01;
    static const UINT32 LPBK_MCRC_ERROR       = 0x02;
    static const UINT32 LPBK_TX_REQUEST_READY    = 0x08;

    // Shift Count for Sequence Number passed back
    // by CNet/ENet port in mCRCLpBkCtrlSts
    static const UINT32 LPBK_SEQUENCE_NO_SHIFT      = 16;

    // Constant for loading byte & word test patterns
    static const UINT32 LPBK_PATTERN_OF_32_BIT_ZEROES_ONES = 0x55555555;
    
    // Timeouts -- account for following possibilities:
    // i.  Power-up delay to start these tests from the host side
    // ii. In case of mCRC, delay for the CNet/ENet CommPort to loopback the response
    // iii. In case of BP, delay for the Apex to loopback the response vis-a-vis backplane traffic load
    // 5 mins should be long enough for all of these cases! 
    // If a timeout occurs, Apex will assert the fault line to the host.
    static const UINT32 MCRC_LOOPBACK_RESPONSE_TIMEOUT_US
    
#if defined(SUPPORT_FAULT_INJECTION)
    // Use a longer timeout to allow time for fault injection
    // procedure to complete before the timeout occurs.
    = 600000000;    // 10 mins
#else
    = 300000000;    // 5 min    
#endif
};

// INLINE FUNCTIONS

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::IsmCRCErrorReported()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks if the CNet/ENet CommPort has reported a mCRC
/// validation failure for a transmit request.
/////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagmCrcLoopback::IsmCRCErrorReported()
{
    return ((HI_ApexParam.mCRC.LpBkCtrlSts & LPBK_MCRC_ERROR) != 0);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::IsmCRCErrorSeqMatch()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks if the CNet/ENet CommPort has reported the
/// correct sequence number for an accompanying mCRC validation failure report.
/////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagmCrcLoopback::IsmCRCErrorSeqMatch()
{
    return ((HI_ApexParam.mCRC.LpBkCtrlSts >> LPBK_SEQUENCE_NO_SHIFT) == m_LoopbackSequeneNumber);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::IsResponseAvailable()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks if the CNet/ENet CommPort has delivered a
/// response to us.
/////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagmCrcLoopback::IsResponseAvailable()
{
    // mCRCLpBkCtrlSts is like a mail-box. ENet/CNet CommPort will
    // write to all 32-bits, but typically, with only the
    // RxAvail or mCRCError bit set. But we will check for any bits
    // set other than TxReqst
    return ((HI_ApexParam.mCRC.LpBkCtrlSts & ~LPBK_TX_REQUEST_READY) != 0);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::IsRxAvailReported()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks if the CNet/ENet CommPort has reported that
/// the looped back packet is available.
/////////////////////////////////////////////////////////////////////////////
inline bool ApexDiagmCrcLoopback::IsRxAvailReported()
{
    return ((HI_ApexParam.mCRC.LpBkCtrlSts & LPBK_RX_AVAILABLE) != 0);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::SetTxReqstReady()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function wakes up the CNet/ENet CommPort to transmit the
/// loopback packet.
/////////////////////////////////////////////////////////////////////////////
inline void ApexDiagmCrcLoopback::SetTxReqstReady()
{
    // mCRCLpBkCtrlSts is like a mail-box. Write to all 32-bits
    // but in this case, with only the TxReqst bit set
    HI_ApexParam.mCRC.LpBkCtrlSts = LPBK_TX_REQUEST_READY;
}

#endif // #if !defined(APEXDIAGMCRCLOOPBACK_HPP)
