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


#include "mg_websys.h"
#include "mg_web.h"
#include "mg_websocket.h"


int mg_websocket_check(MGWEB *pweb)
{
   int rc, n, len, lenu, lenc, upgrade_connection, protocol_version;
   char *p, *pa, *pz;
   char upgrade[128], buffer[256], connection[256], wsfunction[256];
   char host[128], sec_websocket_key[256], token[256], hash[256], sec_websocket_accept[256], sec_websocket_protocol[32];

   protocol_version = 0;
   upgrade_connection = 0;

   if (pweb->request_method_len != 3 && !strncmp(pweb->request_method, "GET", 3)) {
      return -1;
   }
   lenu = 32;
   rc = mg_get_cgi_variable(pweb, "HTTP_UPGRADE", upgrade, &lenu);

   if (rc != MG_CGI_SUCCESS) {
      return -1;
   }
   if (lenu > 0)
      upgrade[lenu] = '\0';
   else
      upgrade[32] = '\0';

   mg_lcase(upgrade);

   lenc = 250;
   rc = mg_get_cgi_variable(pweb, "HTTP_CONNECTION", connection, &lenc);

   if (rc != MG_CGI_SUCCESS) {
      return -1;
   }
   if (lenc > 0)
      connection[lenc] = '\0';
   else
      connection[250] = '\0';

   mg_lcase(connection);

   if (!lenu || !lenc || strcmp(upgrade, "websocket")) {
      return -1;
   }

   upgrade_connection = !strcmp(connection, "upgrade");
   pa = NULL;
   pz = NULL;
   if (!upgrade_connection) {
      p = connection;
      while (p && *p) { /* Parse the Connection value */
         while (*p == ' ')
            p ++;
         pz = strstr(p, ",");
         if (pz) {
            *pz = '\0';
         }
         pa = strstr(p, " ");
         if (pa)
            *pa = '\0';
         pa = strstr(p, ";");
         if (pa)
            *pa = '\0';
         upgrade_connection = !strcmp(p, "upgrade");
         if (upgrade_connection) {
            break;
         }
         p = pz + 1;
      }
   }
   if (!upgrade_connection) {
      return -1;
   }

   len = 0;
   *sec_websocket_accept = '\0';
   *sec_websocket_key = '\0';
   *sec_websocket_protocol = '\0';
   *token = '\0';
   *hash = '\0';

   len = 120;
   rc = mg_get_cgi_variable(pweb, "SERVER_NAME", host, &len);
   host[120] = '\0';
   len = 250;
   rc = mg_get_cgi_variable(pweb, "HTTP_SEC_WEBSOCKET_KEY", sec_websocket_key, &len);
   sec_websocket_key[250] = '\0';
   len = 30;
   rc = mg_get_cgi_variable(pweb, "HTTP_SEC_WEBSOCKET_PROTOCOL", sec_websocket_protocol, &len);
   sec_websocket_protocol[30] = '\0';

   len = 30;
   rc = mg_get_cgi_variable(pweb, "HTTP_SEC_WEBSOCKET_VERSION", buffer, &len);
   buffer[30] = '\0';

   protocol_version = (int) strtol((char *) buffer, NULL, 10);

   lenu = 0;
   lenc = 0;
   len = 0;
   for (n = 0; n < pweb->script_name_len; n ++) {
      if (pweb->script_name[n] == '/') {
         lenu = n + 1;
      }
      else if (pweb->script_name[n] == '^') {
         lenc = n;
         break;
      }
   }
   if (lenc) {
      for (n = lenu; ; n ++) {
         if (pweb->script_name[n] == '/' || pweb->script_name[n] == '.') {
            break;
         }
         wsfunction[len ++] = pweb->script_name[n];
      }
      wsfunction[len] = '\0';
   }
   if (!len) {
      mg_log_buffer(pweb->plog, pweb, pweb->script_name, pweb->script_name_len, "Cannot find WebSocket function name in the URL", 0);
      return -1;
   }

   if (pweb->plog->log_frames) {
      char bufferx[2048];
      sprintf(bufferx, "host=%s; sec_websocket_key=%s; sec_websocket_protocol=%s; protocol_version=%d; function=%s", host, sec_websocket_key, sec_websocket_protocol, protocol_version, wsfunction);
      mg_log_event(pweb->plog, pweb, bufferx, "mg_web: websocket information", 0);
   }

   if ((*host) && (*sec_websocket_key) && ((protocol_version == 7) || (protocol_version == 8) || (protocol_version == 13))) {
      /* const char *sec_websocket_origin = apr_table_get(pRCB->r->headers_in, "Sec-WebSocket-Origin"); */
      /* const char *origin = apr_table_get(pRCB->r->headers_in, "Origin"); */
      /* TODO : We need to validate the Host and Origin */

      pweb->pwsock = (MGWEBSOCK *) mg_malloc(pweb->pweb_server, sizeof(MGWEBSOCK), 0);
      memset((void *) pweb->pwsock, 0, sizeof(MGWEBSOCK));
      pweb->pwsock->closing = 0;
      pweb->pwsock->status = MG_WEBSOCKET_NOCON;
      pweb->pwsock->protocol_version = protocol_version;
      strcpy(pweb->pwsock->sec_websocket_key, sec_websocket_key);
      strcpy(pweb->pwsock->sec_websocket_protocol, sec_websocket_protocol);

      mg_websocket_create_lock(pweb);

      rc = mg_websocket_init(pweb);

      if (pweb->pwsock->status) {
         goto mg_websocket_check_success;
      }

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_alloc - 1000);

      strcpy(token, sec_websocket_key);
      strcpy(token, MG_WS_WEBSOCKET_GUID);

      mg_sha1((unsigned char *) hash, (const unsigned char *) token, (unsigned long) strlen(token));
      hash[20] = '\0';

      n = mg_b64_encode(hash, sec_websocket_accept, 20, 0);
      sec_websocket_accept[n] = '\0';

      strcpy(pweb->response_headers, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ");
      strcat(pweb->response_headers, sec_websocket_accept);
      strcat(pweb->response_headers, "\r\n");
      if (sec_websocket_protocol[0]) {
         strcat(pweb->response_headers, "Sec-WebSocket-Protocol: ");
         strcat(pweb->response_headers, sec_websocket_protocol);
         strcat(pweb->response_headers, "\r\n");
      }
      strcat(pweb->response_headers, "\r\n");
      pweb->response_headers_len = (int) strlen(pweb->response_headers);

      mg_submit_headers(pweb);
      pweb->pwsock->status = MG_WEBSOCKET_HEADERS_SENT;

mg_websocket_check_success:

      sprintf(buffer, "ws=%d", protocol_version);
      len = (int) strlen(buffer);
      p = (char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
      strcpy((char *) p, buffer);
      mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
      pweb->input_buf.len_used += (len + 5);

      sprintf(buffer, "wsfunction=%s", wsfunction);
      len = (int) strlen(buffer);
      p = (char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
      strcpy((char *) p, buffer);
      mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
      pweb->input_buf.len_used += (len + 5);

   }
else
      mg_log_event(pweb->plog, pweb, "bad ws", "mg_web: websocket information", 0);


   return upgrade_connection;
}


int mg_websocket_connection(MGWEB *pweb)
{
   int rc;
   char *p, *pz;

   pweb->pwsock->binary = 0;

   if (pweb->plog->log_frames) {
      mg_log_buffer(pweb->plog, pweb, pweb->response_headers, pweb->response_headers_len, "mg_web: start WebSocket", 0);
   }

   p = strstr(pweb->response_headers, "Error: ");
   if (p) {
      p += 7;
      pz = strstr(p, "\r\n");
      if (pz) {
         *pz = '\0';
      }
      mg_log_event(pweb->plog, pweb, p, "mg_web: start WebSocket: Error", 0);
      return -1;
   }
   p = strstr(pweb->response_headers, "Binary: 1");
   if (p) {
      pweb->pwsock->binary = 1;
   }

#if !defined(_WIN32)
   rc = pipe(pweb->pcon->int_pipe);
   if (rc == -1) {
      return rc;
   }
#endif

   rc = mg_thread_create(&(pweb->pwsock->db_read_thread), mg_websocket_dbserver_read, (void *) pweb);

   /* The main data framing loop */
   rc = mg_websocket_data_framing(pweb);

   return rc;
}


int mg_websocket_disconnect(MGWEB *pweb)
{
   int rc;
/*
   unsigned char status_code_buffer[2];
   status_code_buffer[0] = (MG_WS_STATUS_CODE_INTERNAL_ERROR >> 8) & 0xFF;
   status_code_buffer[1] = MG_WS_STATUS_CODE_INTERNAL_ERROR & 0xFF;
   mg_websocket_write_block(pweb, MG_WS_MESSAGE_TYPE_CLOSE, (unsigned char *) status_code_buffer, sizeof(status_code_buffer));
*/

   mg_cleanup(pweb->pcon, pweb);
   mg_release_connection(pweb, pweb->pcon, 1);

#if !defined(_WIN32)
   rc = write(pweb->pcon->int_pipe[1], "exit", 4);
#endif

   mg_thread_join(&(pweb->pwsock->db_read_thread));
   mg_websocket_destroy_lock(pweb);

#if !defined(_WIN32)
   close(pweb->pcon->int_pipe[0]);
   close(pweb->pcon->int_pipe[1]);
   pweb->pcon->int_pipe[0] = 0;
   pweb->pcon->int_pipe[1] = 0;
#endif

   rc = mg_websocket_exit(pweb);

   return rc;
}



int mg_websocket_data_framing(MGWEB *pweb)
{
   int rc;
   MGWSRSTATE read_state;
   MGWSFDATA control_frame = { 0, NULL, 1, 8, UTF8_VALID, 0 };
   MGWSFDATA message_frame = { 0, NULL, 1, 0, UTF8_VALID, 0 };
   MGWEBSOCK * pwsock = pweb->pwsock;

   if (mg_websocket_frame_init(pweb) == CACHE_SUCCESS) {

      read_state.extension_bytes_remaining = 0;
      read_state.payload_length = 0;
      read_state.mask_offset = 0;
      read_state.framing_state = MG_WS_DATA_FRAMING_START;
      read_state.payload_length_bytes_remaining = 0;
      read_state.mask_index = 0;
      read_state.masking = 0;
      memset(read_state.mask, 0, sizeof(read_state.mask));
      read_state.fin = 0;
      read_state.opcode = 0xFF;
      read_state.control_frame = control_frame;
      read_state.message_frame = message_frame;
      read_state.frame = &(read_state.control_frame);
      read_state.status_code = MG_WS_STATUS_CODE_OK;

      pwsock->status = MG_WEBSOCKET_CONNECTED;
      pwsock->remaining_length = 0;

      while (read_state.framing_state != MG_WS_DATA_FRAMING_CLOSE) {
         rc = mg_websocket_frame_read(pweb, &read_state);
         if (rc != CACHE_SUCCESS) {
            char bufferx[256];
            sprintf(bufferx, "Error code %d", rc);
            mg_log_event(pweb->plog, pweb, bufferx, "mg_web: WebSocket error", 0);
            break;
         }
      }
      if (read_state.message_frame.application_data != NULL) {
         free(read_state.message_frame.application_data);
      }
      if (read_state.control_frame.application_data != NULL) {
         free(read_state.control_frame.application_data);
      }

      /* Send server-side closing handshake */
      pwsock->status_code_buffer[0] = (read_state.status_code >> 8) & 0xFF;
      pwsock->status_code_buffer[1] = read_state.status_code & 0xFF;
      mg_websocket_write_block(pweb, MG_WS_MESSAGE_TYPE_CLOSE, (unsigned char *) pwsock->status_code_buffer, sizeof(pwsock->status_code_buffer));

      mg_websocket_frame_exit(pweb);
   }

   return 0;
}


void mg_websocket_incoming_frame(MGWEB *pweb, MGWSRSTATE *pread_state, char *block, mg_int64_t block_size)
{
   int close_reason;
   mg_int64_t payload_limit, block_offset;
   payload_limit = 32 * 1024 * 1024;
   block_offset = 0;
/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_websocket_incoming_frame: block_size=%d; framing_state=%d; remaining_length=%lu; block_size=%llu", (int) pweb->pwsock->block_size, (int) pread_state->framing_state, (unsigned long) pweb->pwsock->remaining_length, block_size);
      mg_log_buffer(pweb->plog, pweb, (char *) pweb->pwsock->block, (int) block_size, bufferx, 0);
   }
*/
   close_reason = 0;
   while (block_offset < block_size) {
      switch (pread_state->framing_state) {
         case MG_WS_DATA_FRAMING_START:
            /*
               Since we don't currently support any extensions,
               the reserve bits must be 0
            */
            if ((MG_WS_FRAME_GET_RSV1(block[block_offset]) != 0) || (MG_WS_FRAME_GET_RSV2(block[block_offset]) != 0) || (MG_WS_FRAME_GET_RSV3(block[block_offset]) != 0)) {
               pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
               pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
               close_reason = 1;
               break;
            }
            pread_state->fin = MG_WS_FRAME_GET_FIN(block[block_offset]);
            pread_state->opcode = MG_WS_FRAME_GET_OPCODE(block[block_offset ++]);

            pread_state->framing_state = MG_WS_DATA_FRAMING_PAYLOAD_LENGTH;

            if (pread_state->opcode >= 0x8) { /* Control frame */
               if (pread_state->fin) {
                  pread_state->frame = &(pread_state->control_frame);
                  pread_state->frame->opcode = pread_state->opcode;
                  pread_state->frame->utf8_state = UTF8_VALID;
               }
               else {
                  pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                  pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
                  close_reason = 2;
                  break;
               }
            }
            else { /* Message frame */
               pread_state->frame = &(pread_state->message_frame);
               if (pread_state->opcode) {
                  if (pread_state->frame->fin) {
                     pread_state->frame->opcode = pread_state->opcode;
                     pread_state->frame->utf8_state = UTF8_VALID;
                  }
                  else {
                     pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                     pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
                     close_reason = 3;
                     break;
                  }
               }
               else if (pread_state->frame->fin || ((pread_state->opcode = pread_state->frame->opcode) == 0)) {
                  pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                  pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
                  close_reason = 4;
                  break;
               }
               pread_state->frame->fin = pread_state->fin;
            }
            pread_state->payload_length = 0;
            pread_state->payload_length_bytes_remaining = 0;

            if (block_offset >= block_size) {
               break; /* Only break if we need more data */
            }
            /* Fall through */
         case MG_WS_DATA_FRAMING_PAYLOAD_LENGTH:
            pread_state->payload_length = (mg_int64_t) MG_WS_FRAME_GET_PAYLOAD_LEN(block[block_offset]);
            pread_state->masking = MG_WS_FRAME_GET_MASK(block[block_offset ++]);

            if (pread_state->payload_length == 126) {
               pread_state->payload_length = 0;
               pread_state->payload_length_bytes_remaining = 2;
            }
            else if (pread_state->payload_length == 127) {
               pread_state->payload_length = 0;
               pread_state->payload_length_bytes_remaining = 8;
            }
            else {
               pread_state->payload_length_bytes_remaining = 0;
            }
            if ((pread_state->masking == 0) ||   /* Client-side mask is required */
                  ((pread_state->opcode >= 0x8) && /* Control opcodes cannot have a payload larger than 125 bytes */
                  (pread_state->payload_length_bytes_remaining != 0))) {
               pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
               pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
               close_reason = 5;
               break;
            }
            else {
               pread_state->framing_state = MG_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT;
            }
            if (block_offset >= block_size) {
               break;  /* Only break if we need more data */
            }
         case MG_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT:
            while ((pread_state->payload_length_bytes_remaining > 0) && (block_offset < block_size)) {
               pread_state->payload_length *= 256;
               pread_state->payload_length += (unsigned char) block[block_offset ++];
               pread_state->payload_length_bytes_remaining --;
            }
            if (pread_state->payload_length_bytes_remaining == 0) {
               if ((pread_state->payload_length < 0) || (pread_state->payload_length > payload_limit)) {
                  /* Invalid payload length */
                  pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                  pread_state->status_code = (pweb->pwsock->protocol_version >= 13) ? MG_WS_STATUS_CODE_MESSAGE_TOO_LARGE : MG_WS_STATUS_CODE_RESERVED;
                  break;
               }
               else if (pread_state->masking != 0) {
                  pread_state->framing_state = MG_WS_DATA_FRAMING_MASK;
               }
               else {
                  pread_state->framing_state = MG_WS_DATA_FRAMING_EXTENSION_DATA;
                  break;
               }
            }
            if (block_offset >= block_size) {
               break;  /* Only break if we need more data */
            }
         case MG_WS_DATA_FRAMING_MASK:
            while ((pread_state->mask_index < 4) && (block_offset < block_size)) {
               pread_state->mask[pread_state->mask_index ++] = block[block_offset ++];
            }
            if (pread_state->mask_index == 4) {
               pread_state->framing_state = MG_WS_DATA_FRAMING_EXTENSION_DATA;
               pread_state->mask_offset = 0;
               pread_state->mask_index = 0;
               if ((pread_state->mask[0] == 0) && (pread_state->mask[1] == 0) && (pread_state->mask[2] == 0) && (pread_state->mask[3] == 0)) {
                  pread_state->masking = 0;
               }
            }
            else {
               break;
            }
            /* Fall through */
         case MG_WS_DATA_FRAMING_EXTENSION_DATA:
            /* Deal with extension data when we support them -- FIXME */
            if (pread_state->extension_bytes_remaining == 0) {
               if (pread_state->payload_length > 0) {
                  pread_state->frame->application_data = (unsigned char *) realloc(pread_state->frame->application_data, (size_t) (pread_state->frame->application_data_offset + pread_state->payload_length));
                  if (pread_state->frame->application_data == NULL) {
                     pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                     pread_state->status_code = (pweb->pwsock->protocol_version >= 13) ? MG_WS_STATUS_CODE_INTERNAL_ERROR : MG_WS_STATUS_CODE_GOING_AWAY;
                     close_reason = 6;
                     break;
                  }
               }
               pread_state->framing_state = MG_WS_DATA_FRAMING_APPLICATION_DATA;
            }
            /* Fall through */
         case MG_WS_DATA_FRAMING_APPLICATION_DATA:
            {
               mg_int64_t block_data_length;
               mg_int64_t block_length = 0;
               mg_uint64_t application_data_offset = pread_state->frame->application_data_offset;
               unsigned char *application_data = pread_state->frame->application_data;

               block_length = block_size - block_offset;
               block_data_length = (pread_state->payload_length > block_length) ? block_length : pread_state->payload_length;

               if (pread_state->masking) {
                  mg_int64_t i;

                  if (pread_state->opcode == MG_WS_OPCODE_TEXT) {
                     unsigned int utf8_state = pread_state->frame->utf8_state;
                     unsigned char c;

                     for (i = 0; i < block_data_length; i++) {
                        c = block[block_offset ++] ^ pread_state->mask[pread_state->mask_offset ++ & 3];
                        utf8_state = validate_utf8[utf8_state + c];
                        if (utf8_state == UTF8_INVALID) {
                           pread_state->payload_length = block_data_length;
                           break;
                        }
                        application_data[application_data_offset++] = c;
                     }
                     pread_state->frame->utf8_state = utf8_state;
                  }
                  else {
                     /* Need to optimize the unmasking -- FIXME */
                     for (i = 0; i < block_data_length; i ++) {
                        application_data[application_data_offset ++] = block[block_offset ++] ^ pread_state->mask[pread_state->mask_offset ++ & 3];
                     }
                  }
               }
               else if (block_data_length > 0) {
                  memcpy(&application_data[application_data_offset], &block[block_offset], (size_t) block_data_length);
                  if (pread_state->opcode == MG_WS_OPCODE_TEXT) {
                     mg_int64_t i, application_data_end = application_data_offset + block_data_length;
                     unsigned int utf8_state = pread_state->frame->utf8_state;

                     for (i = application_data_offset; i < application_data_end; i ++) {
                        utf8_state = validate_utf8[utf8_state + application_data[i]];
                        if (utf8_state == UTF8_INVALID) {
                           pread_state->payload_length = block_data_length;
                           break;
                        }
                     }
                     pread_state->frame->utf8_state = utf8_state;
                  }
                  application_data_offset += block_data_length;
                  block_offset += block_data_length;
               }
               pread_state->payload_length -= block_data_length;

               if (pread_state->payload_length == 0) {
                  int message_type = MG_WS_MESSAGE_TYPE_INVALID;

                  switch (pread_state->opcode) {
                     case MG_WS_OPCODE_TEXT:
                        if ((pread_state->fin && (pread_state->frame->utf8_state != UTF8_VALID)) || (pread_state->frame->utf8_state == UTF8_INVALID)) {
                           pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                           pread_state->status_code = MG_WS_STATUS_CODE_INVALID_UTF8;
                        }
                        else {
                           message_type = MG_WS_MESSAGE_TYPE_TEXT;
                        }
                        break;
                     case MG_WS_OPCODE_BINARY:
                        message_type = MG_WS_MESSAGE_TYPE_BINARY;
                        break;
                     case MG_WS_OPCODE_CLOSE:
                        pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                        pread_state->status_code = MG_WS_STATUS_CODE_OK;
                        break;
                     case MG_WS_OPCODE_PING:
                        mg_websocket_queue_block(pweb, MG_WS_MESSAGE_TYPE_PONG, (unsigned char *) application_data, (size_t) application_data_offset, 0);
                        break;
                     case MG_WS_OPCODE_PONG:
                        break;
                     default:
                        pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
                        pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
                        close_reason = 7;
                        break;
                  }
                  if (pread_state->fin && (message_type != MG_WS_MESSAGE_TYPE_INVALID) && (application_data_offset > 0)) {
/*
                     {
                        char bufferx[256];
                        sprintf(bufferx, "mg_websocket_incoming_frame: client data: application_data_offset=%llu;", application_data_offset);
                        mg_log_buffer(pweb->plog, pweb, (char *)  application_data, (int) application_data_offset, bufferx, 0);
                     }
*/
                     netx_tcp_write(pweb->pcon, (unsigned char *) application_data, (int) application_data_offset);
                  }
                  if (pread_state->framing_state != MG_WS_DATA_FRAMING_CLOSE) {
                     pread_state->framing_state = MG_WS_DATA_FRAMING_START;

                     if (pread_state->fin) {
                        if (pread_state->frame->application_data != NULL) {
                           free(pread_state->frame->application_data);
                           pread_state->frame->application_data = NULL;
                        }
                        application_data_offset = 0;
                     }
                  }
               }
               pread_state->frame->application_data_offset = application_data_offset;
            }
            break;
         case MG_WS_DATA_FRAMING_CLOSE:
            block_offset = block_size;
            break;
         default:
            pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
            pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
            close_reason = 8;
            break;
      }
   }

   if (pweb->plog->log_frames && close_reason) {
      char bufferx[256];
      sprintf(bufferx, "Closing reason code: %d;", close_reason);
      mg_log_event(pweb->plog, pweb, (char *)  bufferx, "mg_web: mg_websocket_incoming_frame: WebSocket closing", 0);
   }

   return;
}


DBX_THR_TYPE mg_websocket_dbserver_read(void *arg)
{
   int len, timeout, type;
#if defined(_WIN32)
   size_t rc;
#endif
   size_t size;
   unsigned char data[4096];
   MGWEB *pweb;

   pweb = (MGWEB *) arg;
   timeout = 60000;

#if defined(_WIN32)
   rc = 0;
#endif

   for (;;) {
      size = sizeof(data) - 1;
      len = netx_tcp_read(pweb->pcon, (unsigned char *) data, (int) size, timeout, 0);
/*
{
   char bufferx[256];
   sprintf(bufferx, "websocket netx_tcp_read len=%d;", len);
   mg_log_buffer(pweb->plog, pweb, data, len > 0 ? len : 0, bufferx, 0);
}
*/
      if (len == NETX_READ_TIMEOUT) {
         continue;
      }
      if (len == NETX_READ_EOF) {
         mg_websocket_write_block(pweb, MG_WS_MESSAGE_TYPE_CLOSE, (unsigned char *) "", 0);
         break;
      }

      if (len == NETX_READ_ERROR) {
         pweb->pwsock->status = MG_WEBSOCKET_CLOSED_BYSERVER;
         mg_websocket_write_block(pweb, MG_WS_MESSAGE_TYPE_CLOSE, (unsigned char *) "", 0);
         break;
      }

      if (len > 0) {

         if (pweb->pwsock->binary)
            type = MG_WS_MESSAGE_TYPE_BINARY;
         else
            type = MG_WS_MESSAGE_TYPE_TEXT;
#if defined(_WIN32)
         rc = mg_websocket_write_block(pweb, type, (unsigned char *) data, (size_t) len);
#else
/*
{
   char bufferx[256];
   sprintf(bufferx, "websocket netx_tcp_read SEND TO CLIENT len=%d; pweb->pwsock->binary=%d", len, pweb->pwsock->binary);
   mg_log_buffer(pweb->plog, pweb, data, len > 0 ? len : 0, bufferx, 0);
}
*/
         mg_websocket_write_block(pweb, type, (unsigned char *) data, (size_t) len);
#endif
      }
   }

   return DBX_THR_RETURN;
}


size_t mg_websocket_create_header(MGWEB *pweb, int type, unsigned char *header, mg_uint64_t payload_length)
{
   size_t pos;
   unsigned char opcode;

   pos = 0;

   switch (type) {
      case MG_WS_MESSAGE_TYPE_TEXT:
         opcode = MG_WS_OPCODE_TEXT;
         break;
      case MG_WS_MESSAGE_TYPE_BINARY:
         opcode = MG_WS_OPCODE_BINARY;
         break;
      case MG_WS_MESSAGE_TYPE_PING:
         opcode = MG_WS_OPCODE_PING;
         break;
      case MG_WS_MESSAGE_TYPE_PONG:
         opcode = MG_WS_OPCODE_PONG;
         break;
      case MG_WS_MESSAGE_TYPE_CLOSE:
         pweb->pwsock->closing = 1;
         opcode = MG_WS_OPCODE_CLOSE;
         break;
      default:
         pweb->pwsock->closing = 1;
         opcode = MG_WS_OPCODE_CLOSE;
         break;
   }
   header[pos ++] = MG_WS_FRAME_SET_FIN(1) | MG_WS_FRAME_SET_OPCODE(opcode);
   if (payload_length < 126) {
      header[pos ++] = MG_WS_FRAME_SET_MASK(0) | MG_WS_FRAME_SET_LENGTH(payload_length, 0);
   }
   else {
      if (payload_length < 65536) {
         header[pos ++] = MG_WS_FRAME_SET_MASK(0) | 126;
      }
      else {
         header[pos ++] = MG_WS_FRAME_SET_MASK(0) | 127;
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 7);
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 6);
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 5);
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 4);
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 3);
         header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 2);
      }
      header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 1);
      header[pos ++] = MG_WS_FRAME_SET_LENGTH(payload_length, 0);
   }

   return pos;
}


