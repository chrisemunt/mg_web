/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2021 M/Gateway Developments Ltd,                      |
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
#include "mg_webstatus.h"

/* v2.4.24 */

int mg_admin(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, len, scriptlen, json;
   unsigned long size;
   char *pn, *pv, *pz;
   char script[256], buffer[256], op[32], subop[32], server[64], value[64], info[256];
   MGSRV *psrv;
   MGADM adm;
   FILE *fp;

#ifdef _WIN32
__try {
#endif

   *info = '\0';
   json = 1;
   pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr;
   adm.chunkno = 0;
   if (pweb->wserver_chunks_response == 0)
      adm.chunking_allowed = 1;
   else
      adm.chunking_allowed = 0;

   *op = '\0';
   *subop = '\0';
   *server = '\0';
   *value = '\0';

   pweb->request_content = (char *) pweb->output_val.svalue.buf_addr;
   pweb->request_content[0] = '\0';
   if (pweb->request_clen > 0) {
      mg_client_read(pweb, (unsigned char *) pweb->request_content, pweb->request_clen);
      pweb->request_content[pweb->request_clen] = '\0';
      mg_lcase(pweb->request_content);
/*
      mg_log_event(&(mg_system.log), NULL, pweb->request_content, "mg_admin: content", 0);
*/
   }

   scriptlen = 200;
   *script = '\0';
   rc = mg_get_cgi_variable(pweb, "SCRIPT_NAME", script, &scriptlen);
   if (rc != MG_CGI_SUCCESS || scriptlen < 1) {
      return mg_web_simple_response(pweb, NULL, "bad command", json);
   }
   script[scriptlen] = '\0';
   if (script[scriptlen - 1] != '/') {
      script[scriptlen ++] = '/';
      script[scriptlen] = '\0';
   }
/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_admin: script_name: size=%d", scriptlen);
      mg_log_event(&(mg_system.log), NULL, script, bufferx, 0);
   }
*/
   len = 200;
   *buffer = '\0';
   rc = mg_get_cgi_variable(pweb, "PATH_INFO", buffer, &len);
   if (rc == MG_CGI_SUCCESS && len > 0 && len < 64) {
      buffer[len] = '\0';
/*
      {
         char bufferx[256];
         sprintf(bufferx, "mg_admin: path_info: size=%d", len);
         mg_log_event(&(mg_system.log), NULL, buffer, bufferx, 0);
      }
*/
      if (buffer[0] == '/')
         strcat(script, buffer + 1);
      else
         strcat(script, buffer + 1);
      scriptlen = (int) strlen(script);
      if (script[scriptlen - 1] != '/') {
         script[scriptlen ++] = '/';
         script[scriptlen] = '\0';
      }
   }
/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_admin: script_name (pre-process): size=%d", scriptlen);
      mg_log_event(&(mg_system.log), NULL, script, bufferx, 0);
   }
*/
   if (!strcmp(script + (scriptlen - 8), "/status/")) {
      strcpy(op, "status");
   }
   else if (!strcmp(script + (scriptlen - 11), "/conf/list/")) {
      strcpy(op, "conf");
      strcpy(subop, "list");
   }
   else if (!strcmp(script + (scriptlen - 10), "/log/list/")) {
      strcpy(op, "log");
      strcpy(subop, "list");
   }
   else if (!strcmp(script + (scriptlen - 10), "/log/size/")) {
      strcpy(op, "log");
      strcpy(subop, "size");
   }
   else if (!strcmp(script + (scriptlen - 8), "/log/op/")) {
      strcpy(op, "log");
      if (pweb->request_clen > 0) {
         mg_get_value(pweb->request_content, "op", subop);
      }
   }
   else if (!strcmp(script + (scriptlen - 8), "/update/")) {
      if (pweb->request_clen > 0) {
         mg_get_value(pweb->request_content, "log_level", value);
         if (value[0]) {
            strcpy(op, "log_level");
         }
      }
   }
   else if (!strcmp(script + (scriptlen - 15), "/update/server/")) {
      if (pweb->request_clen > 0) {
         mg_get_value(pweb->request_content, "server", server);
         mg_get_value(pweb->request_content, "offline", value);
      }
      if (!strcmp(value, "0"))
         strcpy(op, "online");
      else if (!strcmp(value, "1"))
         strcpy(op, "offline");
   }

