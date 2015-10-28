////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagBinaryCrc.cpp
///
/// @details Apex binary CRC diagnostic class diagnostic implementation.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke      27-NOV-2013 Created.
/// -   pszramo     11-DEC-2013 Updates to fault injection points according to
///                             Coverity defects triage.
/// -   spolke      16-DEC-2013 Assertion on too short time for completion tweaked.
///                             RunTest return type corrected.
/// -   dtstalte    28-FEB-2014 Add new fault injection point for completion
///                             time checks since location of an assert moved.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "ApexDiagBinaryCrc.hpp"            // ApexDiagBinaryCrc class declaration

// FORWARD REFERENCES
// (none)

UINT32 ApexDiagBinaryCrc::m_CodeSize;
UINT32 ApexDiagBinaryCrc::m_CrcExpected;
UINT32 ApexDiagBinaryCrc::m_BytesPerStep;
UINT32 ApexDiagBinaryCrc::m_Crc;
UINT32 ApexDiagBinaryCrc::m_BytesLeft;
const UINT8 *ApexDiagBinaryCrc::m_pCurrentPtr;

//******************************************************************************
// PUBLIC METHODS
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
//  METHOD NAME: ApexDiagBinaryCrc::CrcDataReset
//
/// @memberof ApexDiagBinaryCrc
///
/// @details Resets CRC data to initial values.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagBinaryCrc::CrcDataReset(void)
{
    m_Crc          = 0xffffffff;
    m_BytesLeft    = m_CodeSize;
    m_pCurrentPtr   = (UINT8*)(DGN_CODE_SPACE_START);
}

////////////////////////////////////////////////////////////////////////////////
//  METHOD NAME: ApexDiagBinaryCrc::Init
//
/// @memberof ApexDiagBinaryCrc
///
/// @details Initializes CRC data diagnostic.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagBinaryCrc::Init(void)
{
    // Start Fault Injection Point 3
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 3

    m_CodeSize = *((UINT32 *)DGN_CODE_SIZE_LOC);

    if ((m_CodeSize == 0)
          || (m_CodeSize > (DGN_CODE_SPACE_END - DGN_CODE_SPACE_START)))
    {
        firmExcept();
    }

    UINT8* crcLocation = (UINT8*)(DGN_CODE_SPACE_START + m_CodeSize);

    UINT32 crcExpected = 0;
    crcExpected |= crcLocation[0];
    crcExpected |= crcLocation[1] << 8;
    crcExpected |= (crcLocation[2] & 0xFFFF) << 16;
    crcExpected |= (crcLocation[3] & 0xFF) << 24;

    m_CrcExpected = crcExpected;

    CrcDataReset();
}