char mg_b64_ntc(unsigned char n)
{
   if (n < 26)
      return 'A' + n;
   if (n < 52)
      return 'a' - 26 + n;

   if (n < 62)
      return '0' - 52 + n;
   if (n == 62)
      return '+';

   return '/';
}


unsigned char mg_b64_ctn(char c)
{

   if (c == '/')
      return 63;
   if (c == '+')
      return 62;
   if ((c >= 'A') && (c <= 'Z'))
      return c - 'A';
   if ((c >= 'a') && (c <= 'z'))
      return c - 'a' + 26;
   if ((c >= '0') && (c <= '9'))
      return c - '0' + 52;
   if (c == '=')
      return 80;
   return 100;
}


int mg_b64_encode(char *from, char *to, int length, int quads)
{
/*
   3 8bit numbers become four characters
*/

   int i = 0;
   char *tot = to;
   int qc = 0; /* Quadcount */
   unsigned char c;
   unsigned char d;

   while (i < length) {
      c = from[i];
      *to++ = (char) mg_b64_ntc((unsigned char) (c / 4));
      c = c * 64;
     
      i++;

      if (i >= length) {
         *to++ = mg_b64_ntc((unsigned char) (c / 4));
         *to++ = '=';
         *to++ = '=';
         break;
      }
      d = from[i];
      *to++ = mg_b64_ntc((unsigned char) (c / 4 + d / 16));
      d = d * 16;

      i++;


      if (i >= length) {
         *to++ = mg_b64_ntc((unsigned char) (d / 4));
         *to++ = '=';
         break;
      }
      c = from[i];
      *to++ = mg_b64_ntc((unsigned char) (d / 4 + c / 64));
      c=c * 4;

      i++;

      *to++ = mg_b64_ntc((unsigned char) (c / 4));

      qc ++; /* qz will never be zero, quads = 0 means no linebreaks */
      if (qc == quads) {
         *to++ = '\n';
         qc = 0;
      }
   }

/*
   if ((quads != 0) && (qc != 0))
      *to ++ = '\n';
*/

/* Insert last linebreak */

   return ((int) (to - tot));
}