/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_admin: op=%s; subop=%s; server=%s; value=%s", op, subop, server, value);
      mg_log_event(&(mg_system.log), NULL, bufferx, "mg_admin", 0);
   }
*/

/*
   len = 200;
   *buffer = '\0';
   rc = mg_get_cgi_variable(pweb, "QUERY_STRING", buffer, &len);
   if (rc == MG_CGI_UNDEFINED)
      len = 0;
   if (len < 0 || len > 200) {
      pweb->response_clen = 0;
      mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_NONE);
      mg_submit_headers(pweb);
      return CACHE_SUCCESS;
   }
*/
   len = 0;
   buffer[len] = '\0';
   mg_lcase(buffer);
   pn = buffer;
   while (1) {
      pz = strstr(pn, "&");
      if (pz)
         *pz = '\0';
      pv = strstr(pn, "=");
      if (pv) {
         *pv = '\0';
         pv ++;
         if (!strcmp(pn, "op")) {
            strncpy(op, pv, 30);
            op[30] = '\0';
            mg_lcase(op);
         }
         else if (!strcmp(pn, "subop")) {
            strncpy(subop, pv, 30);
            subop[30] = '\0';
            mg_lcase(subop);
         }
         else if (!strcmp(pn, "server")) {
            strncpy(server, pv, 60);
            server[30] = '\0';
            mg_lcase(server);
         }
         else if (!strcmp(pn, "value")) {
            strncpy(value, pv, 60);
            value[30] = '\0';
            mg_lcase(value);
         }
      }
      if (!pz)
         break;
      pn = (pz + 1);
   }

   if (!strcmp(op, "status")) {
      return mg_status(pweb, &adm, json);
   }
   else if (!strcmp(op, "conf")) {
      adm.filename = mg_system.config_file;
      return mg_get_file(pweb, &adm, 0);
   }
   else if (!strcmp(op, "log")) {
      adm.filename = mg_system.log.log_file;
      if (!strcmp(subop, "list")) {
         return mg_get_file(pweb, &adm, 0);
      }
      else if (!strcmp(subop, "size")) {
         fp = fopen(adm.filename, "r");
         if (fp) {
            fseek(fp, 0, SEEK_END);
            size = (unsigned int) ftell(fp);
            if (size < 1)
               size = 0;
            fclose(fp);
         }
         else
            size = 0;
         sprintf(buffer, "%lu", size);
         return mg_web_simple_response(pweb, buffer, NULL, json);
      }
      else if (!strcmp(subop, "clear")) {
         fp = fopen(adm.filename, "w");
         if (fp) {
            fclose(fp);
            return mg_web_simple_response(pweb, "success", NULL, json);
         }
         else {
            return mg_web_simple_response(pweb, NULL, "log file not cleared", json);
         }
      }
      else {
         return mg_web_simple_response(pweb, NULL, "bad operation", json);
      }
   }
   else if ((!strcmp(op, "online") || !strcmp(op, "offline")) && server[0]) {
      psrv = mg_server;
      while (psrv) {
         if (!strcmp(psrv->lcname, server)) {
            if (!strcmp(op, "online"))
               mg_server_online(pweb, psrv, info, 10);
            else
               mg_server_offline(pweb, psrv, info, 10);
            if (info[0]) {
               mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
            }
            break;
         }
         psrv = psrv->pnext;
      }
      if (psrv)
         return mg_web_simple_response(pweb, "success", NULL, json);
      else
         return mg_web_simple_response(pweb, NULL, "bad server", json);
   }
   else if (!strcmp(op, "log_level") && value[0]) {
      mg_set_log_level(&(mg_system.log), value, 1);
      return mg_web_simple_response(pweb, "success", NULL, json);
   }
   else {
      return mg_web_simple_response(pweb, NULL, "bad command", json);
   }

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_admin: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_status(MGWEB *pweb, MGADM *padm, int context)
{
   DBX_TRACE_INIT(0)
   int n, sn, json;
   unsigned long no_requests, no_connections;
   char buffer[256], info[256];
   MGPATH *ppath;
   MGSRV *psrv;
   DBXCON *pcon;
   time_t time_now;

#ifdef _WIN32
__try {
#endif

   *info = '\0';
   json = context;
   time_now = time(NULL);

   pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr;
   if (json)
      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n");
   else
      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n");
   pweb->response_headers_len = (int) strlen(pweb->response_headers);

   pweb->response_content = (char *) pweb->output_val.svalue.buf_addr + (pweb->response_headers_len + 64);
   padm->len_alloc = pweb->output_val.svalue.len_alloc - (pweb->response_headers_len + 64);
   padm->buf_addr = pweb->response_content;
   padm->len_used = 0;

   if (json)
      sprintf(buffer, "{\r\n\"mg_web\": {\r\n   \"version\": \"%s.%s.%s\",\r\n   \"request_timeout\": %d,\r\n   \"no_requests\": %lu,\r\n", (char *) DBX_VERSION_MAJOR, (char *) DBX_VERSION_MINOR, (char *) DBX_VERSION_BUILD, mg_system.timeout, mg_system.requestno);
   else
      sprintf(buffer, "mg_web\r\n   Version: %s.%s.%s\r\n   Request-Timeout: %d\r\n   No-Requests: %lu\r\n", (char *) DBX_VERSION_MAJOR, (char *) DBX_VERSION_MINOR, (char *) DBX_VERSION_BUILD, mg_system.timeout, mg_system.requestno);
   mg_status_add(pweb, padm, buffer, 0, 0);

   if (json)
      strcpy(buffer, "   \"log_level\": \"");
   else
      strcpy(buffer, "   Log-Level: ");
   if (mg_system.log.log_errors)
      strcat(buffer, "e");
   if (mg_system.log.log_verbose)
      strcat(buffer, "v");
   if (mg_system.log.log_frames)
      strcat(buffer, "f");
   if (mg_system.log.log_transmissions)
      strcat(buffer, "t");
   if (mg_system.log.log_transmissions_to_webserver)
      strcat(buffer, "w");
   if (mg_system.log.log_tls == 1)
      strcat(buffer, "s");
   if (mg_system.log.log_tls == 2)
      strcat(buffer, "S");
   if (json)
      strcat(buffer, "\"\r\n},\r\n\"locations\": [\r\n");
   else
      strcat(buffer, "\r\n");
   mg_status_add(pweb, padm, buffer, 0, 0);

   sn = 0;
   ppath = mg_path;
   while (ppath) {
      sn ++;
      if (ppath->admin) {
         if (json)
            sprintf(buffer, "%s   {\r\n      \"location\": \"%s\",\r\n      \"administrator\": \"on\"\r\n   }", sn > 1 ? ",\r\n" : "", ppath->name);
         else
            sprintf(buffer, "Location: %s\r\n   Administrator Functions\r\n", ppath->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
      }
      else {
         if (json)
            sprintf(buffer, "%s   {\r\n      \"location\": \"%s\",\r\n", sn > 1 ? ",\r\n" : "", ppath->name);
         else
            sprintf(buffer, "Location: %s\r\n", ppath->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
 
         if (json) {
            sprintf(buffer, "      \"function\": \"%s\",\r\n      \"load_balancing\": \"%s\",\r\n      \"server_affinity_precedence\": \"%s\",\r\n      \"server_sffinity_cookie\": \"%s\",\r\n", 
                     ppath->function ? ppath->function : "", ppath->load_balancing ? "on" : "off",
                     ppath->sa_order == 1 ? "variable" : ppath->sa_order == 2 ? "cookie" : "none",
                     ppath->sa_cookie ? ppath->sa_cookie : "");
            mg_status_add(pweb, padm, buffer, 0, 0);

            if (ppath->sa_variables[0]) {
               strcpy(buffer, "      \"server_affinity_variables\": [");
               for (n = 0; ppath->sa_variables[n]; n ++) {
                  if (n)
                     strcat(buffer, ", ");
                  strcat(buffer, "\"");
                  strcat(buffer, ppath->sa_variables[n]);
                  strcat(buffer, "\"");
               }
               strcat(buffer, "],\r\n");
               mg_status_add(pweb, padm, buffer, 0, 0);
            }
            strcpy(buffer, "      \"servers\": [\r\n");
            mg_status_add(pweb, padm, buffer, 0, 0);
            for (n = 0; ppath->servers[n].name; n ++) {
               sprintf(buffer, "%s         {\"number\": %d, \"server\": \"%s\", \"exclusive\": %d, \"offline\": %d}", n > 0 ? ",\r\n" : "", n, ppath->servers[n].name, ppath->servers[n].exclusive, ppath->servers[n].psrv->offline);
               mg_status_add(pweb, padm, buffer, 0, 0);
            }
            mg_status_add(pweb, padm, "\r\n      ]\r\n   }", 0, 0);
         }
         else {
            sprintf(buffer, "   Function: %s\r\n   Load-Balancing: %s\r\n   Server-Affinity-Precedence: %s\r\n   Server-Affinity-Cookie: %s\r\n", 
                     ppath->function ? ppath->function : "null", ppath->load_balancing ? "on" : "off",
                     ppath->sa_order == 1 ? "Variable" : ppath->sa_order == 2 ? "Cookie" : "None",
                     ppath->sa_cookie ? ppath->sa_cookie : "null");
            mg_status_add(pweb, padm, buffer, 0, 0);

            if (ppath->sa_variables[0]) {
               strcpy(buffer, "   Server-Affinity-Variable:");
               for (n = 0; ppath->sa_variables[n]; n ++) {
                  strcat(buffer, " ");
                  strcat(buffer, ppath->sa_variables[n]);
               }
               mg_status_add(pweb, padm, buffer, 0, 0);
               mg_status_add(pweb, padm, "\r\n", 2, 0);
            }
            for (n = 0; ppath->servers[n].name; n ++) {
               sprintf(buffer, "   Server-%d: %s%s%s\r\n", n, ppath->servers[n].name, ppath->servers[n].exclusive ? " Exclusive" : "", ppath->servers[n].psrv->offline ? " Offline" : "");
               mg_status_add(pweb, padm, buffer, 0, 0);
            }
         }
      }
      ppath = ppath->pnext;
   }
   if (json) {
      strcpy(buffer, "\r\n],\r\n\"servers\": [\r\n");
      mg_status_add(pweb, padm, buffer, 0, 0);
   }

   sn = 0;
   psrv = mg_server;
   while (psrv) {
      sn ++;
      if (json) {
         sprintf(buffer, "%s   {\r\n      \"server\": \"%s\",\r\n", sn > 1 ? ",\r\n" : "", psrv->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
 
         sprintf(buffer, "      \"type\": \"%s\",\r\n", psrv->dbtype == DBX_DBTYPE_IRIS ? "InterSystems IRIS" : psrv->dbtype == DBX_DBTYPE_CACHE ? "InterSystems IRIS" : psrv->dbtype == DBX_DBTYPE_YOTTADB ? "YottaDB" : "None");
         mg_status_add(pweb, padm, buffer, 0, 0);

         if (psrv->net_connection) {
            sprintf(buffer, "      \"host\": \"%s:%d\",\r\n      \"nagle_algorithm\": \"%s\",\r\n      \"tls\": \"%s\",\r\n      \"namespace\": \"%s\",\r\n      \"health_check\": %d\r\n   }", psrv->ip_address ? psrv->ip_address : "", psrv->port, psrv->nagle_algorithm ? "on" : "off", psrv->tls_name ? psrv->tls_name : "", psrv->uci ? psrv->uci : "", psrv->health_check);
         }
         else {
            sprintf(buffer, "      \"path\": \"%s\",\r\n      \"namespace\": \"%s\",\r\n      \"health_check\": %d\r\n   }", psrv->shdir ? psrv->shdir : "", psrv->uci ? psrv->uci : "", psrv->health_check);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);
      }
      else {
         sprintf(buffer, "Server: %s\r\n", psrv->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
 
         sprintf(buffer, "   Type: %s\r\n", psrv->dbtype == DBX_DBTYPE_IRIS ? "InterSystems IRIS" : psrv->dbtype == DBX_DBTYPE_CACHE ? "InterSystems IRIS" : psrv->dbtype == DBX_DBTYPE_YOTTADB ? "YottaDB" : "None");
         mg_status_add(pweb, padm, buffer, 0, 0);

         if (psrv->net_connection) {
            sprintf(buffer, "   Host: %s:%d\r\n   Nagle-Algorithm: %s\r\n   TLS: %s\r\n   Namespace: %s\r\n   Health-Check: %d\r\n", psrv->ip_address ? psrv->ip_address : "null", psrv->port, psrv->nagle_algorithm ? "on" : "off", psrv->tls_name ? psrv->tls_name : "null", psrv->uci ? psrv->uci : "null", psrv->health_check);
         }
         else {
            sprintf(buffer, "   Path: %s\r\n   Namespace: %s\r\n   Health-Check: %d\r\n", psrv->shdir ? psrv->shdir : "null", psrv->uci ? psrv->uci : "null", psrv->health_check);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);
      }
      psrv = psrv->pnext;
   }
   if (json) {
      strcpy(buffer, "\r\n],\r\n\"worker_processes\": [\r\n");
      mg_status_add(pweb, padm, buffer, 0, 0);
      sprintf(buffer, "   {\r\n      \"process_id\": %lu,\r\n      \"servers\": [\r\n", mg_current_process_id());
      mg_status_add(pweb, padm, buffer, 0, 0);
   }
   else {
      sprintf(buffer, "Worker-Process: %lu\r\n", mg_current_process_id());
      mg_status_add(pweb, padm, buffer, 0, 0);
   }
   sn = 0;
   psrv = mg_server;
   while (psrv) {
      sn ++;
      no_connections = 0;
      no_requests = 0;

      if (psrv->offline == 1 && psrv->time_offline && psrv->health_check > 0) { /* check to see if offline server is ready for a heath-check */
         if ((int) difftime(time_now, psrv->time_offline) > psrv->health_check) {
            mg_server_online(pweb, psrv, info, 10);
            if (info[0]) {
               mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
            }
         }
      }

      pcon = mg_connection;
      while (pcon) {
         if (pcon->psrv == psrv) {
            no_connections ++;
            no_requests += psrv->no_requests;
         }
         pcon = pcon->pnext;
      }
      if (json) {
         sprintf(buffer, "%s         {\r\n            \"server\": \"%s\",\r\n", sn > 1 ? ",\r\n" : "", psrv->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
         if (no_connections || psrv->offline == 1) {
            sprintf(buffer, "            \"status\": \"%s\",\r\n            \"no_connections\": %lu,\r\n            \"no_requests\": %lu,\r\n", psrv->offline ? "offline" : "online", no_connections, no_requests);
         }
         else {
            sprintf(buffer, "            \"status\": \"unknown\",\r\n            \"no_connections\": %lu,\r\n            \"no_requests\": %lu,\r\n", no_connections, no_requests);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);
         if (psrv->offline == 1) {
            if (psrv->time_offline) {
               if (psrv->health_check > 0)
                  sprintf(buffer, "            \"time_offline\": \"%d/%d\"\r\n         }", (int) difftime(time_now, psrv->time_offline), psrv->health_check);
               else
                  sprintf(buffer, "            \"time_offline\": %d\r\n         }", (int) difftime(time_now, psrv->time_offline));
            }
            else {
               sprintf(buffer, "            \"time_offline\": %d\r\n         }", (int) 0);
            }
         }
         else {
            sprintf(buffer, "            \"time_offline\": %d\r\n         }", (int) 0);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);
      }
      else {
         sprintf(buffer, "   Server: %s\r\n", psrv->name);
         mg_status_add(pweb, padm, buffer, 0, 0);
         if (no_connections || psrv->offline == 1) {
            sprintf(buffer, "      Status: %s\r\n      No-Connections: %lu\r\n      No-Requests: %lu\r\n", psrv->offline ? "Offline" : "Online", no_connections, no_requests);
         }
         else {
            sprintf(buffer, "      Status: Unknown\r\n      No-Connections: %lu\r\n      No-Requests: %lu\r\n", no_connections, no_requests);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);

         if (psrv->offline == 1) {
            if (psrv->time_offline) {
               if (psrv->health_check > 0)
                  sprintf(buffer, "      Time-Offline: %d/%d\r\n", (int) difftime(time_now, psrv->time_offline), psrv->health_check);
               else
                  sprintf(buffer, "      Time-Offline: %d\r\n", (int) difftime(time_now, psrv->time_offline));
            }
         }
         else {
            sprintf(buffer, "      Time-Offline: %d\r\n", (int) 0);
         }
         mg_status_add(pweb, padm, buffer, 0, 0);
      }

      psrv = psrv->pnext;
   }
   if (json) {
      strcpy(buffer, "\r\n      ]\r\n   }\r\n]\r\n}\r\n");
      mg_status_add(pweb, padm, buffer, 0, 0);
   }

   mg_status_add(pweb, padm, "", 0, 0); /* terminate response */
/*
   pweb->response_clen = (int) strlen(pweb->response_content);

   sprintf(buffer, "Content-Length: %d\r\n\r\n", pweb->response_clen);
   strcat(pweb->response_headers, buffer);
   pweb->response_headers_len = (int) strlen(pweb->response_headers);

   mg_submit_headers(pweb);
   mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen);
*/
   return CACHE_SUCCESS;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_status: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}

int mg_get_file(MGWEB *pweb, MGADM *padm, int context)
{
   DBX_TRACE_INIT(0)
   int rc, len;
   char buffer[256];
   FILE *fp;

#ifdef _WIN32
__try {
#endif

   pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr;
   strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n");
   pweb->response_headers_len = (int) strlen(pweb->response_headers);

   pweb->response_content = (char *) pweb->output_val.svalue.buf_addr + (pweb->response_headers_len + 64);
   padm->len_alloc = pweb->output_val.svalue.len_alloc - (pweb->response_headers_len + 64);
   padm->buf_addr = pweb->response_content;
   padm->len_used = 0;

   strcpy(pweb->response_content, "no file");
   pweb->response_clen = (int) strlen(pweb->response_content);
   *buffer = '\0';

   fp = fopen(padm->filename, "r");
   if (fp) {
      pweb->response_clen = 0;
      while (1) {
         rc = (int) fread((void *) (pweb->response_content + pweb->response_clen), 1, (size_t) padm->len_alloc - (pweb->response_clen + 32), fp);
/*
         {
            char bufferx[256];
            sprintf(bufferx, "file=%s; rc=%d; size=%d; space_left=%d;", padm->filename, rc, pweb->response_clen, (padm->len_alloc - (pweb->response_clen + rc)));
            mg_log_event(&(mg_system.log), NULL, bufferx, "mg_get_file", 0);
         }
*/
         if (rc < 1) {
            break;
         }
         pweb->response_clen += (int) rc;
         if ((padm->len_alloc - pweb->response_clen) < 200) {
            padm->chunkno ++;
            if (padm->chunking_allowed) {
               if (padm->chunkno == 1) {
                  strcat(pweb->response_headers, "Transfer-Encoding: chunked\r\n\r\n");
                  pweb->response_headers_len = (int) strlen(pweb->response_headers);
                  mg_submit_headers(pweb);
                  sprintf(buffer, "%x\r\n", pweb->response_clen);
               }
               else {
                  sprintf(buffer, "\r\n%x\r\n", pweb->response_clen);
               }
            }
            else {
               strcat(pweb->response_headers, "\r\n");
               pweb->response_headers_len = (int) strlen(pweb->response_headers);
               mg_submit_headers(pweb);
               *buffer = '\0';
            }
            len = (int) strlen(buffer);
            padm->buf_addr = (pweb->response_content - len);
            padm->len_used = (pweb->response_clen + len);
            memcpy((void *) padm->buf_addr, (void *) buffer, (size_t) len);
            mg_client_write(pweb, (unsigned char *) padm->buf_addr, (int) padm->len_used);
            padm->buf_addr = pweb->response_content;
            padm->len_used = 0;
            pweb->response_clen = 0;
         }
/*
         {
            char bufferx[256];
            sprintf(bufferx, "file=%s; rc=%d; padm->buffer_size=%d; pweb->response_clen=%d; chunk=%s;", padm->filename, rc, padm->len_alloc, pweb->response_clen, buffer);
            mg_log_event(&(mg_system.log), NULL, bufferx, "mg_get_file", 0);
         }
*/
      }
      fclose(fp);
   }
/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_get_file: file=%s; rc=%d; pweb->response_headers_len=%d; padm->buffer_size=%d; pweb->response_clen=%d; chunkno=%d; chunk=%s;", padm->filename, rc, pweb->response_headers_len, padm->len_alloc, pweb->response_clen, padm->chunkno, buffer);
      mg_log_buffer(&(mg_system.log), NULL, pweb->response_headers, pweb->response_headers_len, bufferx, 0);
   }
*/
   if (padm->chunkno) {
      if (padm->chunking_allowed) {
         if (pweb->response_clen > 0) {
            sprintf(buffer, "\r\n%x\r\n", pweb->response_clen);
            len = (int) strlen(buffer);
            padm->buf_addr = (pweb->response_content - len);
            padm->len_used = (pweb->response_clen + len);
            memcpy((void *) padm->buf_addr, (void *) buffer, (size_t) len);
         }
         else {
            padm->buf_addr = pweb->response_content;
            padm->len_used = pweb->response_clen;
         }
         strcpy(padm->buf_addr + padm->len_used, "\r\n0\r\n\r\n");
         padm->len_used += 7;
      }
      else {
         padm->buf_addr = pweb->response_content;
         padm->len_used = pweb->response_clen;
      }
      if (padm->len_used) {
         mg_client_write(pweb, (unsigned char *) padm->buf_addr, (int) padm->len_used);
      }
   }
   else {
      sprintf(buffer, "Content-Length: %d\r\n\r\n", pweb->response_clen);
      strcat(pweb->response_headers, buffer);
      pweb->response_headers_len = (int) strlen(pweb->response_headers);

      mg_submit_headers(pweb);
      mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen);
   }

   return CACHE_SUCCESS;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_get_file: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_status_add(MGWEB *pweb, MGADM *padm, char *data, int data_len, int context)
{
   DBX_TRACE_INIT(0)
   int len;
   char buffer[256];

#ifdef _WIN32
__try {
#endif

   if (data_len == 0) {
      data_len = (int) strlen(data);
   }

   if (data_len == 0) { /* end of data */
      if (padm->chunkno) {
         if (padm->chunking_allowed) {
            if (pweb->response_clen > 0) {
               sprintf(buffer, "\r\n%x\r\n", pweb->response_clen);
               len = (int) strlen(buffer);
               padm->buf_addr = (pweb->response_content - len);
               padm->len_used = (pweb->response_clen + len);
               memcpy((void *) padm->buf_addr, (void *) buffer, (size_t) len);
            }
            else {
               padm->buf_addr = pweb->response_content;
               padm->len_used = pweb->response_clen;
            }
            strcpy(padm->buf_addr + padm->len_used, "\r\n0\r\n\r\n");
            padm->len_used += 7;
         }
         else {
            padm->buf_addr = pweb->response_content;
            padm->len_used = pweb->response_clen;
         }
         if (padm->len_used) {
            mg_client_write(pweb, (unsigned char *) padm->buf_addr, (int) padm->len_used);
         }
      }
      else {
         sprintf(buffer, "Content-Length: %d\r\n\r\n", pweb->response_clen);
         strcat(pweb->response_headers, buffer);
         pweb->response_headers_len = (int) strlen(pweb->response_headers);

         mg_submit_headers(pweb);
         mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen);
      }
      return CACHE_SUCCESS;
   }

   if (data_len < (padm->len_alloc - (pweb->response_clen + 32))) {
      memcpy((void *) (pweb->response_content + pweb->response_clen), (void *) data, (size_t) data_len);
      pweb->response_clen += data_len;
      return CACHE_SUCCESS;
   }

   pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr;

   strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n");
   pweb->response_headers_len = (int) strlen(pweb->response_headers);

   pweb->response_content = (char *) pweb->output_val.svalue.buf_addr + (pweb->response_headers_len + 64);
   padm->len_alloc = pweb->output_val.svalue.len_alloc - (pweb->response_headers_len + 64);
   padm->buf_addr = pweb->response_content;
   padm->len_used = 0;

   strcpy(pweb->response_content, "No File");
   pweb->response_clen = (int) strlen(pweb->response_content);
   *buffer = '\0';

   padm->chunkno ++;
   if (padm->chunking_allowed) {
      if (padm->chunkno == 1) {
         strcat(pweb->response_headers, "Transfer-Encoding: chunked\r\n\r\n");
         pweb->response_headers_len = (int) strlen(pweb->response_headers);
         mg_submit_headers(pweb);
         sprintf(buffer, "%x\r\n", pweb->response_clen);
      }
      else {
         sprintf(buffer, "\r\n%x\r\n", pweb->response_clen);
      }
   }
   else {
      strcat(pweb->response_headers, "\r\n");
      pweb->response_headers_len = (int) strlen(pweb->response_headers);
      mg_submit_headers(pweb);
      *buffer = '\0';
   }
   len = (int) strlen(buffer);
   padm->buf_addr = (pweb->response_content - len);
   padm->len_used = (pweb->response_clen + len);
   memcpy((void *) padm->buf_addr, (void *) buffer, (size_t) len);
   mg_client_write(pweb, (unsigned char *) padm->buf_addr, (int) padm->len_used);
   padm->buf_addr = pweb->response_content;
   padm->len_used = 0;
   pweb->response_clen = 0;

   memcpy((void *) (pweb->response_content + pweb->response_clen), (void *) data, (size_t) data_len);
   pweb->response_clen += data_len;

   return CACHE_SUCCESS;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_status_add: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_get_value(char *json, char *name, char *value)
{
   int len, namelen;
   char *p, *p1;

   len = 0;
   *value = '\0';
   namelen = (int) strlen(name);
   if (*json != '{') {
      return len;
   }
   p = strstr(json, name);
   if (p) {
      if (*(p - 1) != '\"' && *(p - 1) != '\'' && *(p + namelen) != '\"' && *(p + namelen) != '\'') {
         return len;
      }
      p1 = strstr(p, ":");
      if (p1) {
         p1 ++;
         while (*p1 == ' ')
            p1 ++;
         if (*p1 == '\"' || *p1 == '\'') { /* string value */
            p1 ++;
            while (*p1) {
               if (*p1 == '\"' || *p1 == '\'') {
                  break;
               }
               if (len > 30) {
                  break;
               }
               value[len ++] = *p1;
               p1 ++;
            }
         }
         else { /* numeric value */
            while (*p1) {
               if (*p1 == ',' || *p1 == ' ' || *p1 == '}') {
                  break;
               }
               if (len > 30) {
                  break;
               }
               value[len ++] = *p1;
               p1 ++;
            }
         }
      }
   }
   value[len] = '\0';
   return len;
}
