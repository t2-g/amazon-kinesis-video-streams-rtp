/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <string.h>
#include <stdint.h>

/* API includes. */
#include "rtp_api.h"
#include "rtp_data_types.h"
#include "rtp_endianness.h"


/* ===========================  EXTERN VARIABLES  =========================== */

#define MAX_PACKETS_IN_A_FRAME  512
#define MAX_NALU_LENGTH         5 * 1024
#define MAX_FRAME_LENGTH        10 * 1024
#define MAX_PACKET_IN_A_FRAME   512

#define VP8_PACKETS_ARR_LEN     10
#define VP8_FRAME_BUF_LEN       32

#define RTP_HEADER_MIN_LENGTH   12 /* No CSRC and no extension. */

uint8_t buffer[ MAX_FRAME_LENGTH ];
uint8_t packetsArray[ MAX_PACKET_IN_A_FRAME ];

void setUp( void )
{
    memset( &( packetsArray[0] ),
            0,
            sizeof( packetsArray ) );
    memset( &( buffer[ 0 ] ),
            0,
            sizeof( buffer ) );
}

void tearDown( void )
{
}

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate Rtp_Init in case of valid inputs.
 */
void test_Rtp_Init_Pass( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_NOT_NULL( ctx.readWriteFunctions.readUint32Fn );

    TEST_ASSERT_NOT_NULL( ctx.readWriteFunctions.writeUint32Fn );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Serialize in case of valid inputs.
 */
void test_Rtp_Serialize_Pass( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = {0};
    uint8_t buffer[ MAX_FRAME_LENGTH ] = {0};
    size_t length = MAX_FRAME_LENGTH;

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 11;
    packet.pPayload = ( uint8_t * ) "hello world";

    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 23,
                       length );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( ( uint8_t * ) "\x80\x60\x04\xD2\x12\x34\x56\x78\x87\x65\x43\x21\x68\x65\x6C\x6C\x6F\x20\x77\x6F\x72\x6C\x64",
                                   buffer,
                                   length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Serialize in case of valid inputs with CSRC identifiers.
 */
void test_Rtp_Serialize_WithCsrc( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer[MAX_FRAME_LENGTH] = { 0 };
    size_t length = MAX_FRAME_LENGTH;
    uint32_t csrcArray[ 2 ] = { 0x11223344, 0x55667788 };

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.header.csrcCount = 2;
    packet.header.pCsrc = csrcArray;
    packet.payloadLength = 12;
    packet.pPayload = ( uint8_t * )"hello world";

    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 32,
                       length );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( ( uint8_t * )"\x82\x60\x04\xD2\x12\x34\x56\x78\x87\x65\x43\x21\x11\x22\x33\x44\x55\x66\x77\x88\x68\x65\x6C\x6C\x6F\x20\x77\x6F\x72\x6C\x64",
                                   buffer,
                                   length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Serialize in case of flag as RTP_HEADER_FLAG_EXTENSION.
 */
void test_Rtp_Serialize_Pass_FlagExtension( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer[MAX_FRAME_LENGTH] = { 0 };
    size_t length = MAX_FRAME_LENGTH;
    uint32_t extensionPayload[ 2 ] = { 0x11223344, 0x55667788 };

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 12;
    packet.pPayload = ( uint8_t * )"hello world";
    packet.header.flags = 0b100;
    packet.header.extension.extensionPayloadLength = 2;
    packet.header.extension.pExtensionPayload = extensionPayload;

    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 36,
                       length );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( ( uint8_t * )"\x90\x60\x04\xD2\x12\x34\x56\x78\x87\x65\x43\x21\x00\x00\x00\x02\x11\x22\x33\x44\x55\x66\x77\x88\x68\x65\x6C\x6C\x6F\x20\x77\x6F\x72\x6C\x64",
                                   buffer,
                                   length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Serialize in case of flag as RTP_HEADER_FLAG_MARKER.
 */
void test_Rtp_Serialize_Pass_FlagMarker( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer[ MAX_FRAME_LENGTH ] = { 0 };
    size_t length = MAX_FRAME_LENGTH;

    result = Rtp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 10;
    packet.pPayload = NULL;
    packet.header.flags = 0b10;


    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 22,
                       length );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( ( uint8_t * ) "\x80\xE0\x04\xD2\x12\x34\x56\x78\x87\x65\x43\x21\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                                   buffer,
                                   length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Serialize in case of flag as RTP_HEADER_FLAG_PADDING.
 */
void test_Rtp_Serialize_Pass_FlagPadding( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer[ MAX_FRAME_LENGTH ] = { 0 };
    size_t length = MAX_FRAME_LENGTH;

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 0;
    packet.pPayload = ( uint8_t * ) "hello world";
    packet.header.flags = 0b1;


    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 12,
                       length );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( ( uint8_t * ) "\xA0\x60\x04\xD2\x12\x34\x56\x78\x87\x65\x43\x21",
                                   buffer,
                                   length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_DeSerialize in case of valid inputs.
 */
void test_Rtp_DeSerialize_Pass( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = { 0x80, 0x60, 0x04, 0xD2, 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64 };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 96,
                       packet.header.payloadType );

    TEST_ASSERT_EQUAL( 1234,
                       packet.header.sequenceNumber );

    TEST_ASSERT_EQUAL( 0x12345678,
                       packet.header.timestamp );

    TEST_ASSERT_EQUAL( 0x87654321,
                       packet.header.ssrc );

    TEST_ASSERT_EQUAL( 11,
                       packet.payloadLength );

    TEST_ASSERT_EQUAL_STRING_LEN( "hello world",
                                  ( char * ) packet.pPayload,
                                  packet.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTP_Init in case of bad parameters.
 */
void test_Rtp_Init_BadParams( void )
{
    RtpResult_t result;

    result = Rtp_Init( NULL );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_Serialize in case of bad parameters.
 */
void test_Rtp_Serialize_BadParams( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer;
    size_t length = sizeof( buffer );

    result = Rtp_Serialize( NULL,
                            &( packet ),
                            &( buffer ),
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    result = Rtp_Serialize( &( ctx ),
                            NULL,
                            &( buffer ),
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    result = Rtp_Serialize( &( ctx ),
                            &( packet ),
                            &( buffer ),
                            0 );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_Deserialize in case of bad parameters.
 */
void test_Rtp_DeSerialize_BadParams( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = { 0x80, 0x60, 0x04, 0xD2, 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64 };
    size_t length = sizeof( serializedPacket );

    result = Rtp_DeSerialize( NULL,
                              serializedPacket,
                              length,
                              &( packet ) );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    result = Rtp_DeSerialize( &( ctx ),
                              NULL,
                              length,
                              &( packet ) );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    length = RTP_HEADER_MIN_LENGTH - 1;

    result = Rtp_DeSerialize( &( ctx ),
                              serializedPacket,
                              length,
                              &( packet ) );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    packet.pPayload = NULL;
    packet.payloadLength = 0;

    result = Rtp_DeSerialize( &( ctx ),
                              serializedPacket,
                              length,
                              &( packet ) );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );

    result = Rtp_DeSerialize( &( ctx ),
                              serializedPacket,
                              length,
                              NULL );

    TEST_ASSERT_EQUAL( RTP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_Serialize incase of out of memory - buffer
 * is NULL and the provided length is less than the required length.
 */
void test_Rtp_Serialize_NullBuffer_ShortLength( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    size_t length = 5;

    result = Rtp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 12;
    packet.pPayload = ( uint8_t * )"hello world";

    result = Rtp_Serialize( &ctx,
                            &packet,
                            NULL,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 24,
                       length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_Serialize incase of out of memory - buffer
 * is not NULL and the provided length is less than the required length.
 */
void test_Rtp_Serialize_OutOfMemory( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t buffer[ 10 ] = { 0 };
    size_t length = sizeof( buffer );

    result = Rtp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    packet.header.payloadType = 96;
    packet.header.sequenceNumber = 1234;
    packet.header.timestamp = 0x12345678;
    packet.header.ssrc = 0x87654321;
    packet.payloadLength = 12;
    packet.pPayload = ( uint8_t * )"hello world";

    result = Rtp_Serialize( &ctx,
                            &packet,
                            buffer,
                            &length );

    TEST_ASSERT_EQUAL( RTP_RESULT_OUT_OF_MEMORY,
                       result );

    TEST_ASSERT_EQUAL( 10,
                       length );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with a valid packet without extension or padding.
 */
void test_Rtp_DeSerialize_Pass_NoExtensionNoPadding( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x80, 0x60, 0x04, 0xD2,
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64 // Payload
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 0,
                       packet.header.flags );  // No flags set

    TEST_ASSERT_EQUAL( 0,
                       packet.header.csrcCount );

    TEST_ASSERT_NULL( packet.header.pCsrc );

    TEST_ASSERT_EQUAL( 96,
                       packet.header.payloadType );

    TEST_ASSERT_EQUAL( 1234,
                       packet.header.sequenceNumber );

    TEST_ASSERT_EQUAL( 0x12345678,
                       packet.header.timestamp );

    TEST_ASSERT_EQUAL( 0x87654321,
                       packet.header.ssrc );

    TEST_ASSERT_EQUAL_STRING_LEN( "hello world",
                                  ( char * ) packet.pPayload,
                                  packet.payloadLength );

    TEST_ASSERT_EQUAL( 11,
                       packet.payloadLength );

}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with a valid packet with CSRC identifiers.
 */
void test_Rtp_DeSerialize_Pass_WithCsrc( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x82, 0x60, 0x04, 0xD2, // Header with CSRC count = 2
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x11, 0x22, 0x33, 0x44, // CSRC 1
        0x55, 0x66, 0x77, 0x88, // CSRC 2
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64 // Payload
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( 0,
                       packet.header.flags );  // No flags set

    TEST_ASSERT_EQUAL( 2,
                       packet.header.csrcCount );

    TEST_ASSERT_EQUAL_HEX32( 0x11223344,
                             packet.header.pCsrc[0] );

    TEST_ASSERT_EQUAL_HEX32( 0x55667788,
                             packet.header.pCsrc[1] );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with a valid packet with extension header.
 */
void test_Rtp_DeSerialize_Pass_WithExtension( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x90, 0x60, 0x04, 0xD2, // Header with extension flag set
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x00, 0x00, 0x00, 0x02, // Extension header (profile = 0, length = 2)
        0x11, 0x22, 0x33, 0x44, // Extension payload 1
        0x55, 0x66, 0x77, 0x88, // Extension payload 2
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x64 // Payload
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( RTP_HEADER_FLAG_EXTENSION,
                       packet.header.flags );

    TEST_ASSERT_EQUAL( 0,
                       packet.header.csrcCount );

    TEST_ASSERT_EQUAL( 0,
                       packet.header.extension.extensionProfile );

    TEST_ASSERT_EQUAL( 2,
                       packet.header.extension.extensionPayloadLength );

    TEST_ASSERT_EQUAL_HEX32( 0x11223344,
                             packet.header.extension.pExtensionPayload[0] );

    TEST_ASSERT_EQUAL_HEX32( 0x55667788,
                             packet.header.extension.pExtensionPayload[1] );
}

/*-----------------------------------------------------------*/
/**
 * @brief Validate Rtp_DeSerialize with a valid packet with padding.
 */
void test_Rtp_DeSerialize_Pass_WithPadding( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0xA0, 0x60, 0x04, 0xD2, // Header with padding flag set
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, // Payload
        0x00, 0x00 // Padding bytes
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( RTP_HEADER_FLAG_PADDING,
                       packet.header.flags );

    TEST_ASSERT_EQUAL( 0,
                       packet.header.csrcCount );

    TEST_ASSERT_EQUAL( 13,
                       packet.payloadLength );

    TEST_ASSERT_EQUAL_STRING_LEN( "hello world",
                                  ( char * )packet.pPayload,
                                  packet.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with a malformed packet (incorrect version).
 */
void test_Rtp_DeSerialize_MalformedPacket_WrongVersion( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x00, 0x60, 0x04, 0xD2, // Wrong version (0 instead of 2)
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x64 // Payload
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_WRONG_VERSION,
                       result );
}


/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with a malformed packet (no extension header specified).
 */
void test_Rtp_DeSerialize_MalformedPacket_NoExtensionHeader( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x90, 0x60, 0x04, 0xD2, // Extension flag set
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21
        // Extension header missing
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with insufficient data to read CSRC identifiers.
 */
void test_Rtp_DeSerialize_InsufficientCsrcData( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x82, 0x60, 0x04, 0xD2, // Header with CSRC count = 2
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x11, 0x22, 0x33, 0x44  // Only 1 CSRC identifier (not enough for count = 2)
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with insufficient data to read extension payload.
 */
void test_Rtp_DeSerialize_InsufficientExtensionPayloadData( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x90, 0x60, 0x04, 0xD2, // Extension flag set
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x00, 0x00, 0x00, 0x02, // Extension header (profile = 0, length = 2)
        0x11, 0x22, 0x33, 0x44  // Only 1 extension payload word (not enough for length = 2)
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with no payload data.
 */
void test_Rtp_DeSerialize_NoPayload( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x80, 0x60, 0x04, 0xD2,
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    TEST_ASSERT_NULL( packet.pPayload );

    TEST_ASSERT_EQUAL( 0,
                       packet.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Rtp_DeSerialize with the marker bit not set.
 */
void test_Rtp_DeSerialize_MarkerBitNotSet( void )
{
    RtpResult_t result;
    RtpContext_t ctx = { 0 };
    RtpPacket_t packet = { 0 };
    uint8_t serializedPacket[] = {
        0x80, 0x80, 0x04, 0xD2, // Marker bit not set
        0x12, 0x34, 0x56, 0x78,
        0x87, 0x65, 0x43, 0x21,
        0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x64 // Payload
    };
    size_t length = sizeof( serializedPacket );

    result = Rtp_Init( &ctx );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );

    result = Rtp_DeSerialize( &ctx,
                              serializedPacket,
                              length,
                              &packet );

    TEST_ASSERT_EQUAL( RTP_RESULT_OK,
                       result );
}

/*-----------------------------------------------------------*/