int mg_b64_decode(char *from, char *to, int length)
{
   unsigned char c, d, e, f;
   char A, B, C;
   int i;
   int add;
   char *tot = to;

   for (i = 0; i + 3 < length;) {
      add = 0;
      A = B = C = 0;
      c = d = e = f = 100;

      while ((c == 100) && (i < length))
         c = mg_b64_ctn(from[i++]);
      while ((d == 100) && (i < length))
         d = mg_b64_ctn(from[i++]);
      while ((e == 100) && (i < length))
         e = mg_b64_ctn(from[i++]);
      while ((f == 100) && (i < length))
         f = mg_b64_ctn(from[i++]);

      if (f == 100)
         return -1; /* Not valid end */

      if (c < 64) {
         A += c * 4;
         if (d < 64) {
            A += d / 16;

            B += d * 16;

            if (e < 64) {
               B += e / 4;
               C += e * 64;

               if (f < 64) {
                  C += f;
                  to[2] = C;
                  add += 1;

               }
               to[1] = B;
               add += 1;

            }
            to[0] = A;
            add += 1;
         }
      }
      to += add;

      if (f == 80)
         return ((int) (to - tot)); /* end because '=' encountered */
   }
   return ((int) (to - tot));
}


int mg_b64_get_ebuffer_size(int l, int q)
{
   int ret;

   ret = (l / 3) * 4;
   if (l % 3 != 0)
      ret += 4;
   if (q != 0) {
      ret += (ret / (q * 4));

/* if (ret % (q / 4) != 0)
      ret ++;
*/

/* Add space for trailing \n */
   }
   return ret;
}