////////////////////////////////////////////////////////////////////////////////
// ApexDiagBinaryCrc::CalculateCrc32
//
/// Function that calculates a 32-bit CRC (Sum2Bin implementation based).
///
/// @par Full Description
/// This function calculates the CRC value using the generator polynomial:
/// x^32+x^26+x^23+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
/// @n (0x104c11db7)
///
/// @param pData     Pointer to the data to be CRCed.
/// @param byteLen   Length of data in bytes to be CRCed.
/// @param crc       CRC seed value.
/// @return Returns a 32-bit CRC value.
///
/// @safety
///
////////////////////////////////////////////////////////////////////////////////
UINT32 ApexDiagBinaryCrc::CalculateCrc32(const UINT8 *&pData, const UINT32 byteLen, UINT32 crc)
{
    // CRC table, used in the CRC calculation.
    static const UINT32 table[256] =
    {
        0x00000000U,0x77073096U,0xEE0E612CU,0x990951BAU,
        0x076DC419U,0x706AF48FU,0xE963A535U,0x9E6495A3U,
        0x0EDB8832U,0x79DCB8A4U,0xE0D5E91EU,0x97D2D988U,
        0x09B64C2BU,0x7EB17CBDU,0xE7B82D07U,0x90BF1D91U,
        0x1DB71064U,0x6AB020F2U,0xF3B97148U,0x84BE41DEU,
        0x1ADAD47DU,0x6DDDE4EBU,0xF4D4B551U,0x83D385C7U,
        0x136C9856U,0x646BA8C0U,0xFD62F97AU,0x8A65C9ECU,
        0x14015C4FU,0x63066CD9U,0xFA0F3D63U,0x8D080DF5U,
        0x3B6E20C8U,0x4C69105EU,0xD56041E4U,0xA2677172U,
        0x3C03E4D1U,0x4B04D447U,0xD20D85FDU,0xA50AB56BU,
        0x35B5A8FAU,0x42B2986CU,0xDBBBC9D6U,0xACBCF940U,
        0x32D86CE3U,0x45DF5C75U,0xDCD60DCFU,0xABD13D59U,
        0x26D930ACU,0x51DE003AU,0xC8D75180U,0xBFD06116U,
        0x21B4F4B5U,0x56B3C423U,0xCFBA9599U,0xB8BDA50FU,
        0x2802B89EU,0x5F058808U,0xC60CD9B2U,0xB10BE924U,
        0x2F6F7C87U,0x58684C11U,0xC1611DABU,0xB6662D3DU,
        0x76DC4190U,0x01DB7106U,0x98D220BCU,0xEFD5102AU,
        0x71B18589U,0x06B6B51FU,0x9FBFE4A5U,0xE8B8D433U,
        0x7807C9A2U,0x0F00F934U,0x9609A88EU,0xE10E9818U,
        0x7F6A0DBBU,0x086D3D2DU,0x91646C97U,0xE6635C01U,
        0x6B6B51F4U,0x1C6C6162U,0x856530D8U,0xF262004EU,
        0x6C0695EDU,0x1B01A57BU,0x8208F4C1U,0xF50FC457U,
        0x65B0D9C6U,0x12B7E950U,0x8BBEB8EAU,0xFCB9887CU,
        0x62DD1DDFU,0x15DA2D49U,0x8CD37CF3U,0xFBD44C65U,
        0x4DB26158U,0x3AB551CEU,0xA3BC0074U,0xD4BB30E2U,
        0x4ADFA541U,0x3DD895D7U,0xA4D1C46DU,0xD3D6F4FBU,
        0x4369E96AU,0x346ED9FCU,0xAD678846U,0xDA60B8D0U,
        0x44042D73U,0x33031DE5U,0xAA0A4C5FU,0xDD0D7CC9U,
        0x5005713CU,0x270241AAU,0xBE0B1010U,0xC90C2086U,
        0x5768B525U,0x206F85B3U,0xB966D409U,0xCE61E49FU,
        0x5EDEF90EU,0x29D9C998U,0xB0D09822U,0xC7D7A8B4U,
        0x59B33D17U,0x2EB40D81U,0xB7BD5C3BU,0xC0BA6CADU,
        0xEDB88320U,0x9ABFB3B6U,0x03B6E20CU,0x74B1D29AU,
        0xEAD54739U,0x9DD277AFU,0x04DB2615U,0x73DC1683U,
        0xE3630B12U,0x94643B84U,0x0D6D6A3EU,0x7A6A5AA8U,
        0xE40ECF0BU,0x9309FF9DU,0x0A00AE27U,0x7D079EB1U,
        0xF00F9344U,0x8708A3D2U,0x1E01F268U,0x6906C2FEU,
        0xF762575DU,0x806567CBU,0x196C3671U,0x6E6B06E7U,
        0xFED41B76U,0x89D32BE0U,0x10DA7A5AU,0x67DD4ACCU,
        0xF9B9DF6FU,0x8EBEEFF9U,0x17B7BE43U,0x60B08ED5U,
        0xD6D6A3E8U,0xA1D1937EU,0x38D8C2C4U,0x4FDFF252U,
        0xD1BB67F1U,0xA6BC5767U,0x3FB506DDU,0x48B2364BU,
        0xD80D2BDAU,0xAF0A1B4CU,0x36034AF6U,0x41047A60U,
        0xDF60EFC3U,0xA867DF55U,0x316E8EEFU,0x4669BE79U,
        0xCB61B38CU,0xBC66831AU,0x256FD2A0U,0x5268E236U,
        0xCC0C7795U,0xBB0B4703U,0x220216B9U,0x5505262FU,
        0xC5BA3BBEU,0xB2BD0B28U,0x2BB45A92U,0x5CB36A04U,
        0xC2D7FFA7U,0xB5D0CF31U,0x2CD99E8BU,0x5BDEAE1DU,
        0x9B64C2B0U,0xEC63F226U,0x756AA39CU,0x026D930AU,
        0x9C0906A9U,0xEB0E363FU,0x72076785U,0x05005713U,
        0x95BF4A82U,0xE2B87A14U,0x7BB12BAEU,0x0CB61B38U,
        0x92D28E9BU,0xE5D5BE0DU,0x7CDCEFB7U,0x0BDBDF21U,
        0x86D3D2D4U,0xF1D4E242U,0x68DDB3F8U,0x1FDA836EU,
        0x81BE16CDU,0xF6B9265BU,0x6FB077E1U,0x18B74777U,
        0x88085AE6U,0xFF0F6A70U,0x66063BCAU,0x11010B5CU,
        0x8F659EFFU,0xF862AE69U,0x616BFFD3U,0x166CCF45U,
        0xA00AE278U,0xD70DD2EEU,0x4E048354U,0x3903B3C2U,
        0xA7672661U,0xD06016F7U,0x4969474DU,0x3E6E77DBU,
        0xAED16A4AU,0xD9D65ADCU,0x40DF0B66U,0x37D83BF0U,
        0xA9BCAE53U,0xDEBB9EC5U,0x47B2CF7FU,0x30B5FFE9U,
        0xBDBDF21CU,0xCABAC28AU,0x53B39330U,0x24B4A3A6U,
        0xBAD03605U,0xCDD70693U,0x54DE5729U,0x23D967BFU,
        0xB3667A2EU,0xC4614AB8U,0x5D681B02U,0x2A6F2B94U,
        0xB40BBE37U,0xC30C8EA1U,0x5A05DF1BU,0x2D02EF8DU,
    };

    // Calculate the CRC by doing a lookup in the table for each byte to be
    // CRCed.
    for (INT32 i = byteLen; i > 0; i--)
    {
        crc = table[*pData ^ (UINT8)crc] ^ (crc >> 8);
        pData++;
    }

    // Return the CRC that was calculated.
    return crc;
}

