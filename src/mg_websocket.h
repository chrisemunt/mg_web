/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2022 M/Gateway Developments Ltd,                      |
   | Surrey UK.                                                               |
   | All rights reserved.                                                     |
   |                                                                          |
   | http://www.mgateway.com                                                  |
   |                                                                          |
   | Licensed under the Apache License, Version 2.0 (the "License"); you may  |
   | not use this file except in compliance with the License.                 |
   | You may obtain a copy of the License at                                  |
   |                                                                          |
   | http://www.apache.org/licenses/LICENSE-2.0                               |
   |                                                                          |
   | Unless required by applicable law or agreed to in writing, software      |
   | distributed under the License is distributed on an "AS IS" BASIS,        |
   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
   | See the License for the specific language governing permissions and      |
   | limitations under the License.                                           |      
   |                                                                          |
   ----------------------------------------------------------------------------
*/

#ifndef MG_WEBSOCKET_H
#define MG_WEBSOCKET_H

#define MG_WS_DATA_FRAMING_MASK               0
#define MG_WS_DATA_FRAMING_START              1
#define MG_WS_DATA_FRAMING_PAYLOAD_LENGTH     2
#define MG_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT 3
#define MG_WS_DATA_FRAMING_EXTENSION_DATA     4
#define MG_WS_DATA_FRAMING_APPLICATION_DATA   5
#define MG_WS_DATA_FRAMING_CLOSE              6

#define MG_WS_FRAME_GET_FIN(BYTE)         (((BYTE) >> 7) & 0x01)
#define MG_WS_FRAME_GET_RSV1(BYTE)        (((BYTE) >> 6) & 0x01)
#define MG_WS_FRAME_GET_RSV2(BYTE)        (((BYTE) >> 5) & 0x01)
#define MG_WS_FRAME_GET_RSV3(BYTE)        (((BYTE) >> 4) & 0x01)
#define MG_WS_FRAME_GET_OPCODE(BYTE)      ( (BYTE)       & 0x0F)
#define MG_WS_FRAME_GET_MASK(BYTE)        (((BYTE) >> 7) & 0x01)
#define MG_WS_FRAME_GET_PAYLOAD_LEN(BYTE) ( (BYTE)       & 0x7F)

#define MG_WS_FRAME_SET_FIN(BYTE)         (((BYTE) & 0x01) << 7)
#define MG_WS_FRAME_SET_OPCODE(BYTE)       ((BYTE) & 0x0F)
#define MG_WS_FRAME_SET_MASK(BYTE)        (((BYTE) & 0x01) << 7)
#define MG_WS_FRAME_SET_LENGTH(X64, IDX)  (unsigned char)(((X64) >> ((IDX)*8)) & 0xFF)

#define MG_WS_OPCODE_CONTINUATION 0x0
#define MG_WS_OPCODE_TEXT         0x1
#define MG_WS_OPCODE_BINARY       0x2
#define MG_WS_OPCODE_CLOSE        0x8
#define MG_WS_OPCODE_PING         0x9
#define MG_WS_OPCODE_PONG         0xA

#define MG_WS_WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MG_WS_WEBSOCKET_GUID_LEN 36

#define MG_WS_STATUS_CODE_OK                1000
#define MG_WS_STATUS_CODE_GOING_AWAY        1001
#define MG_WS_STATUS_CODE_PROTOCOL_ERROR    1002
#define MG_WS_STATUS_CODE_RESERVED          1004 /* Protocol 8: frame too large */
#define MG_WS_STATUS_CODE_INVALID_UTF8      1007
#define MG_WS_STATUS_CODE_POLICY_VIOLATION  1008
#define MG_WS_STATUS_CODE_MESSAGE_TOO_LARGE 1009
#define MG_WS_STATUS_CODE_INTERNAL_ERROR    1011

#define MG_WS_MESSAGE_TYPE_INVALID  -1
#define MG_WS_MESSAGE_TYPE_TEXT      0
#define MG_WS_MESSAGE_TYPE_BINARY  128
#define MG_WS_MESSAGE_TYPE_CLOSE   255
#define MG_WS_MESSAGE_TYPE_PING    256
#define MG_WS_MESSAGE_TYPE_PONG    257


#define S0 0x000
#define T1 0x100
#define T2 0x200
#define S1 0x300
#define S2 0x400
#define T3 0x500
#define S3 0x600
#define S4 0x700
#define ER 0x800

#define UTF8_VALID   0x000
#define UTF8_INVALID 0x800

static const unsigned short validate_utf8[2048] = {
   /* S0 (0x000) */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x00-0F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x10-1F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x20-2F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x30-3F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x40-4F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x50-5F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x60-6F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x70-7F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x80-8F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x90-9F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xA0-AF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xB0-BF */
   ER,ER,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xC0-CF */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xD0-DF */
   S1,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,S2,T2,T2, /* %xE0-EF */
   S3,T3,T3,T3,S4,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* T1 (0x100) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x80-8F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %x90-9F */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %xA0-AF */
   S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0,S0, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* T2 (0x200) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %x80-8F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %x90-9F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xA0-AF */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* S1 (0x300) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x80-8F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x90-9F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xA0-AF */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* S2 (0x400) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %x80-8F */
   T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1,T1, /* %x90-9F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xA0-AF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* T3 (0x500) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %x80-8F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %x90-9F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %xA0-AF */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* S3 (0x600) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x80-8F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %x90-9F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %xA0-AF */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
   /* S4 (0x700) */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x00-0F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x10-1F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x20-2F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x30-3F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x40-4F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x50-5F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x60-6F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x70-7F */
   T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2,T2, /* %x80-8F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %x90-9F */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xA0-AF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xB0-BF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xC0-CF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xD0-DF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xE0-EF */
   ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER,ER, /* %xF0-FF */
};

#undef S0
#undef T1
#undef T2
#undef S1
#undef S2
#undef T3
#undef S3
#undef S4
#undef ER


#ifdef __cplusplus
extern "C" {
#endif

int            mg_websocket_check            (MGWEB *pweb);
int            mg_websocket_connection       (MGWEB *pweb);
int            mg_websocket_disconnect       (MGWEB *pweb);
int            mg_websocket_data_framing     (MGWEB *pweb);
void           mg_websocket_incoming_frame   (MGWEB *pweb, MGWSRSTATE *pread_state, char *block, mg_int64_t block_size);
DBX_THR_TYPE   mg_websocket_dbserver_read    (void *arg);
size_t         mg_websocket_create_header    (MGWEB *pweb, int type, unsigned char *header, mg_uint64_t payload_length);

#ifdef __cplusplus
}
#endif

#endif