int mg_b64_strip_ebuffer(char *buf, int length)
{
   int i;
   int ret = 0;

   for (i = 0;i < length;i ++)
      if (mg_b64_ctn(buf[i]) != 100)
         buf[ret++] = buf[i];
 
   return ret;

}


void mg_sha1_compile(sha1_ctx ctx[1])
{
   sha1_32t *w = ctx->wbuf;

#ifdef ARRAY
   sha1_32t    v[5];
   T_MEMCPY(v, ctx->hash, 5 * sizeof(sha1_32t));
#else
   sha1_32t    v0, v1, v2, v3, v4;
   v0 = ctx->hash[0]; v1 = ctx->hash[1];
   v2 = ctx->hash[2]; v3 = ctx->hash[3];
   v4 = ctx->hash[4];
#endif

#define hf(i)   w[i]

   five_cycle(v, ch, 0x5a827999,  0);
   five_cycle(v, ch, 0x5a827999,  5);
   five_cycle(v, ch, 0x5a827999, 10);
   one_cycle(v,0,1,2,3,4, ch, 0x5a827999, hf(15)); \

#undef  hf
#define hf(i) (w[(i) & 15] = rotl32(                    \
                 w[((i) + 13) & 15] ^ w[((i) + 8) & 15] \
               ^ w[((i) +  2) & 15] ^ w[(i) & 15], 1))

   one_cycle(v,4,0,1,2,3, ch, 0x5a827999, hf(16));
   one_cycle(v,3,4,0,1,2, ch, 0x5a827999, hf(17));
   one_cycle(v,2,3,4,0,1, ch, 0x5a827999, hf(18));
   one_cycle(v,1,2,3,4,0, ch, 0x5a827999, hf(19));

   five_cycle(v, parity, 0x6ed9eba1,  20);
   five_cycle(v, parity, 0x6ed9eba1,  25);
   five_cycle(v, parity, 0x6ed9eba1,  30);
   five_cycle(v, parity, 0x6ed9eba1,  35);

   five_cycle(v, maj, 0x8f1bbcdc,  40);
   five_cycle(v, maj, 0x8f1bbcdc,  45);
   five_cycle(v, maj, 0x8f1bbcdc,  50);
   five_cycle(v, maj, 0x8f1bbcdc,  55);

   five_cycle(v, parity, 0xca62c1d6,  60);
   five_cycle(v, parity, 0xca62c1d6,  65);
   five_cycle(v, parity, 0xca62c1d6,  70);
   five_cycle(v, parity, 0xca62c1d6,  75);

