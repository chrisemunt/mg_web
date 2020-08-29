/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2020 M/Gateway Developments Ltd,                      |
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

#if !defined(APR_ARRAY_IDX)
#define APR_ARRAY_IDX(ary,i,type) (((type *)(ary)->elts)[i])
#endif
#if !defined(APR_ARRAY_PUSH)
#define APR_ARRAY_PUSH(ary,type) (*((type *)apr_array_push(ary)))
#endif

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

#define MG_WS_QUEUE_CAPACITY 16 /* capacity of queue used for communication between main thread and other threads */


/* SHA1 */
#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20

/* define an unsigned 32-bit type */

#if defined(_MSC_VER)
  typedef   unsigned long    sha1_32t;
#elif defined(ULONG_MAX) && ULONG_MAX == 0xfffffffful
  typedef   unsigned long    sha1_32t;
#elif defined(UINT_MAX) && UINT_MAX == 0xffffffff
  typedef   unsigned int     sha1_32t;
#else
#  error Define sha1_32t as an unsigned 32 bit type in csp.h
#endif

/* type to hold the SHA256 context  */

typedef struct {
   sha1_32t count[2];
   sha1_32t hash[5];
   sha1_32t wbuf[16];
} sha1_ctx;

/*
   To obtain the highest speed on processors with 32-bit words, this code
   needs to determine the order in which bytes are packed into such words.
   The following block of code is an attempt to capture the most obvious
   ways in which various environments specify their endian definitions.
   It may well fail, in which case the definitions will need to be set by
   editing at the points marked **** EDIT HERE IF NECESSARY **** below.
*/

/*  PLATFORM SPECIFIC INCLUDES */

#define BRG_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define BRG_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if defined(__GNUC__) || defined(__GNU_LIBRARY__)
#  if defined(__FreeBSD__) || defined(__OpenBSD__)
#if defined(FREEBSD)
#      include <machine/endian.h>
#else
#    include <sys/endian.h>
#endif
#      include <machine/endian.h>
#  elif defined( BSD ) && ( BSD >= 199103 )
#      include <machine/endian.h>
#  elif defined(__APPLE__)
#    if defined(__BIG_ENDIAN__) && !defined( BIG_ENDIAN )
#      define BIG_ENDIAN
#    elif defined(__LITTLE_ENDIAN__) && !defined( LITTLE_ENDIAN )
#      define LITTLE_ENDIAN
#    endif
#  else
#if !defined(SOLARIS)
#    include <endian.h>
#endif
#    if !defined(__BEOS__)
#if !defined(SOLARIS) && !defined(LINUX)
#      include <byteswap.h>
#endif
#    endif
#  endif
#endif

#if !defined(PLATFORM_BYTE_ORDER)
#  if defined(LITTLE_ENDIAN) || defined(BIG_ENDIAN)
#    if    defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(LITTLE_ENDIAN) &&  defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  elif defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN)
#    if    defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(_LITTLE_ENDIAN) &&  defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#   endif
#  elif defined(__LITTLE_ENDIAN__) || defined(__BIG_ENDIAN__)
#    if    defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(__LITTLE_ENDIAN__) &&  defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __LITTLE_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  endif
#endif


/*  if the platform is still unknown, try to find its byte order    */
/*  from commonly used machine defines                              */

#if !defined(PLATFORM_BYTE_ORDER)

#if   defined( __alpha__ ) || defined( __alpha ) || defined( i386 )       || \
      defined( __i386__ )  || defined( _M_I86 )  || defined( _M_IX86 )    || \
      defined( __OS2__ )   || defined( sun386 )  || defined( __TURBOC__ ) || \
      defined( vax )       || defined( vms )     || defined( VMS )        || \
      defined( __VMS )
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN

#elif defined( AMIGA )    || defined( applec )  || defined( __AS400__ )  || \
      defined( _CRAY )    || defined( __hppa )  || defined( __hp9000 )   || \
      defined( ibm370 )   || defined( mc68000 ) || defined( m68k )       || \
      defined( __MRC__ )  || defined( __MVS__ ) || defined( __MWERKS__ ) || \
      defined( sparc )    || defined( __sparc)  || defined( SYMANTEC_C ) || \
      defined( __TANDEM ) || defined( THINK_C ) || defined( __VMCMS__ )
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN

#elif defined(AIX) || defined(AIX5)

#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN

#elif defined(LINUX) /* CMT669 */

#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN

#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#else

#ifdef _WIN64 /* Itanium */
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#else
#  error "Cannot determine the platform byte order for the SHA1 algorithm"
#endif

#endif

#endif

#ifdef _MSC_VER
#pragma intrinsic(memcpy)
#endif

#if 0 && defined(_MSC_VER)
#define rotl32  _lrotl
#define rotr32  _lrotr
#else
#define rotl32(x,n)   (((x) << n) | ((x) >> (32 - n)))
#define rotr32(x,n)   (((x) >> n) | ((x) << (32 - n)))
#endif

#if !defined(bswap_32)
#define bswap_32(x) ((rotr32((x), 24) & 0x00ff00ff) | (rotr32((x), 8) & 0xff00ff00))
#endif

#if (PLATFORM_BYTE_ORDER == BRG_LITTLE_ENDIAN)
#define SWAP_BYTES
#else
#undef  SWAP_BYTES
#endif

#if defined(SWAP_BYTES)
#define bsw_32(p,n) \
    { int _i = (n); while(_i--) ((sha1_32t*)p)[_i] = bswap_32(((sha1_32t*)p)[_i]); }
#else
#define bsw_32(p,n)
#endif

#define SHA1_MASK   (SHA1_BLOCK_SIZE - 1)

#if 0

#define ch(x,y,z)       (((x) & (y)) ^ (~(x) & (z)))
#define parity(x,y,z)   ((x) ^ (y) ^ (z))
#define maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#else

#define ch(x,y,z)       ((z) ^ ((x) & ((y) ^ (z))))
#define parity(x,y,z)   ((x) ^ (y) ^ (z))
#define maj(x,y,z)      (((x) & (y)) | ((z) & ((x) ^ (y))))

#endif

/* Compile 64 bytes of hash data into SHA1 context. Note    */
/* that this routine assumes that the byte order in the     */
/* ctx->wbuf[] at this point is in such an order that low   */
/* address bytes in the ORIGINAL byte stream in this buffer */
/* will go to the high end of 32-bit words on BOTH big and  */
/* little endian systems                                    */

#ifdef ARRAY
#define q(v,n)  v[n]
#else
#define q(v,n)  v##n
#endif

#define one_cycle(v,a,b,c,d,e,f,k,h)            \
    q(v,e) += rotr32(q(v,a),27) +               \
              f(q(v,b),q(v,c),q(v,d)) + k + h;  \
    q(v,b)  = rotr32(q(v,b), 2)

#define five_cycle(v,f,k,i)                 \
    one_cycle(v, 0,1,2,3,4, f,k,hf(i  ));   \
    one_cycle(v, 4,0,1,2,3, f,k,hf(i+1));   \
    one_cycle(v, 3,4,0,1,2, f,k,hf(i+2));   \
    one_cycle(v, 2,3,4,0,1, f,k,hf(i+3));   \
    one_cycle(v, 1,2,3,4,0, f,k,hf(i+4))


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

char           mg_b64_ntc                    (unsigned char n);
unsigned char  mg_b64_ctn                    (char c);
int            mg_b64_encode                 (char *from, char *to, int length, int quads);
int            mg_b64_decode                 (char *from, char *to, int length);
int            mg_b64_get_ebuffer_size       (int l, int q);
int            mg_b64_strip_ebuffer          (char *buf, int length);
void           mg_sha1_compile               (sha1_ctx ctx[1]);
void           mg_sha1_begin                 (sha1_ctx ctx[1]);
void           mg_sha1_hash                  (const unsigned char data[], unsigned long len, sha1_ctx ctx[1]);
void           mg_sha1_end                   (unsigned char hval[], sha1_ctx ctx[1]);
void           mg_sha1                       (unsigned char hval[], const unsigned char data[], unsigned long len);

#ifdef __cplusplus
}
#endif

#endif