////////////////////////////////////////////////////////////////////////////////
// ApexDiagBinaryCrc::PowerupTest
///
/// @memberof ApexDiagBinaryCrc
///
/// @details This function performs a CRC test of the Apex executable RAM area
///          at powerup.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagBinaryCrc::PowerupTest(void)
{
    const UINT8* codeSpaceStart = (UINT8*)DGN_CODE_SPACE_START;
    UINT32 crc = 0xffffffff;

    // Count CRC of the whole image and measure how long it took - this will be
    // used to determine amount of data to process during background CRC check
    UINT32 startTimeUs = HI_ApexReg.SystemTime;
    crc = CalculateCrc32(codeSpaceStart, m_CodeSize, crc);
    UINT32 crcCountTimeUs = HI_ApexReg.SystemTime - startTimeUs;

    // Start Fault Injection Point 1
    // Code which modifies m_CrcExpected to cause CRC fault at power-up
    // will be injected here.
    // End Fault Injection Point 1

    if (crc != m_CrcExpected)
    {
        firmExcept();
    }

    // prepare data for a dry run to ...
    m_BytesPerStep = 0;
    // ... determine overhead time
    startTimeUs = HI_ApexReg.SystemTime;
    (void)RunTest(NULL);
    UINT32 crcOverheadUs = HI_ApexReg.SystemTime - startTimeUs;

    // calculate bytes for timeslice
    UINT32 bytesPerStep = 0;
    if (crcOverheadUs < ApexDiagnostic::SLICE_TIME_LIMIT_US)
    {
        UINT32 timeForCalculationUs = (ApexDiagnostic::SLICE_TIME_LIMIT_US - crcOverheadUs);
        bytesPerStep = (m_CodeSize * timeForCalculationUs) / crcCountTimeUs;
    }
    else
    {
        // do at least one per slice
        bytesPerStep = 1;
    }

    // Make sure CRC count over all data will finish in desired number of slices
    UINT32 activeTimeslicesBeforeTimeout = 
                    ApexDiagnostic::DgnList[ApexDiagnostic::DGN_EXE_CRC_NUM].timeoutTimeslice 
                    / ApexDiagnostic::DgnList[ApexDiagnostic::DGN_EXE_CRC_NUM].stepValueTimeslice;

    UINT32 timeslicesToFinishCRCCount = (m_CodeSize / bytesPerStep);
    
    // Start Fault Injection Point 4
    // The CRC diagnostic is used to cause the completion check to fail
    // by setting timeoutTimeslice equal to zero.  This will cause the
    // ASSERT below to trip and not fault at the correct spot.  Remove
    // this line to ensure proper fault injection behavior.
    ASSERT( activeTimeslicesBeforeTimeout > timeslicesToFinishCRCCount );
    // End Fault Injection Point 4

    m_BytesPerStep = bytesPerStep;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagBinaryCrc::RunTest
//
/// This function performs runtime tests of the executable RAM area of Apex.
///
/// @param          (not used)
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagBinaryCrc::RunTest(ApexDiagnostic::DiagControlBlock *)
{
    UINT32 numOfBytes = min(m_BytesPerStep, m_BytesLeft);
    m_BytesLeft -=  numOfBytes;

    // Start Fault Injection Point 2
    // Code which modifies m_BytesLeft when InjectFaultFlag is set to cause
    // CRC fault at run-time will be injected here.
    // End Fault Injection Point 2

    m_Crc = CalculateCrc32(m_pCurrentPtr, numOfBytes, m_Crc);

    if (m_BytesLeft > 0)
    {
        return ApexDiagnostic::TEST_IN_PROGRESS;
    }

    // Fail if CRC does not match, unless the debugger running
    if (m_Crc != m_CrcExpected)
    {
        if (!DebugSession())
        {
            firmExcept();
        }
    }

    CrcDataReset();

    return (ApexDiagnostic::TEST_LOOP_COMPLETE);
}