#ifdef ARRAY
   ctx->hash[0] += v[0]; ctx->hash[1] += v[1];
   ctx->hash[2] += v[2]; ctx->hash[3] += v[3];
   ctx->hash[4] += v[4];
#else
   ctx->hash[0] += v0; ctx->hash[1] += v1;
   ctx->hash[2] += v2; ctx->hash[3] += v3;
   ctx->hash[4] += v4;
#endif
}



void mg_sha1_begin(sha1_ctx ctx[1])
{
   ctx->count[0] = ctx->count[1] = 0;
   ctx->hash[0] = 0x67452301;
   ctx->hash[1] = 0xefcdab89;
   ctx->hash[2] = 0x98badcfe;
   ctx->hash[3] = 0x10325476;
   ctx->hash[4] = 0xc3d2e1f0;
}



/* SHA1 hash data in an array of bytes into hash buffer and */
/* call the hash_compile function as required.              */

void mg_sha1_hash(const unsigned char data[], unsigned long len, sha1_ctx ctx[1])
{
   sha1_32t pos = (sha1_32t)(ctx->count[0] & SHA1_MASK), space = SHA1_BLOCK_SIZE - pos;
   const unsigned char *sp = data;

   if ((ctx->count[0] += len) < len)
      ++(ctx->count[1]);

   /* transfer whole blocks if possible  */
   while (len >= space) {
      memcpy(((unsigned char*)ctx->wbuf) + pos, sp, space);
      sp += space; len -= space; space = SHA1_BLOCK_SIZE; pos = 0;
      bsw_32(ctx->wbuf, SHA1_BLOCK_SIZE >> 2);
      mg_sha1_compile(ctx);
   }

   memcpy(((unsigned char*)ctx->wbuf) + pos, sp, len);
}


/* SHA1 final padding and digest calculation  */

void mg_sha1_end(unsigned char hval[], sha1_ctx ctx[1])
{
   sha1_32t i = (sha1_32t)(ctx->count[0] & SHA1_MASK);

   /* put bytes in the buffer in an order in which references to   */
   /* 32-bit words will put bytes with lower addresses into the    */
   /* top of 32 bit words on BOTH big and little endian machines   */
   bsw_32(ctx->wbuf, (i + 3) >> 2);

   /* we now need to mask valid bytes and add the padding which is */
   /* a single 1 bit and as many zero bits as necessary. Note that */
   /* we can always add the first padding byte here because the    */
   /* buffer always has at least one empty slot                    */
   ctx->wbuf[i >> 2] &= 0xffffff80 << 8 * (~i & 3);
   ctx->wbuf[i >> 2] |= 0x00000080 << 8 * (~i & 3);

   /* we need 9 or more empty positions, one for the padding byte  */
   /* (above) and eight for the length count. If there is not      */
   /* enough space, pad and empty the buffer                       */
   if (i > SHA1_BLOCK_SIZE - 9) {
      if (i < 60)
         ctx->wbuf[15] = 0;
      mg_sha1_compile(ctx);
      i = 0;
   }
   else    /* compute a word index for the empty buffer positions  */
      i = (i >> 2) + 1;

   while (i < 14) /* and zero pad all but last two positions        */
      ctx->wbuf[i++] = 0;

   /* the following 32-bit length fields are assembled in the      */
   /* wrong byte order on little endian machines but this is       */
   /* corrected later since they are only ever used as 32-bit      */
   /* word values.                                                 */
   ctx->wbuf[14] = (ctx->count[1] << 3) | (ctx->count[0] >> 29);
   ctx->wbuf[15] = ctx->count[0] << 3;
   mg_sha1_compile(ctx);

   /* extract the hash value as bytes in case the hash buffer is   */
   /* misaligned for 32-bit words                                  */
   for(i = 0; i < SHA1_DIGEST_SIZE; ++i)
      hval[i] = (unsigned char)(ctx->hash[i >> 2] >> (8 * (~i & 3)));

   return;
}



void mg_sha1(unsigned char hval[], const unsigned char data[], unsigned long len)
{
   sha1_ctx cx[1];

   mg_sha1_begin(cx);
   mg_sha1_hash(data, len, cx);
   mg_sha1_end(hval, cx);
   return;
}


