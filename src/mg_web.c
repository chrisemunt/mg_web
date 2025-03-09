/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2025 MGateway Ltd                                     |
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

/*
   Development Diary (in brief):

Version 1.0.1 21 July 2020:
   First release.

Version 1.0.2 31 July 2020:
   Improve the parsing and validation of the mg_web configuration file (mgweb.conf).

Version 1.0.3 1 August 2020:
   Correct a fault in the network connectivity code between mg_web and InterSystems databases under UNIX.
   Correct a fault in the API-based connectivity code between mg_web and YottaDB under UNIX.
   Introduce an Event Log facility that can be controlled by the log_level configuration parameter.

Version 1.1.4 17 August 2020:
   Introduce the ability to stream response content back to the client using M Write statements (InterSystems Databases) or by using the supplied write^%zmgsis() procedure (YottaDB and InterSystems Databases).
   Include the configuration path and server names used for the request in the DB Server function's %system array.

Version 1.1.5 19 August 2020:
   Introduce HTTP v2 compliance.

Version 2.0.6 29 August 2020:
   Introduce WebSocket support.
   Introduce stream ASCII mode.
   Reset the UCI/Namespace after completing each web request (and before re-using the same DB server process for processing the next web request). 
   Insert a default HTTP response header (Content-type: text/html) if the application doesn't return one.

Version 2.0.7 2 September 2020:
   Correct a fault in WebSocket connectivity for Nginx under UNIX.

Version 2.0.8 30 October 2020:
   Introduce a configuration parameter ('chunking') to control the level at which HTTP chunked transfer is used. Chunking can be completely disabled ('chunking off'), or set to only be used if the response payload exceeds a certain size (e.g. 'chunking 250KB').
   Introduce the ability to define custom HTML pages (specified as full URLs) to be returned on mg_web error conditions. parameters: custompage_dbserver_unavailable, custompage_dbserver_busy, custompage_dbserver_disabled

Version 2.1.9 6 November 2020:
   Introduce the functionality to support load balancing and failover.
      Web Path Configuration Parameters:
         load_balancing <on|off> (default is off)
         server_affinity variable:<variables, comma-separated> cookie:<name>
   Correct a fault that led to response payloads being truncated when connecting to YottaDB via its API.

Version 2.1.10 12 November 2020:
   Correct a fault that led to web server worker processes crashing if an error occurred while processing a request via a DB Server API
      Error conditions will now be handled gracefully.
   Correct a memory leak that could potentially occur when using Cookies to implement server affinity in a multi-server configuration.
   Introduce a global-level configuration parameter to allow administrators to set the size of the working buffer for handling requests and their response (request_buffer_size).

Version 2.1.11 20 November 2020:
   Correct a fault that led to mg_web not working correctly under Nginx on the Raspberry Pi.

Version 2.1.12 23 November 2020:
   Ensure that API bindings to the DB Server are gracefully closed when the web server terminates the hosting worker process.
      This correction does not affect configurations using network based connectivity between mg_web and the DB Server.

Version 2.1.13 29 January 2021:
   Improved error reporting (to the event log).
   Miscellaneous minor bug fixes.

Version 2.1.14 6 February 2021:
   Correct a regression in the generation of chunked responses (regression introduced in v2.1.13).

Version 2.1.15 20 April 2021:
   Parse multipart content to extract server-affinity variables used for load-balancing and failover.
      In addition to the inclusion of 'helper functions' to parse multipart content in the DB Superserver code base, this update will also parse such content on the web server side to extract variables used for Server Affinity.
	   This enhancement requires DB Superserver version 4.2; Revision 20 (or later)

Version 2.1.16 3 May 2021:
   Introduce a mechanism through which DB Servers can be excluded from Load-Balancing and Failover.
      DB Servers marked in this way are usually reserved to enable specific applications to be accessed through the hosting path.
      Example: servers LOCALA LOCALB:ex LOCALC

Version 2.1.17 26 May 2021:
   Make DB Server names case-insensitive in the configuration and in any server affinity variables.

Version 2.2.18 18 June 2021:
   Introduce support for request payloads that exceed the maximum string length of the target DB Server.
	   This enhancement requires DB Superserver version 4.3; Revision 22 (or later).

Version 2.2.19 23 June 2021:
   Mark all DB Servers for a path as being 'online' after a request fails on account of all servers being being marked 'offline'.
      This will allow subsequent requests to succeed if, in the meantime, a participating DB Server becomes available.

Version 2.2.20 30 June 2021:
   Introduce a configuration parameter (health_check) to instruct mg_web to retry connecting to DB Servers marked as 'offline' after the specified period of time.
      Example: health_check 600 - retry server after 10 minutes.

Version 2.3.21 18 August 2021:
   Introduce support for TLS-secured connectivity between mg_web and InterSystems DB Servers.
      This enhancement requires DB Superserver version 4.4; Revision 23 (or later).

Version 2.3.22 25 August 2021:
   Support the renamed TLS libraries introduced with OpenSSL v1.1.
	   libeay32.dll was renamed as libcrypto.dll (or libcrypto-1_1-x64.dll under x64 Windows and libcrypto-1_1.dll for x86 Windows).
      ssleay32.dll was renamed as libssl.dll (or libssl-1_1-x64.dll under x64 Windows and libssl-1_1.dll for x86 Windows).
   Correct a memory initialization fault that could occasionally lead to connectivity failures between mg_web and the DB Superserver.
	   Recommend that mg_web is not used with DB Superserver v4.4.23. 

Version 2.3.23 1 September 2021:
   Make DB Server names case-insensitive in any server affinity cookie values (the same change was applied to server affinity variables in v2.1.17).
   Improve the validation of values assigned to server affinity variables.
   Introduce a verbose (v) log level.  If set, the key processing steps involved in extracting DB server affinity variables (and cookies). The DB server name chosen will be recorded for each request. 

Version 2.4.24 27 September 2021:
   Introduce Administrator Facilities.  Implemented as REST requests.
   Improve the granularity of error reporting.

Version 2.4.25 13 October 2021:
   Introduce a DB Server configuration parameter (connection_retries) to control the number of attempts (and the total time spent) in connecting to a DB Server before marking it offline.
      connection_retries number_of_connection_retries/total_time_allowed

Version 2.4.26 22 December 2021:
   Introduce a DB Server configuration parameter (idle_timeout) to limit the amount of time that a network connection will remain in the pool without receiving any work.
      idle_timeout timeout_in_seconds
   This enhancement requires DB Superserver version 4.5; Revision 26 (or later).

Version 2.4.27 4 April 2022:
   Correct a regression that led to request payloads not being correctly transmitted to the DB Server from Nginx-based mg_web installations.
      This change only affects mg_web for Nginx and this regression was introduced in v2.2.18.

Version 2.4.28 17 January 2023:
   Remove an unnecessary "mg_web: Bad request" error that was previously recorded in the Apache event log for requests that were destined to be served by modules other than mg_web.
   Correct a fault that led to mg_web connections erroneously closing down if no value was specified for the (optional) idle_timeout configuration parameter. 

Version 2.4.29 29 May 2023:
   Documentation update.

Version 2.5.30 19 July 2023:
   Introduce a parameter to limit the number of connections created to a DB Server
      max_connections
   When the limit is reached, and all connections in the pool are busy, additional requests will queue up to the time allowed in the timeout setting.

Version 2.5.31 3 October 2023:
   Correct a fault in memory management for the Nginx solution.
   Add server types for GT.M, Node.js and Bun.

Version 2.6.32 17 January 2024:
   Improve the mechanism through which WebSocket functions are invoked.
   Instead of embedding the WebSocket function name and path in the client-side script, a mapping must be created in the appropriate location block of the configuration.
   For example:
	   websocket mywebsocket.mgw websocket^webroutine

Version 2.7.33 3 June 2024:
   Introduce support for Server-Sent Events (SSE).
      Dependent on DB SuperServer v4.5.32.
   Ensure that the 'administrator: off' configuration setting is properly honoured.
   Return a HTTP status code of '504 Gateway Timeout' if a request to the DB Server times-out.
      Previous versions would return '500 Internal Server Error' on response timeout.
      Custom error form defined in: custompage_dbserver_timeout.
   Correct a warning that lead to compiler errors on ARM64 (Raspberry Pi) platforms.
      error: pointer 'fp' used after 'fclose' [-Werror=use-after-free] n = fcntl(fileno(fp), F_SETLK, &lock);

Version 2.7.34 7 June 2024:
   Correct a fault in the management of SSE channels that could lead to infinite loops - particularly when used with the JavaScript Superserver.

Version 2.7.35 19 June 2024:
   Record extra error information when connections made through the YottaDB API fail.
   For UNIX systems, record at initialisation-time the user and group under which the hosting web server worker process is running.
   For example:
      mg_web: worker initialization
      configuration: /opt/nginx1261/conf/mgweb.conf (user=nobody; group=nogroup)

Version 2.7.36 21 June 2024:
   Correct a fault that resulted in the build for the Apache module failing under UNIX.
	   This regression was introduced in v2.7.35.

Version 2.8.37 28 August 2024:
   Introduce support for chunked request payloads.
   - HTTP_TRANSFER_ENCODING=="chunked"

Version 2.8.38 7 September 2024:
   Ensure that excessive amounts of memory are not allocated for large response payloads where chunking is disabled in the mg_web configuration.
   - If chunking cannot be used for request payloads over 500K, the payload will be streamed back with a 'Connection: close' response header added.
   - The end of the stream will be marked by the server closing the connection.

Version 2.8.39 28 October 2024:
   Ensure that global resources are explicitly released when hosting web server worker processes are closed down.

Version 2.8.40 11 November 2024:
   Add extra checks to ensure that an application returns a valid HTTP response header with its forms.
   - If a form's header is found to be faulty (or missing), return the default HTTP header to the client.

Version 2.8.41 9 March 2025:
   Detect the 'client aborted' scenario for SSE channels.
   - Send a termination message to the server ("\xff\xff\xff\xff\xff").
   - For M servers use the $$clientgone^%zmgsis(.%sys) function to detect this termination message (returns 1 if the client has aborted, 0 otherwise).
*/


#include "mg_websys.h"
#include "mg_web.h"
#include "mg_websocket.h"
#include "mg_webstatus.h"
#include "mg_webtls.h"


#if !defined(_WIN32)
extern int errno;
#endif

MGSYS                mg_system         = {0, 0, 0, 0, 0, 0, "", "", "", "", NULL, NULL, NULL, NULL, NULL, NULL, "", {NULL}, {"", "", "", 0, 0, 0, 0, 0, 0, 0, 0, "", ""}};
static NETXSOCK      netx_so           = {0, 0, 0, 0, 0, 0, 0, {'\0'}};
DBXCON *             mg_connection     = NULL;

MGSRV *              mg_server         = NULL;
MGPATH *             mg_path           = NULL;
MGTLS *              mg_tls            = NULL; /* v2.3.21 */

MG_MALLOC            mg_ext_malloc     = NULL;
MG_REALLOC           mg_ext_realloc    = NULL;
MG_FREE              mg_ext_free       = NULL;

static DBXISCSO *    mg_isc_so_global  = NULL;
static DBXYDBSO *    mg_ydb_so_global  = NULL;
static DBXGTMSO *    mg_gtm_so_global  = NULL;

#if defined(_WIN32)
CRITICAL_SECTION     mg_global_mutex;
#else
pthread_mutex_t      mg_global_mutex   = PTHREAD_MUTEX_INITIALIZER;
#endif


int mg_web(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, len;
   unsigned char *p;
   char buffer[256];

#ifdef _WIN32
__try {
#endif

   pweb->plog = mg_system.plog;
   pweb->response_chunked = 0; /* v2.0.8 */
   pweb->response_maxclen = 0; /* v2.8.38 */

   if (!mg_server || !mg_path || mg_system.config_error[0]) {
      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      if (mg_system.config_error[0]) {
         pweb->response_clen = (int) strlen(mg_system.config_error);
         pweb->response_content = mg_system.config_error;
      }
      mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);
      if (pweb->response_clen && pweb->response_content) {
         MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pweb->response_content, pweb->response_clen);
         mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen);
      }
      return CACHE_FAILURE;
   }

   DBX_TRACE(1)
   strcpy(buffer, DBX_WEB_ROUTINE);
   len = (int) strlen((char *) buffer);
   strcpy(pweb->input_buf.buf_addr + 5, buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_DATA, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   /* ctx parameter */
   strcpy(buffer, "");
   len = (int) strlen((char *) buffer);
   strcpy(pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 5), buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_DATA, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   pweb->offs_content = (int) pweb->input_buf.len_used;
   pweb->input_buf.len_used += 5;

   DBX_TRACE(2)
   rc = mg_get_all_cgi_variables(pweb);
   DBX_TRACE(3)
   rc = mg_get_path_configuration(pweb);

#if 0
   if (pweb->sse && !pweb->evented) { /* v2.7.33 sse */
      int n;
      char buffer[256];

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      pweb->response_content = buffer;
      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-cache\r\nConnection: keep-alive\r\n\r\n");
      mg_submit_headers(pweb);
  
      for (n = 0; n < 7; n ++) {
         sprintf(pweb->response_content, "data: line=%d; time=%lu\r\n\r\n", n, (unsigned long) time(NULL));
         pweb->response_clen = (int) strlen(pweb->response_content);
         mg_log_event(pweb->plog, pweb, pweb->response_content, "SSE", 0);
         mg_client_write_now(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen);
#if defined(_WIN32)
         Sleep(7000);
#else
         sleep(7);
#endif
      }
      return CACHE_SUCCESS;
   }
#endif

   /* v2.4.24 */
   DBX_TRACE(4)
   if (!pweb->ppath || rc == CACHE_FAILURE) {
      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);
      mg_log_event(pweb->plog, pweb, "No valid PATH configuration found", "mg_web: error", 0);
      return CACHE_FAILURE;
   }

   if (pweb->ppath->admin) { /* v2.4.24 */
      if (pweb->evented) { /* v2.5.31 */
         return CACHE_SUCCESS;
      }
      else {
         return mg_admin(pweb);
      }
   }

   if (mg_system.log.log_frames) {
      char bufferx[1024];
      sprintf(bufferx, "request=%s; configuration path=%s; server1=%s;", pweb->script_name_lc, pweb->ppath->name, pweb->ppath->servers[0].psrv->name);
      mg_log_event(pweb->plog, pweb, bufferx, "mg_web: information", 0);
   }

   DBX_TRACE(5)
   pweb->psrv = pweb->ppath->servers[0].psrv;
   pweb->server_no = -1;

   if (!pweb->psrv) {
      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);
      mg_log_event(pweb->plog, pweb, "No valid DB SERVER configuration found", "mg_web: error", 0);
      return CACHE_FAILURE;
   }
/*
   {
      char bufferx[256];
      sprintf(bufferx, "HTTP version: %d.%d;", pweb->http_version_major, pweb->http_version_minor);
      mg_log_event(pweb->plog, pweb, bufferx, "mg_web: information", 0);
   }
*/

/*
   sprintf(buffer, "server=%s", pweb->psrv->name);
   len = (int) strlen(buffer);
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);
*/

   DBX_TRACE(6)
   strcpy(buffer, "server=01234567890123456789012345678901");
   len = 39;
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   pweb->server = (char *) (p + 7);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   strcpy(buffer, "server_no=00");
   len = 12;
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   pweb->serverno = (char *) (p + 10);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   sprintf(buffer, "path=%s", pweb->ppath->name);
   len = (int) strlen(buffer);
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   sprintf(buffer, "function=%s", pweb->ppath->function);
   len = (int) strlen(buffer);
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   strcpy(buffer, "no=####");
   len = 7;
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   pweb->requestno = (char *) (p + 3);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   /* v2.2.18 */
   memset((void *) buffer, 0, 15);
   strcpy(buffer, "key=");
   len = 14;
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   memcpy((void *) p, (void *) buffer, 14);
   pweb->requestkey = (char *) (p + 4);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   /* v2.2.18 */
   strcpy(buffer, "mode=tcp");
   len = 8;
   p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
   strcpy((char *) p, buffer);
   pweb->mode = (char *) (p + 5);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   if (pweb->sse) { /* v2.7.33 */
      strcpy(buffer, "sse=1");
      len = 5;
      p = (unsigned char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);
      strcpy((char *) p, buffer);
      pweb->mode = (char *) (p + 5);
      mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBSYS, DBX_DTYPE_STR);
      pweb->input_buf.len_used += (len + 5);
   }

   DBX_TRACE(7)
   rc = mg_websocket_check(pweb);

   if (pweb->evented) {
      return rc;
   }

   /* v2.8.37 test procedure for chunked requests */
#if 0
   if (pweb->request_chunked) {
      int n, rc, total;

      total = 0;
      pweb->request_content = (char *) pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 5); /* space for content length */
      for (n = 0; n < 100; n ++) {
         rc = mg_client_read(pweb, (unsigned char *) pweb->request_content, 10000);
         total += rc;
         {
            char bufferx[1024];
            sprintf(bufferx, "HTTP Request: n=%d; rc=%d; total=%d; request_read_status=%d;", n, rc, total, pweb->request_read_status);
            mg_log_event(pweb->plog, pweb, bufferx, (char * ) "mg_client_read", 0);
         }

         if (pweb->request_read_status == 1) {
            break;
         }
      }

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);
      mg_log_event(pweb->plog, pweb, "Test Probe", "mg_web: error", 0);
      return CACHE_FAILURE;
   }
#endif

   DBX_TRACE(8)
   pweb->request_long = 0;
   if (pweb->request_clen) {
      /* v2.2.18 */
      pweb->request_bsize = (pweb->input_buf.len_alloc - (pweb->input_buf.len_used + 15)); /* amount of buffer space available for request payload */
      if (pweb->psrv->max_string_size < pweb->input_buf.len_alloc) {
         pweb->request_bsize = (pweb->psrv->max_string_size - (pweb->input_buf.len_used + 15)); /* amount of buffer space available for request payload */
         /* mg_log_event(pweb->plog, pweb, (char *) "******* Revise buffer size downwards *******", (char *) "HTTP Request Payload", 0); */
      }
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "HTTP Request: pweb->input_buf.len_used=%d (len_alloc=%d;max_string_size=%lu); pweb->request_clen=%d; payload_buffer_available=%d;", pweb->input_buf.len_used, pweb->input_buf.len_alloc, pweb->psrv->max_string_size, pweb->request_clen, pweb->request_bsize);
         mg_log_event(pweb->plog, pweb, bufferx, (char *) "HTTP Request Payload", 0);
      }
*/
      if (pweb->request_clen < pweb->request_bsize) {
         pweb->request_content = (char *) pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 5); /* space for content length */
         pweb->request_bsize = (pweb->input_buf.len_alloc - pweb->input_buf.len_used);
         mg_client_read(pweb, (unsigned char *) pweb->request_content, pweb->request_clen);
         mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) pweb->request_clen, DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
         pweb->input_buf.len_used += (pweb->request_clen + 5);
      }
      else { /* v2.2.18 */
         pweb->request_long = 1;
         pweb->request_content = (char *) pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 20); /* space for headers EOF marker, content chunk length and param = '' AND 5 byte header for content itself */
         pweb->request_clen_remaining = pweb->request_clen;
         mg_client_read(pweb, (unsigned char *) pweb->request_content, pweb->request_bsize);
         mg_add_block_size((unsigned char *) (pweb->request_content - 5), (unsigned long) 0, (unsigned long) pweb->request_bsize, DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
         pweb->request_csize = pweb->request_bsize;
         pweb->request_clen_remaining -= pweb->request_csize;
/*
         {
            char bufferx[1024];
            sprintf(bufferx, "HTTP Long Request 1: pweb->request_content=%p; pweb->input_buf.len_used=%d; pweb->request_clen=%d; pweb->request_clen_remaining=%d; clen_to_send=%d;", pweb->request_content, pweb->input_buf.len_used, pweb->request_clen, pweb->request_clen_remaining, pweb->request_bsize);
            mg_log_buffer(pweb->plog, pweb, (pweb->request_content - 5), 30, bufferx, 0);
         }
*/
      }
   }
   else if (pweb->request_chunked) { /* v2.8.37 */
      /* v2.2.18 */
      pweb->request_bsize = (pweb->input_buf.len_alloc - (pweb->input_buf.len_used + 15)); /* amount of buffer space available for request payload */
      if (pweb->psrv->max_string_size < pweb->input_buf.len_alloc) {
         pweb->request_bsize = (pweb->psrv->max_string_size - (pweb->input_buf.len_used + 15)); /* amount of buffer space available for request payload */
         /* mg_log_event(pweb->plog, pweb, (char *) "******* Revise buffer size downwards *******", (char *) "HTTP Request Payload", 0); */
      }
      pweb->request_long = 1;
      pweb->request_content = (char *) pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 20); /* space for headers EOF marker, content chunk length and param = '' AND 5 byte header for content itself */
      len = mg_client_read(pweb, (unsigned char *) pweb->request_content, pweb->request_bsize);
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "HTTP Chunked Request: len=%d; buffer_size=%d; request_read_status=%d;", len, pweb->request_bsize, pweb->request_read_status);
         mg_log_event(pweb->plog, pweb, bufferx, (char *) "mg_client_read 1", 0);
      }
*/
      mg_add_block_size((unsigned char *) (pweb->request_content - 5), (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
      pweb->request_csize = len;
   }

   DBX_TRACE(9)
   rc = mg_web_process(pweb);

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_web: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_web_process(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, len, len1, get, get1, close_connection, failover_no;
   unsigned char *p;
   char buffer[256], info[256];
   DBXVAL *pval;

#ifdef _WIN32
__try {
#endif

   *info = '\0';
   close_connection = 0;

   if (pweb->ppath->admin && pweb->evented) { /* v2.5.31 */
      return mg_admin(pweb);
   }

#if 0
   if (pweb->sse && pweb->evented) { /* v2.7.33 sse */
      int n;
      char buffer[256];

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      pweb->response_content = buffer;
      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-cache\r\nConnection: keep-alive\r\n\r\n");

      mg_submit_headers(pweb);
  
      for (n = 0; n < 7; n ++) {
         sprintf(pweb->response_content + 4, "data: (evented) line=%d; time=%lu\r\n\r\n", n, (unsigned long) time(NULL));
         pweb->response_clen = (int) strlen(pweb->response_content + 4);
         sprintf(pweb->response_content, "%x", pweb->response_clen);
         pweb->response_content[2] = '\r';
         pweb->response_content[3] = '\n';
         strcat(pweb->response_content, "\r\n");

         mg_log_buffer(pweb->plog, pweb, pweb->response_content, pweb->response_clen + 6, "SSE", 0);
         mg_client_write_now(pweb, (unsigned char *) pweb->response_content, (int) pweb->response_clen + 6);
#if defined(_WIN32)
         Sleep(7000);
#else
         sleep(7);
#endif
      }
      mg_client_write_now(pweb, (unsigned char *) "0\r\n\r\n", (int) 5);
      return CACHE_SUCCESS;
   }
#endif

   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr, pweb->input_buf.len_used, 0,  DBX_DSORT_EOD, DBX_DTYPE_STR8);
   pweb->input_buf.len_used += 5;

   len = (pweb->input_buf.len_used - (pweb->offs_content + 5));
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->offs_content, (unsigned long) 0, (unsigned long) len, DBX_DSORT_DATA, DBX_DTYPE_STR);

   DBX_TRACE(1)
   /* param parameter */
   strcpy(buffer, "");
   len = (int) strlen((char *) buffer);
   strcpy(pweb->input_buf.buf_addr + (pweb->input_buf.len_used + 5), buffer);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_DATA, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   DBX_TRACE(2)
   if (pweb->ppath->servers[1].psrv) { /* more than one server - look for affinity variable or cookie */
      if (pweb->ppath->sa_order == 1) { /* look for server affinity variable first */
         DBX_TRACE(3)
         if (mg_find_sa_variable(pweb) == -1) {
            if (pweb->ppath->sa_cookie) {
               mg_find_sa_cookie(pweb);
            }
         }
      }
      else if (pweb->ppath->sa_order == 2) { /* look for server affinity cookie first */
         DBX_TRACE(4)
         if (mg_find_sa_cookie(pweb) == -1) {
            if (pweb->ppath->sa_variables[0]) {
               mg_find_sa_variable(pweb);
            }
         }
      }
   }

/*
{
   char bufferx[1024];
   sprintf(bufferx, "HTTP Request: pweb=%p; psrv=%p; ppath=%p;", (void *) pweb, (void *) pweb->psrv, (void *) pweb->ppath);
   mg_log_buffer(pweb->plog, pweb, pweb->input_buf.buf_addr, pweb->input_buf.len_used, bufferx, 0);
}
*/

   DBX_TRACE(5)
   failover_no = 0;
   pweb->failover_possible = 1;

mg_web_process_failover:

   pweb->mg_connect_failed = 0;
   DBX_TRACE(6)
   rc = mg_obtain_connection(pweb);

   DBX_TRACE(7)
   if (rc == CACHE_FAILURE) { /* v2.1.13 */
      if (!pweb->error[0]) {
         strcpy(pweb->error, "Cannot connect to DB Server");
      }
      mg_log_event(pweb->plog, pweb, pweb->error, "mg_web: connectivity error", 0);

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      mg_web_http_error(pweb, 503, MG_CUSTOMPAGE_DBSERVER_BUSY);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);

      return 0;
   }
   if (rc == CACHE_MAXCON) { /* v2.5.30 */
      if (!pweb->error[0]) {
         strcpy(pweb->error, "Cannot connect to DB Server - All connections busy");
      }
      mg_log_event(pweb->plog, pweb, pweb->error, "mg_web: connectivity error", 0);

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      mg_web_http_error(pweb, 503, MG_CUSTOMPAGE_DBSERVER_BUSY);
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);

      return 0;
   }

   DBX_TRACE(20)
   if (rc != CACHE_SUCCESS) {
      if (!pweb->error[0]) {
         strcpy(pweb->error, "Cannot connect to DB Server");
      }
      mg_log_event(pweb->plog, pweb, pweb->error, "mg_web: connectivity error", 0);

      if (pweb->mg_connect_failed) { /* v2.4.24 only mark a server offline on account of a new connection failure */
         mg_server_offline(pweb, pweb->psrv, info, 0);
         if (info[0]) {
            mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
         }
      }
      /* v2.1.16 : DB Servers marked for exclusive use cannot failover to another server in the list */
      if (pweb->ppath->srv_max > 1 && pweb->ppath->srv_max > failover_no && pweb->ppath->servers[pweb->server_no].exclusive == 0) {
         sprintf(pweb->error, "Cannot connect to DB Server %s; attempting to failover", pweb->psrv ? pweb->psrv->name : "null");
         mg_log_event(pweb->plog, pweb, pweb->error, "mg_web: connectivity error", 0);
         failover_no ++;
         goto mg_web_process_failover;
      }

      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      if (rc == NETX_READ_TIMEOUT) { /* v2.7.33 */
         mg_web_http_error(pweb, 504, MG_CUSTOMPAGE_DBSERVER_TIMEOUT);
      }
      else {
         mg_web_http_error(pweb, 503, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      }
      MG_LOG_RESPONSE_HEADER(pweb);
      mg_submit_headers(pweb);
      return 0;
   }

   DBX_TRACE(21)
   *(pweb->serverno) = (unsigned char) ((pweb->server_no / 10) + 48);
   *(pweb->serverno + 1) = (unsigned char) ((pweb->server_no % 10) + 48);
   strncpy(pweb->server, pweb->psrv->name, pweb->psrv->name_len);
   pweb->server[pweb->psrv->name_len] = '\0';
   mg_set_size((unsigned char *) pweb->requestno, pweb->requestno_in);
   DBX_TRACE(22)
   if (pweb->request_long) { /* v2.2.18 */
      DBX_TRACE(23)
      mg_set_size((unsigned char *) pweb->requestkey, (unsigned long) mg_current_process_id());
      mg_set_size((unsigned char *) (pweb->requestkey + 4), (unsigned long) pweb->requestno_in);
      sprintf(pweb->key, "%lu:%lu", (unsigned long) mg_current_process_id(), (unsigned long) pweb->requestno_in);
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "HTTP Long Request: pid=%ld; req_no=%ld; key=%s", (unsigned long) mg_current_process_id(), (unsigned long) pweb->requestno_in, pweb->key);
         mg_log_buffer(pweb->plog, pweb, pweb->requestkey, 10, bufferx, 0);
      }
*/
   }

/*
   {
      int n;
      for (n = 0; n < 3; n ++) {
         netx_tcp_ping(pcon, pweb, 0);
      }
   }
*/

   DBX_TRACE(24)
   rc = mg_web_execute(pweb);

   /* v2.7.33 */
/*
   {
      char bufferx[1024];
      sprintf(bufferx, "HTTP Response: rc=%d; len_used=%d;", rc, pweb->output_val.svalue.len_used);
      mg_log_buffer(pweb->plog, pweb, pweb->output_val.svalue.buf_addr + 5, pweb->output_val.svalue.len_used - 5, bufferx, 0);
   }
*/

   DBX_TRACE(30)
   get = 0;
   if (rc == CACHE_SUCCESS) {
      pweb->response_headers = pweb->output_val.svalue.buf_addr + 10;
      if (strncmp(pweb->response_headers, "HTTP/", 5)) { /* v2.8.40 */
         p = NULL;
      }
      else {
         p = (unsigned char *) strstr(pweb->response_headers, "\x0d\x0a\x0d\x0a");
      }
      if (p) {
         *p = '\0';
         pweb->response_content = (char *) (p + 4);
         pweb->response_headers_len = (int) strlen(pweb->response_headers) + 4;

         pweb->response_clen = (pweb->response_size - (pweb->response_headers_len + 5)); /* 5 Byte offset (rno) */
         get = (pweb->output_val.svalue.len_used - (pweb->response_headers_len + 10)); /* 10 Byte offset (blen + rno) */
/*
         {
            char bufferx[1024];
            sprintf(bufferx, "len_used=%d; headers_len=%d; content-length=%d; response_size=%d; get=%d;", pweb->output_val.svalue.len_used, pweb->response_headers_len, pweb->response_clen, pweb->response_size, get);
            mg_log_event(pweb->plog, pweb, (char *) bufferx, "Diagnostic", 0);
         }
*/
      }
      else {
         pweb->response_content = (char *) (pweb->output_val.svalue.buf_addr + 10);
         pweb->response_headers = (char *) MG_DEFAULT_HEADER;
         pweb->response_headers_len = (int) strlen(pweb->response_headers) + 4;

         if (pweb->output_val.svalue.len_used > 10) {
            pweb->response_clen = (pweb->response_size - 5); /* 5 Byte offset (rno) */
            get = (pweb->output_val.svalue.len_used - 10); /* 10 Byte offset (blen + rno) */
         }
         else {
            pweb->response_clen = 0;
            get = 0;
         }
      }

      /* v2.7.33 */
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "HTTP Response (raw headers): len_used=%d; alloc=%d; content-length=%d; headers_len=%d; get=%d; pweb->wserver_chunks_response=%d", pweb->output_val.svalue.len_used, pweb->output_val.svalue.len_alloc, pweb->response_clen, pweb->response_headers_len, get, pweb->wserver_chunks_response);
         mg_log_buffer(pweb->plog, pweb, (char *) pweb->response_headers, (int) pweb->response_headers_len, bufferx, 0);
      }
*/
      DBX_TRACE(40)
      p = (unsigned char *) (pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4));
      strcpy((char *) p, pweb->response_headers);
      pweb->response_headers = (char *) p;

      mg_parse_headers(pweb);

      DBX_TRACE(41)
      pweb->response_headers_len = (int) strlen(pweb->response_headers);
      if (pweb->ppath->sa_cookie) {
         if (pweb->tls) {
            sprintf(buffer, "\r\nSet-Cookie: %s=%d; path=/; httpOnly; secure;", (char *) pweb->ppath->sa_cookie, pweb->server_no);
         }
         else {
            sprintf(buffer, "\r\nSet-Cookie: %s=%d; path=/; httpOnly;", (char *) pweb->ppath->sa_cookie, pweb->server_no);
         }
         len = (int) strlen(buffer);
         strcpy(pweb->response_headers + pweb->response_headers_len, buffer);
         pweb->response_headers_len += len;
      }

      if (pweb->sse) { /* v2.7.33 */
         pweb->wserver_chunks_response = 1; /* Effectively turn off chunking - let web server handle response framing */
         if (!pweb->response_content_type) {
            strcpy(buffer, "\r\nContent-Type: text/event-stream");
            len = (int) strlen(buffer);
            strcpy(pweb->response_headers + pweb->response_headers_len, buffer);
            pweb->response_headers_len += len; 
         }
         if (pweb->wserver_chunks_response == 0)
            strcpy(buffer, "\r\nTransfer-Encoding: chunked\r\n\r\n");
         else
            strcpy(buffer, "\r\n\r\n");
         len = (int) strlen(buffer);
         strcpy(pweb->response_headers + pweb->response_headers_len, buffer);
         pweb->response_headers_len += len;
      }
      else {
         if (pweb->response_clen_server >= 0) { /* DB server supplied content length */
            pweb->wserver_chunks_response = 1; /* Effectively turn off chunking */
            strcpy(pweb->response_headers + pweb->response_headers_len, "\r\n\r\n");
            pweb->response_headers_len += 4;
         }
         else {
            if (pweb->response_streamed && pweb->response_chunked && pweb->response_remaining > 0) {
               if (pweb->wserver_chunks_response == 0)
                  strcpy(buffer, "\r\nTransfer-Encoding: chunked\r\n\r\n");
               else
                  strcpy(buffer, "\r\n\r\n");
            }
            else if (pweb->response_streamed && pweb->response_maxclen && pweb->response_remaining > 0) { /* v2.8.38 */
               strcpy(buffer, "\r\nConnection: close\r\n\r\n");
               pweb->wserver_chunks_response = 1; /* Effectively turn off chunking */
/*
               if (pweb->wserver_chunks_response == 0)
                  strcpy(buffer, "\r\nTransfer-Encoding: chunked\r\n\r\n");
               else
                  strcpy(buffer, "\r\n\r\n");
*/
            }
            else {
               pweb->response_streamed = 0;
               sprintf(buffer, "\r\nContent-Length: %d\r\n\r\n", pweb->response_clen);
            }
            len = (int) strlen(buffer);
            strcpy(pweb->response_headers + pweb->response_headers_len, buffer);
            pweb->response_headers_len += len;
            /* mg_log_event(pweb->plog, pweb, pweb->response_headers, "Parsed Response Headers", 0); */
         }
      }
   }
   else { /* v2.1.13 */
      DBX_TRACE(42)

      if (pweb->mg_connect_failed) { /* v2.4.24 only mark a server offline on account of a new connection failure */
         mg_server_offline(pweb, pweb->psrv, info, 1);
         if (info[0]) {
            mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
         }
      }
      else {
         mg_release_connection(pweb, 1); /* v2.4.24 */
      }
      /* v2.1.16 : DB Servers marked for exclusive use cannot failover to another server in the list */
      if (pweb->failover_possible && pweb->ppath->srv_max > 1 && pweb->ppath->srv_max > failover_no && pweb->ppath->servers[pweb->server_no].exclusive == 0) {
         sprintf(pweb->error, "Cannot send request to DB Server %s; attempting to failover", pweb->psrv ? pweb->psrv->name : "null");
         mg_log_event(pweb->plog, pweb, pweb->error, "mg_web: connectivity error", 0);
         failover_no ++;
         goto mg_web_process_failover;
      }
   }

   DBX_TRACE(50)
   if (rc != CACHE_SUCCESS) {

      /* v2.1.10 */
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "response: rc=%d; pweb->output_val.svalue.len_used=%d;", rc, (int) pweb->output_val.svalue.len_used);
         mg_log_event(pweb->plog, pweb, (char *) bufferx, "Error from mg_web_execute", 0);
      }
*/
      if (pweb->output_val.svalue.len_used > 10) {
         pweb->response_clen = (pweb->output_val.svalue.len_used - 10);
         pweb->response_content = (char *) (pweb->output_val.svalue.buf_addr + 10);
      }
      pweb->response_headers = (char *) (pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4));
      if (pweb->error[0]) {
         pweb->response_content = (char *) pweb->error;
         pweb->response_clen = (int) strlen(pweb->error);
         if (mg_system.log.log_errors) {
            mg_log_event(&(mg_system.log), pweb, pweb->error, "mg_web: error", 0);
         }
      }
      pweb->response_headers = (char *) pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + 4);
      if (rc == NETX_READ_TIMEOUT) { /* v2.7.33 */
         mg_web_http_error(pweb, 504, MG_CUSTOMPAGE_DBSERVER_TIMEOUT);
      }
      else {
         mg_web_http_error(pweb, 500, MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE);
      }
      get = pweb->response_clen;
      pweb->response_remaining = 0;
      if (pweb->pcon->net_connection) {
         close_connection = 1;
      }
   }

/*
   {
      char bufferx[1024];
      sprintf(bufferx, "HTTP Response: rc=%d; len_used=%d; alloc=%d; content-length=%d; headers=%s;", rc, pweb->output_val.svalue.len_used, pweb->output_val.svalue.len_alloc, pweb->response_clen, pweb->response_headers);
      mg_log_buffer(pweb->plog, pweb, (char *) pweb->response_content, (int) pweb->response_clen, bufferx, 0);
   }
*/

   MG_LOG_RESPONSE_HEADER(pweb);

   DBX_TRACE(60)
   if (pweb->pwsock) {
      rc = mg_websocket_connection(pweb);
      rc = mg_websocket_disconnect(pweb);
      return rc;
   }

   DBX_TRACE(70)

   mg_submit_headers(pweb);
   DBX_TRACE(71)

   if (pweb->sse) { /* v2.7.33 */

      p = (unsigned char *) (pweb->output_val.svalue.buf_addr + (pweb->output_val.svalue.len_used + pweb->response_headers_len + 6));
      len = (pweb->output_val.svalue.len_alloc - (pweb->output_val.svalue.len_used + pweb->response_headers_len + 6));
      get = 0;

      while (1) {
         rc = netx_tcp_read(pweb, (unsigned char *) p, len, 60, 0); /* v2.8.41: was 86400 */
/*
         {
            char bufferx[256];
            sprintf(bufferx, "SSE Data: rc=%d; len=%d;", rc, len);
            mg_log_buffer(pweb->plog, pweb, (char *) p, (rc > 0 ? rc : 0), bufferx, 0);
         }
*/
         if (rc == NETX_READ_TIMEOUT) { /* v2.8.41 */
            if (mg_client_gone(pweb)) {
               /* mg_log_event(pweb->plog, pweb, "*** client gone (post DBServer timeout test) ***", "SSE Message", 0); */
               break;
            }
            continue;
         }

         if (rc < 1) { /* v2.7.34 */
            break;
         }

         if (pweb->wstype == MG_WS_NGINX) {
            sprintf(buffer, "%x", rc);
            len1 = (int) strlen(buffer);
            p -= (len1 + 2);
            rc += (len1 + 2);
            strncpy((char *) p, buffer, len1);
            *(p + len1) = '\r';
            *(p + (len1 + 1)) = '\n';
            *(p + rc) = '\r';
            *(p + (rc + 1)) = '\n';
            rc += 2;
            /* mg_log_buffer(pweb->plog, pweb, (char *) p, rc, "SSE Data (Chunked)", 0); */
            get = mg_client_write_now(pweb, (unsigned char *) p, (int) rc);
            p += (len1 + 2);
         }
         else {
            get = mg_client_write_now(pweb, (unsigned char *) p, (int) rc);
         }
         if (get < 1) {
            /* mg_log_event(pweb->plog, pweb, "*** client gone (cannot write to client) ***", "SSE Message", 0); */
            break;
         }
      }

      if (pweb->wstype == MG_WS_NGINX) {
         mg_client_write_now(pweb, (unsigned char *) "0\r\n\r\n", (int) 5);
      }

      DBX_TRACE(90)
      mg_cleanup(pweb);
      DBX_TRACE(91)
      if (pweb->pcon) {
         pweb->pcon->no_requests ++;
      }

      get = netx_tcp_write(pweb, (unsigned char *) "\xff\xff\xff\xff\xff", 5);
/*
      {
         char bufferx[256];
         sprintf(bufferx, "netx_tcp_write: fired a 0xff*5 result=%d;", get);
         mg_log_event(pweb->plog, pweb, bufferx, "SSE", 0);
      }
*/
      mg_sleep(10000); /* give server a chance to cleanup. */
      close_connection = 1;
      mg_release_connection(pweb, close_connection);

      /* mg_log_event(pweb->plog, pweb, "*** SSE connection closed ***", "SSE", 0); */

      return CACHE_SUCCESS;
   }

   if (get) {
      if (pweb->response_streamed) {
         if (pweb->wserver_chunks_response == 0) {
            get1 = get;
            pval = pweb->output_val.pnext;
            while (pval) { /* 2.0.8 */
               get1 += (int) pval->svalue.len_used;
               pval = pval->pnext;
            }
            sprintf(buffer, "%x\r\n", get1);
            len = (int) strlen(buffer);
            pweb->response_content -= len;
            get += len;
            /* v2.1.14 */
            memcpy((void *) pweb->response_content, (void *) buffer, (size_t) len);
         }
         MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pweb->response_content, get);
         mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) get);
         pval = pweb->output_val.pnext;
         while (pval) { /* v2.0.8 */
            mg_client_write(pweb, (unsigned char *) pval->svalue.buf_addr, (int) pval->svalue.len_used);
            pval = pval->pnext;
         }
      }
      else {
         MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pweb->response_content, get);
         mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) get);
         pval = pweb->output_val.pnext;
         while (pval) { /* v2.0.8 */
            MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pval->svalue.buf_addr, pval->svalue.len_used); /* v2.1.14 */
            mg_client_write(pweb, (unsigned char *) pval->svalue.buf_addr, (int) pval->svalue.len_used);
            pval = pval->pnext;
         }
      }
   }

   DBX_TRACE(80)
   while (pweb->response_remaining > 0) {
/*
      {
         char bufferx[256];
         sprintf(bufferx, "response_size=%d; buffer_size=%d; response_clen=%d; response_remaining=%d;", pweb->response_size, pweb->output_val.svalue.len_alloc, pweb->response_clen, pweb->response_remaining);
         mg_log_event(pweb->plog, pweb, bufferx, "netx_tcp_read: response_remaining", 0);
      }
*/
      if (pweb->response_streamed) {
         pweb->output_val.svalue.len_used = 0;
         rc = netx_tcp_read_stream(pweb);
         get = pweb->output_val.svalue.len_used;
         pweb->response_content = pweb->output_val.svalue.buf_addr;
         if (pweb->wserver_chunks_response == 0) {
            sprintf(buffer, "\r\n%x\r\n", get);
            len = (int) strlen(buffer);
            pweb->response_content -= len;
            get += len;
            /* v2.1.14 */
            memcpy((void *) pweb->response_content, (void *) buffer, (size_t) len);
            if (pweb->response_remaining == 0) {
               strcpy(pweb->response_content + get, "\r\n0\r\n\r\n");
               get += 7;
            }
         }
         MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pweb->response_content, get);
         mg_client_write(pweb, (unsigned char *) pweb->response_content, (int) get);
      }
      else {

         if (pweb->output_val.num.str) {
            MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, (pweb->output_val.num.str + pweb->output_val.svalue.len_used), pweb->response_remaining);
            mg_client_write(pweb, (unsigned char *) pweb->output_val.num.str + pweb->output_val.svalue.len_used, (int) pweb->response_remaining);
            pweb->response_remaining = 0;
         }
         else {
            get = pweb->response_remaining;
            if (get > (int) pweb->output_val.svalue.len_alloc) {
               get = pweb->output_val.svalue.len_alloc;
            }
            netx_tcp_read(pweb, (unsigned char *) pweb->output_val.svalue.buf_addr, get, pweb->pcon->timeout, 1);
            MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(pweb, pweb->output_val.svalue.buf_addr, get);
            mg_client_write(pweb, (unsigned char *) pweb->output_val.svalue.buf_addr, (int) get);
            pweb->response_remaining -= get;
         }
      }
   }

   DBX_TRACE(90)
   mg_cleanup(pweb);
   DBX_TRACE(91)
   if (pweb->pcon) { /* v2.4.24 */
      pweb->pcon->no_requests ++;
   }
   mg_release_connection(pweb, close_connection);

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_web_process: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


/* 2.0.8 */
int mg_parse_headers(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int len;
   char *pn, *ps, *pv, *pz;
   char head[64];

#ifdef _WIN32
__try {
#endif

   pweb->response_clen_server = -1;
   pweb->response_content_type = NULL; /* v2.7.33 */
   pweb->response_cache_control = NULL;
   pweb->response_connection = NULL;

   pn = pweb->response_headers;
   while (pn && *pn) {
      pz = strstr(pn, "\r\n");
      if (pz) {
         *pz = '\0';
      }
      ps = strstr(pn, ":");
      if (ps) {
         *ps = '\0';
         pv = ps;
         len = (int) (pv - pn);
         if (len > 0 && len < 60) {
            while (*(++ pv) == ' ')
               ;
/*
            {
               char bufferx[256];
               sprintf(bufferx, "Response Header: len=%d; name=%s", len, pn);
               mg_log_event(pweb->plog, pweb, pv, bufferx, 0);
            }
*/
            strcpy(head, pn);
            mg_lcase(head);
            if (strstr(head, "content-length")) {
               pweb->response_clen_server = (int) strtol(pv, NULL, 10);
            }
            else if (strstr(head, "content-type")) { /* v2.7.33 */
               pweb->response_content_type = pn;
            }
            else if (strstr(head, "cache-control")) {
               pweb->response_cache_control = pn;
            }
            else if (strstr(head, "connection")) { /* v2.8.38 */
               pweb->response_connection = pn;
            }
         }
         *ps = ':';
      }
      if (pz) {
         *pz = '\r';
         pn = (pz + 2);
      }
      else {
         pn = NULL;
      }
   }
/*
   {
      char bufferx[256];
      sprintf(bufferx, "Response Header: clen=%d;", pweb->response_clen_server);
      mg_log_event(pweb->plog, pweb, bufferx, "Content Length from Server", 0);
   }
*/

   return CACHE_SUCCESS;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_parse_headers: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int mg_web_execute(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, len, get;
   unsigned long offset, netbuf_used;
   unsigned char *netbuf;
   char label[16], routine[16], params[9], buffer[8];
   DBXFUN fun;
   DBXCON *pcon;

#ifdef _WIN32
__try {
#endif

   pcon = pweb->pcon;
   rc = CACHE_SUCCESS;
   offset = 5;

   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr, pweb->input_buf.len_used, 0,  DBX_DSORT_EOD, DBX_DTYPE_STR8);
   pweb->input_buf.len_used += 5;

   netbuf = (unsigned char *) (pweb->input_buf.buf_addr - DBX_IBUFFER_OFFSET);
   netbuf_used = (pweb->input_buf.len_used + DBX_IBUFFER_OFFSET);
   mg_add_block_size((unsigned char *) netbuf, 0, netbuf_used,  0, DBX_CMND_FUNCTION);
   memset(netbuf + 5, 0, DBX_IBUFFER_OFFSET - 5); /* v2.3.22 */

   strcpy(params, "dbx");
   strcpy(label, "ifc");

   strcpy(routine, "%zmgsis");
   fun.label = label;
   fun.label_len = (int) strlen(label);
   fun.routine = routine;
   fun.routine_len = (int) strlen(routine);
   pweb->output_val.num.str = (char *) 0;

   DBX_TRACE(1)
   MG_LOG_REQUEST_FRAME(pweb, netbuf, netbuf_used);
   MG_LOG_REQUEST_BUFFER(pweb, netbuf, netbuf_used);

/*
   if (mg_system.log.log_transmissions) {
      mg_log_buffer(pweb->plog, pweb, pweb->input_buf.buf_addr, pweb->input_buf.len_used, "mg_web: request buffer", 0);
   }
*/
   /* v2.1.10 */

   if (pcon->net_connection) {
      DBX_TRACE(10)
      memcpy((void *) pweb->mode, (void *) "tcp", (size_t) 3);
      rc = netx_tcp_command(pweb, DBX_CMND_FUNCTION, 0);

/* v2.7.33 */
/*
      {
         char bufferx[64];
         sprintf(bufferx, "netx_tcp_command: rc=%d;", rc);
         mg_log_event(pweb->plog, pweb, bufferx, "Trace", 0);
      }
*/

   }
   else if (pcon->psrv->dbtype == DBX_DBTYPE_YOTTADB) {

      ydb_string_t out, in1, in2, in3;
#if !defined(_WIN32)
      uid_t uid;
      gid_t gid;
      struct passwd *pwd;
      struct group *grp;
#endif

      DBX_TRACE(20)
      memcpy((void *) pweb->mode, (void *) "api", (size_t) 3);
      pweb->failover_possible = 0;
      strcat(fun.label, "_zmgsis");
      fun.label_len = (int) strlen(label);

      strcpy(buffer, "");

      out.address = (char *) pweb->output_val.svalue.buf_addr;
      out.length = (unsigned long) pweb->output_val.svalue.len_alloc;
      in1.address = (char *) buffer;
      in1.length = (unsigned long) strlen(buffer);
      in2.address = (char *) netbuf;
      in2.length = (unsigned long) netbuf_used;
      in3.address = (char *) params;
      in3.length = (unsigned long) strlen(params);

      DBX_LOCK(0);

      if (pweb->request_long) { /* v2.2.18 */
         rc = mg_execute_request_long(pweb, mg_write_chunk_ydb);
      }

      rc = pcon->p_ydb_so->p_ydb_ci(fun.label, &out, &in1, &in2, &in3);

/*
{
   char bufferx[1024];
   sprintf(bufferx, "mg_web_execute (YottaDB:ydb_string_t): rc=%d; len_used=%d; len_alloc=%d; out.length=%d;", rc,  pweb->output_val.svalue.len_used, pweb->output_val.svalue.len_alloc, (int) out.length);
   mg_log_event(pweb->plog, pweb, bufferx, "mg_web_execute: YottaDB API", 0);
}
*/

/*
{
   char bufferx[1024];
   sprintf(bufferx, "mg_web_execute (YottaDB:ydb_buffer_t): rc=%d; len_used=%d; len_alloc=%d; out.len_used=%d;", rc,  (int) pweb->output_val.svalue.len_used, (int) pweb->output_val.svalue.len_alloc, (int) out.len_used);
   mg_log_event(pweb->plog, pweb, bufferx, "mg_web_execute: YottaDB API", 0);
}
*/

      if (rc == YDB_OK) {
/*
         pweb->output_val.svalue.len_used = (unsigned int) mg_get_size((unsigned char *) pweb->output_val.svalue.buf_addr);
         pweb->response_size = (pweb->output_val.svalue.len_used - 5);
*/

         pweb->output_val.api_size = (unsigned int) out.length;
/*
         pweb->output_val.api_size = (unsigned int) out.len_used;
*/

         pweb->output_val.svalue.len_used = pweb->output_val.api_size;
         pweb->response_size = (pweb->output_val.api_size - 5);
         pweb->response_remaining = 0;
      }
      else if (pcon->p_ydb_so->p_ydb_zstatus) { /* v2.7.35 */
         char buffer[1024], uuser[32], ugroup[32];

         *uuser = '\0';
         *ugroup = '\0';
#if defined(_WIN32)
         strcpy(buffer, "mg_web_execute: YottaDB API error text");
#else
         uid = getuid();
         gid = getgid();
         if ((pwd = getpwuid(uid)) != NULL) {
            strncpy(uuser, pwd->pw_name, 30);
            uuser[30] = '\0';
         }
         if ((grp = getgrgid(gid)) != NULL) {
            strncpy(ugroup, grp->gr_name, 30);
            ugroup[30] = '\0';
         }
         sprintf(buffer, "mg_web_execute: YottaDB API error text: process user=%s; group=%s;", uuser, ugroup);
#endif

         pcon->p_ydb_so->p_ydb_zstatus((ydb_char_t *) pweb->error, (ydb_long_t) DBX_ERROR_SIZE - 1); /* v2.7.35 */
         mg_log_event(pweb->plog, pweb, pweb->error, buffer, 0);
      }

      DBX_UNLOCK();
   }
   else {
      unsigned long ne;
      DBXCVAL cvalue;

      DBX_TRACE(30)
      memcpy((void *) pweb->mode, (void *) "api", (size_t) 3);
      pweb->failover_possible = 0;
      strcpy(buffer, "");
      cvalue.pstr = NULL;

      DBX_LOCK(0);

      if (pweb->request_long) { /* v2.2.18 */
         rc = mg_execute_request_long(pweb, mg_write_chunk_isc);
      }

      rc = pcon->p_isc_so->p_CachePushFunc(&(fun.rflag), (int) fun.label_len, (const Callin_char_t *) fun.label, (int) fun.routine_len, (const Callin_char_t *) fun.routine);
      if (rc == CACHE_SUCCESS) {
         rc = pcon->p_isc_so->p_CachePushStr((int) strlen(buffer), (Callin_char_t *) buffer);
         if (rc == CACHE_SUCCESS) {
            if (netbuf_used < CACHE_MAXSTRLEN) { /* v2.2.18 */
               rc = pcon->p_isc_so->p_CachePushStr((int) netbuf_used, (Callin_char_t *) netbuf);
            }
            else {
               cvalue.pstr = (void *) pcon->p_isc_so->p_CacheExStrNew((CACHE_EXSTRP) &(cvalue.zstr), netbuf_used + 1);
               if (cvalue.pstr) {
                  for (ne = 0; ne < netbuf_used; ne ++) {
                     cvalue.zstr.str.ch[ne] = (char) netbuf[ne];
                  }
                  cvalue.zstr.str.ch[ne] = (char) 0;
                  cvalue.zstr.len = netbuf_used;
                  rc = pcon->p_isc_so->p_CachePushExStr((CACHE_EXSTRP) &(cvalue.zstr));
               }
               else {
                  rc = CACHE_FAILURE;
               }
            }
/*
            {
               char bufferx[1024];
               sprintf(bufferx, "p_CachePush[Ex]Str: rc=%d; netbuf_used=%lu; cvalue.pstr=%p;", rc, netbuf_used, cvalue.pstr);
               mg_log_event(pweb->plog, pweb, bufferx, "web_execute", 0);
            }
*/
            if (rc == CACHE_SUCCESS) {
               rc = pcon->p_isc_so->p_CachePushStr((int) strlen(params), (Callin_char_t *) params);
               if (rc == CACHE_SUCCESS) {
                  rc = pcon->p_isc_so->p_CacheExtFun(fun.rflag, 3);
               }
            }
            if (cvalue.pstr) {
               pcon->p_isc_so->p_CacheExStrKill(&(cvalue.zstr));
            }
         }
      }
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "p_CacheExtFun: rc=%d; cvalue.pstr=%p;", rc, cvalue.pstr);
         mg_log_event(pweb->plog, pweb, bufferx, "web_execute", 0);
      }
*/
      if (rc == CACHE_SUCCESS) {
         isc_pop_value(pcon, &(pweb->output_val), DBX_DTYPE_STR);
/*
         {
            char bufferx[1024];
            sprintf(bufferx, "isc_pop_value: len_used=%d; api_size=%d; len_alloc=%d", pweb->output_val.svalue.len_used, pweb->output_val.api_size, pweb->output_val.svalue.len_alloc);
            mg_log_event(pweb->plog, pweb, bufferx, "web_execute", 0);
            mg_log_buffer(pweb->plog, pweb, (char *) pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.len_used, "web_execute", 0);
         }
*/
         pweb->response_size = (pweb->output_val.api_size - 5);
         if (pweb->response_size > (unsigned int) (pweb->output_val.svalue.len_alloc - DBX_HEADER_SIZE)) {
            get = (pweb->output_val.svalue.len_alloc - DBX_HEADER_SIZE);
            memcpy((void *) pweb->output_val.svalue.buf_addr, (void *) pweb->output_val.num.str, get);
            pweb->output_val.svalue.len_used = get;
            pweb->response_remaining = (pweb->response_size - (get - 5));
         }
         else {
            memcpy((void *) pweb->output_val.svalue.buf_addr, (void *) pweb->output_val.num.str, pweb->output_val.api_size);
            pweb->output_val.svalue.len_used = pweb->output_val.api_size;
            pweb->response_remaining = 0;
         }
      }
      else {
         mg_error_message(pweb, rc);
      }
      DBX_UNLOCK();
   }
/*
{
   char buffer[1024];
   sprintf_s(buffer, 1000, "mg_web_execute: rc=%d; len_used=%d;", rc,  pweb->output_val.svalue.len_used);
   mg_log_buffer(pweb->plog, pweb, pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.len_used, buffer, 0);
}
*/

   DBX_TRACE(50)

   if (rc != CACHE_SUCCESS) {
      if (rc == NETX_READ_TIMEOUT) { /* v2.7.33 */
         if (!pweb->error[0]) {
            strcpy(pweb->error, "TCP Request Timeout");
         }
      }
      else {
         rc = CACHE_NOCON;
         if (!pweb->error[0]) {
            strcpy(pweb->error, "Database connectivity error");
         }
      }
      goto mg_web_execute_exit;
   }

   DBX_TRACE(51)

   len = mg_get_block_size((unsigned char *) pweb->output_val.svalue.buf_addr, 0, &(pweb->output_val.sort), &(pweb->output_val.type));

   if (!pcon->net_connection) {
      MG_LOG_RESPONSE_FRAME(pweb, pweb->output_val.svalue.buf_addr, len);
      MG_LOG_RESPONSE_BUFFER(pweb, pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.len_used);
   }
/*
   if (mg_system.log.log_transmissions) {
      mg_log_buffer(pweb->plog, pweb, pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.len_used, "mg_web: response buffer", 0);
   }
*/

   DBX_TRACE(52)

   if (pweb->output_val.sort == DBX_DSORT_EOD || pweb->output_val.sort == DBX_DSORT_ERROR) {
      DBX_TRACE(53)
      rc = CACHE_FAILURE;
      strncpy(pweb->error, pweb->output_val.svalue.buf_addr + offset, len);
      pweb->error[len] = '\0';
      goto mg_web_execute_exit;
   }

   DBX_TRACE(60)

mg_web_execute_exit:

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_web_execute: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int mg_execute_request_long(MGWEB *pweb, int (*p_write_chunk) (MGWEB *, unsigned char *, unsigned int, int))
{
   DBX_TRACE_INIT(0)
   int rc, blen, bsize, ptr, tail_ptr, tail_len, chunk_no;
   unsigned int netbuf_used;
   unsigned char *netbuf;

#ifdef _WIN32
__try {
#endif

   blen = (int) strlen(pweb->boundary);
   pweb->failover_possible = 0; /* can't failover after this point */
   netbuf = (unsigned char *) (pweb->request_content - 5);
   chunk_no = 0;
   DBX_TRACE(1)
   while (1) {
      netbuf_used = pweb->request_csize + 5;
      tail_ptr = 0;
      tail_len = 0;
      if (blen && blen > pweb->request_csize) {
         for (ptr = (pweb->request_csize - blen) + 1; ptr < pweb->request_csize; ptr ++) {
            if (!strncmp(pweb->request_content + ptr, pweb->boundary, pweb->request_csize - ptr)) {
               if (pweb->request_content[ptr - 1] == '-' && pweb->request_content[ptr - 2] == '-') {
                  tail_ptr = ptr;
                  tail_len = pweb->request_csize - tail_ptr;
                  netbuf_used -= tail_len;
                  mg_add_block_size((unsigned char *) netbuf, (unsigned long) 0, (unsigned long) (pweb->request_csize - tail_len), DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
               }
            }
         }
      }
      chunk_no ++;
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "HTTP Long Request 2: chunk_no=%d; pweb->request_content=%p; pweb->input_buf.len_used=%d; pweb->request_clen=%d; pweb->request_clen_remaining=%d; clen_to_send (+5)=%d; tail_ptr=%d; tail_len=%d;", chunk_no, pweb->request_content, pweb->input_buf.len_used, pweb->request_clen, pweb->request_clen_remaining, netbuf_used, tail_ptr, tail_len);
         mg_log_buffer(pweb->plog, pweb, netbuf, 30, bufferx, 0);
         mg_log_event(pweb->plog, pweb, bufferx, "HTTP Long Request 2", 0);
      }
*/
      DBX_TRACE(2)
      rc = p_write_chunk(pweb, (unsigned char *) netbuf, netbuf_used, chunk_no);
      if (rc < 0) {
         break;
      }

      if (pweb->request_chunked) { /* v2.8.37 */
         if (pweb->request_read_status == 1) {
            /* mg_log_event(pweb->plog, pweb, "******* End of Chunked Request Data *******", "HTTP Long Request", 0); */
            rc = CACHE_SUCCESS;
            break;
         }

         DBX_TRACE(3)
         bsize = pweb->request_bsize;
         if (tail_ptr) {
            bsize -= tail_len;
            mg_memcpy((void *) pweb->request_content, (void *) (pweb->request_content + tail_ptr), (size_t) tail_len);
         }

         pweb->request_csize = bsize;

         DBX_TRACE(4)
         pweb->request_csize = mg_client_read(pweb, (unsigned char *) (pweb->request_content + tail_len), pweb->request_csize);
/*
         {
            char bufferx[1024];
            sprintf(bufferx, "HTTP Chunked Request: len=%d; buffer_size=%d; request_read_status=%d;", pweb->request_csize, bsize, pweb->request_read_status);
            mg_log_event(pweb->plog, pweb, bufferx, (char*)"mg_client_read 1", 0);
         }
*/
         mg_add_block_size((unsigned char *) netbuf, 0, (pweb->request_csize + tail_len), DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
      }
      else {
         if (pweb->request_clen_remaining == 0) {
            rc = CACHE_SUCCESS;
            break;
         }

         DBX_TRACE(3)
         bsize = pweb->request_bsize;
         if (tail_ptr) {
            bsize -= tail_len;
            mg_memcpy((void *) pweb->request_content, (void *) (pweb->request_content + tail_ptr), (size_t) tail_len);
         }

         pweb->request_csize = pweb->request_clen_remaining;
         if (pweb->request_csize > bsize) {
            pweb->request_csize = bsize;
         }

         DBX_TRACE(4)
         mg_client_read(pweb, (unsigned char *) (pweb->request_content + tail_len), pweb->request_csize);
         mg_add_block_size((unsigned char *) netbuf, 0, (pweb->request_csize + tail_len), DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
         pweb->request_clen_remaining -= pweb->request_csize;
      }
   }

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_web_execute_request_long: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int mg_write_chunk_tcp(MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no)
{
   int rc;

   rc = netx_tcp_write(pweb, (unsigned char *) netbuf, (int) netbuf_used);
   return rc;
}


int mg_write_chunk_isc(MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no)
{
#if 1

   int rc, option;
   char label[32], routine[32];
   DBXFUN fun;
   unsigned long ne;
   DBXCVAL cvalue;
   DBXCON *pcon;

   pcon = pweb->pcon;
/*
   {
      char bufferx[1024];
      sprintf(bufferx, "mg_write_chunk_isc: chunk_no=%d; key=%s", chunk_no, pweb->key);
      mg_log_buffer(pweb->plog, pweb, netbuf, 32, "mg_write_chunk_isc", 0);
   }
*/

   /* dbxwebsn(key,cn,data,option) */
   rc = CACHE_SUCCESS;
   option = 0;
   strcpy(label, "dbxwebsn");

   strcpy(routine, "%zmgsis");
   fun.label = label;
   fun.label_len = (int) strlen(label);
   fun.routine = routine;
   fun.routine_len = (int) strlen(routine);

   rc = pcon->p_isc_so->p_CachePushFunc(&(fun.rflag), (int) fun.label_len, (const Callin_char_t *) fun.label, (int) fun.routine_len, (const Callin_char_t *) fun.routine);
   if (rc == CACHE_SUCCESS) {
      rc = pcon->p_isc_so->p_CachePushStr((int) strlen(pweb->key), (Callin_char_t *) pweb->key);
      if (rc == CACHE_SUCCESS) {
         rc = pcon->p_isc_so->p_CachePushInt((int) chunk_no);
         if (rc == CACHE_SUCCESS) {
            cvalue.pstr = (void *) pcon->p_isc_so->p_CacheExStrNew((CACHE_EXSTRP) &(cvalue.zstr), (netbuf_used - 5) + 1);
            if (cvalue.pstr) {
               for (ne = 5; ne < netbuf_used; ne ++) {
                  cvalue.zstr.str.ch[ne - 5] = (char) netbuf[ne];
               }
               cvalue.zstr.str.ch[ne - 5] = (char) 0;
               cvalue.zstr.len = (netbuf_used - 5);
               rc = pcon->p_isc_so->p_CachePushExStr((CACHE_EXSTRP) &(cvalue.zstr));
               if (rc == CACHE_SUCCESS) {
                  rc = pcon->p_isc_so->p_CachePushInt((int) option);
                  if (rc == CACHE_SUCCESS) {
                     rc = pcon->p_isc_so->p_CacheExtFun(fun.rflag, 4);
/*
                     {
                        char bufferx[1024];
                        sprintf(bufferx, "mg_write_chunk_isc: p_CacheExtFun=%d; chunk_no=%d; key=%s", rc, chunk_no, pweb->key);
                        mg_log_event(pweb->plog, pweb, bufferx, "mg_write_chunk_isc", 0);
                     }
*/
                     if (rc == CACHE_SUCCESS) {
                        rc = pcon->p_isc_so->p_CachePopInt(&option);
                     }
                  }
               }
            }
         }
      }
   }
   if (cvalue.pstr) {
      pcon->p_isc_so->p_CacheExStrKill(&(cvalue.zstr));
   }

   return rc;

#else

   int rc;
   char global[32];
   unsigned long ne;
   DBXCVAL cvalue;
   DBXCON *pcon;

   pcon = pweb->pcon;
   strcpy(global, "mgweb");

   if (chunk_no == 1) {
      rc = pcon->p_isc_so->p_CachePushGlobal((int) strlen(global), (const Callin_char_t *) global);
      if (rc == CACHE_SUCCESS) {
         rc = pcon->p_isc_so->p_CachePushStr((int) strlen(pweb->key), (Callin_char_t *) pweb->key);
         if (rc == CACHE_SUCCESS) {
            rc = pcon->p_isc_so->p_CacheGlobalKill(1, 0);
/*
            {
               char bufferx[1024];
               sprintf(bufferx, "mg_write_chunk_isc: p_CacheGlobalKill=%d; chunk_no=%d; key=%s", rc, chunk_no, pweb->key);
               mg_log_event(pweb->plog, pweb, bufferx, "mg_write_chunk_isc", 0);
            }
*/
         }
      }
   }

   rc = pcon->p_isc_so->p_CachePushGlobal((int) strlen(global), (const Callin_char_t *) global);
   if (rc == CACHE_SUCCESS) {
      rc = pcon->p_isc_so->p_CachePushStr((int) strlen(pweb->key), (Callin_char_t *) pweb->key);
      if (rc == CACHE_SUCCESS) {
         rc = pcon->p_isc_so->p_CachePushInt((int) chunk_no);
         if (rc == CACHE_SUCCESS) {
            cvalue.pstr = (void *) pcon->p_isc_so->p_CacheExStrNew((CACHE_EXSTRP) &(cvalue.zstr), netbuf_used + 1);
            if (cvalue.pstr) {
               for (ne = 0; ne < netbuf_used; ne ++) {
                  cvalue.zstr.str.ch[ne] = (char) netbuf[ne];
               }
               cvalue.zstr.str.ch[ne] = (char) 0;
               cvalue.zstr.len = netbuf_used;
               rc = pcon->p_isc_so->p_CachePushExStr((CACHE_EXSTRP) &(cvalue.zstr));
               if (rc == CACHE_SUCCESS) {
                  rc = pcon->p_isc_so->p_CacheGlobalSet(2);
/*
                  {
                     char bufferx[1024];
                     sprintf(bufferx, "mg_write_chunk_isc: p_CacheGlobalSet=%d; chunk_no=%d; key=%s", rc, chunk_no, pweb->key);
                     mg_log_event(pweb->plog, pweb, bufferx, "mg_write_chunk_isc", 0);
                  }
*/
               }
            }
         }
      }
   }
   if (cvalue.pstr) {
      pcon->p_isc_so->p_CacheExStrKill(&(cvalue.zstr));
   }

   return rc;

#endif
}


int mg_write_chunk_ydb(MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no)
{
#if 1

   int rc, option;
   char label[32], param[32], obuffer[32];
   DBXFUN fun;
   DBXCON *pcon;
   ydb_string_t out, in1, in2, in3;

   pcon = pweb->pcon;
/*
   {
      char bufferx[1024];
      sprintf(bufferx, "mg_write_chunk_ydb: chunk_no=%d; key=%s", chunk_no, pweb->key);
      mg_log_buffer(pweb->plog, pweb, (char *) netbuf, 32, bufferx, 0);
   }
*/
   /* dbxwebsn(key,cn,data,option) */
   rc = CACHE_SUCCESS;
   option = 0;
   sprintf(param, "websn:%d:%d", chunk_no, option);
   strcpy(label, "ifc_zmgsis");
   fun.label = label;
   fun.label_len = (int) strlen(label);

   out.address = (char *) obuffer;
   out.length = (unsigned long) sizeof(obuffer);
   in1.address = (char *) pweb->key;
   in1.length = (unsigned long) strlen(pweb->key);
   in2.address = (char *) (netbuf + 5);
   in2.length = (unsigned long) (netbuf_used - 5);
   in3.address = (char *) param;
   in3.length = (unsigned long) strlen(param);

   rc = pcon->p_ydb_so->p_ydb_ci(fun.label, &out, &in1, &in2, &in3);
/*
   {
      char bufferx[1024];
      sprintf(bufferx, "mg_write_chunk_ydb: rc=%d; chunk_no=%d; key=%s", rc, chunk_no, pweb->key);
      mg_log_event(pweb->plog, pweb, bufferx, "mg_write_chunk_isc", 0);
   }
*/
   return rc;

#else

   int rc;
   char global[32], buffer[32];
   DBXCON *pcon;
   ydb_buffer_t subs[4], glo, data;

   pcon = pweb->pcon;
   strcpy(global, "^mgweb");

   glo.buf_addr = (char *) global;
   glo.len_used = (unsigned long) strlen(global);
   glo.len_alloc = glo.len_used;

   if (chunk_no == 1) {
      subs[0].buf_addr = (char *) pweb->key;
      subs[0].len_used = (unsigned long) strlen(pweb->key);
      subs[0].len_alloc = subs[0].len_used;

      rc = pcon->p_ydb_so->p_ydb_delete_s(&glo, 1, &subs[0], YDB_DEL_TREE);
   }

   sprintf(buffer, "%d", chunk_no);
   subs[0].buf_addr = (char *) pweb->key;
   subs[0].len_used = (unsigned long) strlen(pweb->key);
   subs[0].len_alloc = subs[0].len_used;
   subs[1].buf_addr = (char *) buffer;
   subs[1].len_used = (unsigned long) strlen(buffer);
   subs[1].len_alloc = subs[1].len_used;

   data.buf_addr = (char *) netbuf;
   data.len_used = (unsigned long) netbuf_used;
   data.len_alloc = data.len_used;

   rc = pcon->p_ydb_so->p_ydb_set_s(&glo, 2, &subs[0], &data);

   return rc;

#endif
}


int mg_web_http_error(MGWEB *pweb, int http_status_code, int custompage)
{
   DBX_TRACE_INIT(0)
   char *custompage_url;
   char buffer[64];

#ifdef _WIN32
__try {
#endif

   custompage_url = NULL;
   if (custompage > MG_CUSTOMPAGE_DBSERVER_NONE) {
      if (custompage == MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE && mg_system.custompage_dbserver_unavailable)
         custompage_url =  mg_system.custompage_dbserver_unavailable;
      else if (custompage == MG_CUSTOMPAGE_DBSERVER_BUSY && mg_system.custompage_dbserver_busy)
         custompage_url =  mg_system.custompage_dbserver_busy;
      else if (custompage == MG_CUSTOMPAGE_DBSERVER_DISABLED && mg_system.custompage_dbserver_disabled)
         custompage_url =  mg_system.custompage_dbserver_disabled;
      else if (custompage == MG_CUSTOMPAGE_DBSERVER_TIMEOUT && mg_system.custompage_dbserver_timeout) /* v2.7.33 */
         custompage_url =  mg_system.custompage_dbserver_timeout;

      if (custompage_url) {
         if (pweb->request_method && !strcmp(pweb->request_method, "POST"))
            sprintf(pweb->response_headers, "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\nConnection: close\r\n\r\n", custompage_url);
         else
            sprintf(pweb->response_headers, "HTTP/1.1 307 Temporary Redirect\r\nLocation: %s\r\nConnection: close\r\n\r\n", custompage_url);
         pweb->response_headers_len = (int) strlen(pweb->response_headers);
         return 0;
      }
   }

   /* v2.7.33 504 Gateway Timeout (wating for the DB Server) ; 408 Request Timeout (wating for the client) */
   if (http_status_code == 504) {
      sprintf(pweb->response_headers, "HTTP/1.1 %d Gateway Timeout\r\nConnection: close", http_status_code);
   }
   else if (http_status_code == 408) {
      sprintf(pweb->response_headers, "HTTP/1.1 %d Request Timeout\r\nConnection: close", http_status_code);
   }
   else {
      sprintf(pweb->response_headers, "HTTP/1.1 %d Internal Server Error\r\nConnection: close", http_status_code);
   }
   if (pweb->response_clen >= 0) {
      sprintf(buffer, "\r\nContent-Length: %d\r\n\r\n", pweb->response_clen);
   }
   else {
      strcpy(buffer, "\r\n\r\n");
   }
   strcat(pweb->response_headers, buffer);
   pweb->response_headers_len = (int) strlen(pweb->response_headers);

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_web_http_error: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_web_simple_response(MGWEB *pweb, char *text, char *error, int context)
{
   int len;
   char *p;
   char buffer[64], response[256];

   p = NULL;
   len = 0;
   if (context == 0) {
      if (text) {
         p = text;
         len = (int) strlen(text);
      }
      else if (error) {
         p = error;
         len = (int) strlen(error);
      }
      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n");

      if (len) {
         sprintf(buffer, "Content-Length: %d\r\n\r\n", len);
      }
      else {
         strcpy(buffer, "\r\n");
      }
      strcat(pweb->response_headers, buffer);
      pweb->response_headers_len = (int) strlen(pweb->response_headers);
   }
   else {
      sprintf(response, "{\r\n   \"ok\": %d,\r\n   \"result\": \"%s\",\r\n   \"error\": \"%s\"\r\n}\r\n", error ? 0 : 1, text ? text : "", error ? error : "");
      len = (int) strlen(response);
      p = response;

      strcpy(pweb->response_headers, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n");

      if (len) {
         sprintf(buffer, "Content-Length: %d\r\n\r\n", len);
         strcat(pweb->response_headers, buffer);
      }
      else {
         strcpy(buffer, "\r\n");
      }
      strcat(pweb->response_headers, buffer);
      pweb->response_headers_len = (int) strlen(pweb->response_headers);
   }

   mg_submit_headers(pweb);
   if (p) {
      mg_client_write(pweb, (unsigned char *) p, (int) len);
   }
   return CACHE_SUCCESS;
}


int mg_get_all_cgi_variables(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int result, rc, n, len, lenv;
   char *p;
   char buffer[128];

#ifdef _WIN32
__try {
#endif

   lenv = 32;
   rc = mg_get_cgi_variable(pweb, "HTTPS", buffer, &lenv);
   if (rc == MG_CGI_SUCCESS) {
      mg_lcase(buffer);
      if (!strcmp(buffer, "on")) {
         pweb->tls = 1;
      }
   }

   /* v2.7.33 look for SSE header: Accept: text/event-stream */
   pweb->sse = 0;
   lenv = 127;
   rc = mg_get_cgi_variable(pweb, "HTTP_ACCEPT", buffer, &lenv);
   if (rc == MG_CGI_SUCCESS) {
      mg_lcase(buffer);
      if (strstr(buffer, "/event-stream")) {
         pweb->sse = 1;
      }
   }

   result = 0;
   for (n = 0; n < mg_system.cgi_max; n ++) {
      p = (pweb->input_buf.buf_addr + pweb->input_buf.len_used + 5);

      strcpy(p, mg_system.cgi[n]);
      strcat(p, "=");
      len = (int) strlen(p);
      p += len;

      lenv = (pweb->input_buf.len_alloc - (pweb->input_buf.len_used + len + 5));
      rc = mg_get_cgi_variable(pweb, mg_system.cgi[n], p, &lenv);

      if (rc == MG_CGI_SUCCESS) {
         len += lenv;
         mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBCGI, DBX_DTYPE_STR);
         pweb->input_buf.len_used += (len + 5);

         if (n == DBX_CGI_REQUEST_METHOD) {
            pweb->request_method = p;
            pweb->request_method_len = lenv;
         }
         else if (n == DBX_CGI_SCRIPT_NAME) {
            pweb->script_name = p;
            pweb->script_name_len = lenv;
            if (lenv > 250)
               lenv = 250;
            strncpy(pweb->script_name_lc, p, lenv);
            pweb->script_name_lc[lenv] = '\0';
            mg_lcase(pweb->script_name_lc);
         }
         if (n == DBX_CGI_QUERY_STRING) {
            pweb->query_string = p;
            pweb->query_string_len = lenv;
         }
      }
   }

	return result;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_get_all_cgi_variables: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif

}


int mg_get_path_configuration(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, len_max;
   MGPATH *ppath, *ppath_max;

#ifdef _WIN32
__try {
#endif

   rc = 0;
   len_max = 0;
   ppath_max = NULL;

   ppath = mg_path;
   while (ppath) {
      if (!strncmp(pweb->script_name_lc, ppath->name, ppath->name_len)) {
         if (ppath->name_len > len_max) {
            len_max = ppath->name_len;
            ppath_max = ppath;
         }
      }
/*
      if (mg_system.log.log_frames) {
         char bufferx[256];
         sprintf(bufferx, "request=%s; ppath=%p; path=%s; ppath->name_len=%d; server1=%s; len_max=%d;", pweb->script_name_lc, ppath, ppath->name, ppath->name_len, ppath->servers[0].psrv->name, len_max);
         mg_log_event(pweb->plog, pweb, bufferx, "mg_web: information", 0);
      }
*/
      ppath = ppath->pnext;
   }
   
   if (len_max) {
      rc = CACHE_SUCCESS;
      pweb->ppath = ppath_max;
   }
   else {
      rc = CACHE_FAILURE;
   }
      
	return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_get_path_configuration: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_add_cgi_variable(MGWEB *pweb, char *name, int name_len, char *value, int value_len)
{
   DBX_TRACE_INIT(0)
   int alloc, len;
   char *p;

#ifdef _WIN32
__try {
#endif

   alloc = (pweb->input_buf.len_alloc - pweb->input_buf.len_used);
   if ((name_len + value_len + 6) >= alloc) {
      return CACHE_FAILURE;
   }

   p = (char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used);
   p += 5;
   strncpy(p, name, name_len);
   p += name_len;
   *p = '=';
   p ++;
   strncpy(p, value, value_len);
   p += value_len;

   len = (name_len + value_len + 1);
   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) len, DBX_DSORT_WEBCGI, DBX_DTYPE_STR);
   pweb->input_buf.len_used += (len + 5);

   return CACHE_SUCCESS;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_add_cgi_variable: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int mg_obtain_connection(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, use_existing, idle_time, connections_inuse, queue_time, queue_timeout, server_busy;
   DBXCON *pcon, *pcon_last, *pcon_free;
   char bufferx[256], info[256];
   MGSRV *psrv;
   MGPATH *ppath;
   char *p, *p1, *p2;
   time_t time_now;

#ifdef _WIN32
__try {
#endif

   psrv = pweb->psrv;
   ppath = pweb->ppath;
   use_existing = 0;
   pcon_free = NULL;
   pcon_last = NULL;
   bufferx[0] = '\0';
   info[0] = '\0';

   DBX_TRACE(1)

   mg_enter_critical_section((void *) &mg_global_mutex);

   if (pweb->server_no >= 0 && pweb->server_no < 32) { /* server affinity as server number pre-defined */
      /* v2.1.16 : DB Server can be used if online and must be used if it's marked for exclusive use */
      if (pweb->server_no < ppath->srv_max && ppath->servers[pweb->server_no].psrv && (ppath->servers[pweb->server_no].psrv->offline == 0 || ppath->servers[pweb->server_no].exclusive)) { /* check server in range and online */
         psrv = ppath->servers[pweb->server_no].psrv;
      }
      else {
         /* v2.4.24 */
         if (ppath->servers[pweb->server_no].psrv)
            sprintf(bufferx, "Preferred DB Server %s not available; offline=%d;", ppath->servers[pweb->server_no].psrv->name, ppath->servers[pweb->server_no].psrv->offline);
         else
            sprintf(bufferx, "Preferred DB Server number %d not available", pweb->server_no);
         pweb->server_no = -1; /* preferred server not available, start again */
      }
   }
   else if (pweb->server_no != -1) {
      sprintf(bufferx, "Out of range DB Server number %d", pweb->server_no);
      pweb->server_no = -1; /* bad server number */
   }

   if (pweb->server_no == -1) {
      pweb->server_no = mg_obtain_server(pweb, info, 0);
      if (pweb->server_no == -1) { /* no online servers in list */
         pweb->server_no = 0;
         pweb->psrv = ppath->servers[pweb->server_no].psrv;
         mg_server_online(pweb, NULL, info, 0); /* v2.2.19 */
         mg_leave_critical_section((void *) &mg_global_mutex);
         /* v2.4.24 */
         if (bufferx[0]) {
            mg_log_event(pweb->plog, pweb, bufferx, "mg_web: connectivity error", 0);
         }
         if (info[0]) {
            mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
         }
         strcpy(pweb->error, "All available DB Servers are currently offline");
         return CACHE_FAILURE;
      }
      psrv = ppath->servers[pweb->server_no].psrv;
   }

   queue_timeout = NETX_QUEUE_TIMEOUT;
   queue_time = 0;
   server_busy = 0;
   connections_inuse = 0;
   pcon = mg_connection;
   while (pcon) {
      pcon_last = pcon;
      if (pcon->alloc && pcon->psrv == psrv) {
         if (psrv->net_connection == 1) { /* network */
            if (pcon->inuse == 0) {
               pcon->inuse = 1;
               use_existing = 1;
               break;
            }
            else { /* v2.5.30 */
               connections_inuse ++;
               if (psrv->max_connections && connections_inuse >= psrv->max_connections) {
                  if (psrv->timeout) {
                     queue_timeout = psrv->timeout;
                  }
                  if (queue_time < queue_timeout) { /* queue request */
                     mg_leave_critical_section((void *) &mg_global_mutex);
                     mg_sleep(1000);
                     mg_enter_critical_section((void *) &mg_global_mutex);
                     queue_time ++;
                     connections_inuse = 0;
                     pcon = mg_connection;
                     continue;
                  }
                  else { /* give up and return HTTP Server Busy */
                     pcon = NULL;
                     server_busy = 1;
                     break;
                  }
               }
            }
         }
         if (psrv->net_connection == 0) { /* API */
            pcon->inuse = 1;
            use_existing = 1;
            break;
         }
      }
      if (!pcon->alloc && !pcon_free) {
         pcon_free = pcon;
      }
      pcon = pcon->pnext;
   }
   if (!pcon) {
      if (!server_busy) {
         if (pcon_free) {
            pcon = pcon_free;
            pcon->alloc = 1;
            pcon->inuse = 1;
            pcon->closed = 0;
            pcon->no_requests = 0;
         }
         else {
            pcon = (DBXCON *) mg_malloc(NULL, sizeof(DBXCON), 1);
            if (pcon) {
               memset((void *) pcon, 0, sizeof(DBXCON));
               if (!mg_connection) {
                  mg_connection = pcon;
               }
               else {
                  pcon_last->pnext = pcon;
               }
               pcon->alloc = 1;
               pcon->inuse = 1;
               pcon->closed = 0;
               pcon->time_request = 0; /* v2.4.26 */
               pcon->no_requests = 0;
               pcon->int_pipe[0] = 0;
               pcon->int_pipe[1] = 0;
            }
         }
      }
   }
   if (!pweb->requestno_in) {
      pweb->requestno_in = mg_system.requestno ++;
      psrv->no_requests ++;
   }
   mg_leave_critical_section((void *) &mg_global_mutex);
/*
   {
      char buffer[256];
      sprintf(buffer, "pcon=%p; connections_inuse=%d; queue_timeout=%d; queue_time=%d; server_busy=%d;", pcon, connections_inuse, queue_timeout, queue_time, server_busy);
      mg_log_event(pweb->plog, pweb, buffer, "mg_obtain_connection", 0);
   }
*/
   /* v2.4.24 */
   if (bufferx[0]) {
      mg_log_event(pweb->plog, pweb, bufferx, "mg_web: connectivity error", 0);
   }
   if (info[0]) {
      mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
   }

   pweb->psrv = psrv;
   if (!pcon && server_busy) { /* v2.5.30 */
      strcpy(pweb->error, "Cannot connect to DB Server - All connections busy");
      return CACHE_MAXCON;
   }
   if (!pcon) {
      strcpy(pweb->error, "Unable to allocate memory for a new connection");
      return CACHE_FAILURE;
   }

   pweb->pcon = pcon;
   pweb->error[0] = '\0';
   pweb->error_code = 0;
   pweb->error_no = 0;

   /* v2.4.26 */
   idle_time = 0;
   time_now = time(NULL);
   if (psrv->net_connection == 1 && psrv->idle_timeout && pcon->time_request) { /* v2.4.28 check connetion idle timeout for network connections */
      idle_time = (int) difftime(time_now, pcon->time_request);
      if (idle_time > (psrv->idle_timeout - 5)) {
         use_existing = 0;
         netx_tcp_disconnect(pweb, 1);
/*
         {
            char buffer[256];
            sprintf(buffer, "pcon=%p; (connection within 5 seconds of idle timeout - create new connection idle_timeout=%d;idle_time=%d;)", pcon, psrv->idle_timeout, idle_time);
            mg_log_event(pweb->plog, pweb, buffer, "mg_obtain_connection", 0);
         }
*/
      }
   }
   pcon->time_request = time_now; /* v2.4.26 */

/*
   {
      char buffer[256];
      sprintf(buffer, "pcon=%p; pcon_last=%p; pcon_free=%p; (alloc=%d;inuse=%d;use_existing=%d;net_connection=%d;idle_time=%d)", pcon, pcon_last, pcon_free, pcon->alloc, pcon->inuse, use_existing, psrv->net_connection, idle_time);
      mg_log_event(pweb->plog, pweb, buffer, "mg_obtain_connection", 0);
   }
*/

   if (use_existing) {
      if (pcon->psrv->offline == 2) { /* v2.2.20 */
         char buffer[256];
         sprintf(buffer, "Previously offline DB Server %s is due for a health check; marking as online", pweb->psrv ? pweb->psrv->name : "null");
         mg_log_event(pweb->plog, pweb, buffer, "mg_web: information", 0);
         mg_server_online(pweb, pcon->psrv, info, 0);
         if (info[0]) {
            mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
         }
      }
      return CACHE_SUCCESS;
   }

   pcon->net_connection = 0;
   pcon->closed = 0;
   pcon->timeout = psrv->timeout;
   pcon->current_timeout = 0;

/*
   {
      char buffer[256];
      sprintf(buffer, "pcon=%p; shdir=%p; ip_address=%p; port=%d;", pcon, psrv->shdir, psrv->ip_address, psrv->port);
      mg_log_event(pweb->plog, pweb, buffer, "mg_obtain_connection : New Connection", 0);
   }
*/

   pcon->p_isc_so = NULL;
   pcon->p_ydb_so = NULL;
   pcon->psrv = psrv;
   pcon->p_db_mutex = &pcon->db_mutex;
   mg_mutex_create(pcon->p_db_mutex);
   pcon->p_zv = &pcon->zv;

   pcon->pid = 0;
  
   if (pcon->psrv->penv) {
      p = (char *) pcon->psrv->penv->p_buffer;
      p2 = strstr(p, "\n");
      while (p2) {
         *p2 = '\0';
         p1 = strstr(p, "=");
         if (p1) {
            *p1 = '\0';
            p1 ++;
#if defined(_WIN32)
            SetEnvironmentVariable((LPCTSTR) p, (LPCTSTR) p1);
#else
            /* printf("\nLinux : environment variable p=%s p1=%s;", p, p1); */
            setenv(p, p1, 1);
#endif
         }
         else {
            break;
         }
         p = p2 + 1;
         p2 = strstr(p, "\n");
      }
   }

   if (!pcon->psrv->dbtype) {
      strcpy(pweb->error, "Unable to determine the database type");
      rc = CACHE_NOCON;
      pcon->alloc = 0;
      pcon->inuse = 0;
      pweb->pcon = NULL;
      return rc;
   }

   rc = mg_connect(pweb, 0);

   if (rc == CACHE_SUCCESS) {
      if (pcon->psrv->offline == 2) { /* v2.2.20 */
         char buffer[256];
         sprintf(buffer, "Previously offline DB Server %s has passed its health check; marking as online", pweb->psrv ? pweb->psrv->name : "null");
         mg_log_event(pweb->plog, pweb, buffer, "mg_web: information", 0);
         mg_server_online(pweb, pcon->psrv, info, 0);
         if (info[0]) {
            mg_log_event(pweb->plog, pweb, info, "mg_web: connectivity", 0);
         }
      }
   }
   else {
      pcon->alloc = 0;
      pcon->inuse = 0;
      pweb->pcon = NULL;
   }

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_obtain_connection: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_obtain_server(MGWEB *pweb, char *info, int context)
{
   DBX_TRACE_INIT(0)
   int server_no, server_no_start;
   time_t time_now;

#ifdef _WIN32
__try {
#endif

   time_now = time(NULL);
   server_no_start = pweb->ppath->server_no;
   server_no = -1;
   DBX_TRACE(1)
   while (server_no == -1) {

      /* v2.1.16 */
/*
      {
         char buffer[256];
         sprintf(buffer, "pweb->ppath->server_no=%d; psrv->offline=%d; psrv->health_check=%d; exclusive=%d; time_offline=%d", pweb->ppath->server_no, pweb->ppath->servers[pweb->ppath->server_no].psrv->offline, pweb->ppath->servers[pweb->ppath->server_no].psrv->health_check, pweb->ppath->servers[pweb->ppath->server_no].exclusive, pweb->ppath->servers[pweb->ppath->server_no].psrv->offline ? (int) difftime(time_now, pweb->ppath->servers[pweb->ppath->server_no].psrv->time_offline) : 0);
         mg_log_event(pweb->plog, pweb, buffer, "mg_obtain_server", 0);
      }
*/
      /* v2.2.20 */
      if (pweb->ppath->servers[pweb->ppath->server_no].exclusive == 0) {
         if (pweb->ppath->servers[pweb->ppath->server_no].psrv->offline == 1 && pweb->ppath->servers[pweb->ppath->server_no].psrv->health_check > 0 && difftime(time_now, pweb->ppath->servers[pweb->ppath->server_no].psrv->time_offline) > pweb->ppath->servers[pweb->ppath->server_no].psrv->health_check) {

            DBX_TRACE(2)
            if (info) {
               sprintf(info, "Retry DB Server %s; number=%d; health_check=%d; time_offline=%d;", pweb->ppath->servers[pweb->ppath->server_no].psrv->name, pweb->ppath->server_no, pweb->ppath->servers[pweb->ppath->server_no].psrv->health_check, (int) difftime(time_now, pweb->ppath->servers[pweb->ppath->server_no].psrv->time_offline));
            }
            pweb->ppath->servers[pweb->ppath->server_no].psrv->time_offline = time(NULL); /* push time offline forward so only this request tries */
            pweb->ppath->servers[pweb->ppath->server_no].psrv->offline = 2; /* indicate that we're retrying this offline server */
            server_no = pweb->ppath->server_no;
         }
         else if (pweb->ppath->servers[pweb->ppath->server_no].psrv->offline == 0) { /* server online so we can use it */
            DBX_TRACE(3)
            server_no = pweb->ppath->server_no;
         }
      }
/*
      if (pweb->ppath->servers[pweb->ppath->server_no].psrv->offline == 0 && pweb->ppath->servers[pweb->ppath->server_no].exclusive == 0) {
         server_no = pweb->ppath->server_no;
      }
*/
      DBX_TRACE(4)
      if (pweb->ppath->load_balancing || pweb->ppath->servers[pweb->ppath->server_no].psrv->offline > 0) { /* increment core server number if one is offline or load balancing is on */
         pweb->ppath->server_no ++;
         if (!pweb->ppath->servers[pweb->ppath->server_no].psrv) {
            pweb->ppath->server_no = 0;
         }
      }
      if (pweb->ppath->server_no == server_no_start) { /* all servers examined */
         break;
      }
   }

   DBX_TRACE(10)
   return server_no;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_obtain_server: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return -1;
}
#endif
}


int mg_server_offline(MGWEB *pweb, MGSRV *psrv, char *info, int context)
{
   DBX_TRACE_INIT(0)
   DBXCON *pcon;

#ifdef _WIN32
__try {
#endif

   if (context == 10) { /* from admin */
      if (!psrv) {
         return -1;
      }
   }
   else {
      if (!psrv || !pweb->ppath) {
         return -1;
      }
      if (!pweb->ppath->servers[1].psrv) { /* only one server, therefore no failover */
         return -1;
      }
   }

   DBX_TRACE(1)
   /* block new connections to this server */
   mg_enter_critical_section((void *) &mg_global_mutex);

   psrv->offline = 1;
   psrv->time_offline = time(NULL); /* v2.2.20 */

   mg_leave_critical_section((void *) &mg_global_mutex);

   /* v2.4.24 */
   if (info) {
      sprintf(info, "DB Server %s marked offline; context=%d;", psrv->name, context);
   }

   DBX_TRACE(2)
   pcon = mg_connection;
   while (pcon) {
      if (pcon->alloc && pcon->psrv == psrv) {
         if (psrv->net_connection == 1 && pcon->inuse == 0) { /* network */
            DBX_TRACE(3)
            mg_release_connection(pweb, 1);
         }
      }
      pcon = pcon->pnext;
   }
   DBX_TRACE(10)

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_server_offline: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


/* v2.2.19 */
int mg_server_online(MGWEB *pweb, MGSRV *psrv, char *info, int context)
{
   DBX_TRACE_INIT(0)
   int server_no;

#ifdef _WIN32
__try {
#endif

   if (psrv) { /* just mark one server as online */
      if (context) {
         /* block new connections to this server */
         mg_enter_critical_section((void *) &mg_global_mutex);
      }
      psrv->offline = 0;
      if (context) {
         mg_leave_critical_section((void *) &mg_global_mutex);
      }

      /* v2.4.24 */
      if (info) {
         sprintf(info, "DB Server %s marked online; context=%d;", psrv->name, context);
      }

      return 0;
   }

   if (!pweb->ppath) {
      return -1;
   }
   if (!pweb->ppath->servers[1].psrv) { /* only one server, therefore no server list for failover */
      return -1;
   }

   if (context) {
      /* block new connections to this server */
      mg_enter_critical_section((void *) &mg_global_mutex);
   }
   for (server_no = 0; pweb->ppath->servers[server_no].psrv; server_no ++) {
      pweb->ppath->servers[server_no].psrv->offline = 0;
   }
   pweb->ppath->server_no = 0;
   if (context) {
      mg_leave_critical_section((void *) &mg_global_mutex);
   }

   /* v2.4.24 */
   if (info) {
      sprintf(info, "All DB Servers marked online; context=%d;", context);
   }

/*
   mg_log_event(pweb->plog, pweb, "Mark all servers for this path as 'online' following an 'All DB Servers offline' event", "mg_server_online", 0);
*/

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_server_online: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_connect(MGWEB *pweb, int context)
{
   DBX_TRACE_INIT(0)
   int rc, con_retry_no, con_retry_time;
   char bufferx[256];
   DBXCON *pcon;
   time_t time_start, time_now;

#ifdef _WIN32
__try {
#endif

   /* v2.4.25 */
   time_start = time(NULL);
   con_retry_no = 0;
   con_retry_time = 0;
   pweb->mg_connect_failed = 0;
   pcon = pweb->pcon;

   rc = CACHE_SUCCESS;
   if (!pcon->psrv->shdir && pcon->psrv->ip_address && pcon->psrv->port) {

      while (1) {
         rc = netx_tcp_connect(pweb, 0);
/*
         {
            char buffer[256];
            sprintf(buffer, "network rc=%d;", rc);
            mg_log_event(pweb->plog, pweb, buffer, "mg_connect: New Connection", 0);
         }
*/
         if (rc == CACHE_SUCCESS) {
            rc = netx_tcp_handshake(pweb, 0);
/*
            {
               char buffer[256];
               sprintf(buffer, "network handshake rc=%d;", rc);
               mg_log_event(pweb->plog, pweb, buffer, "mg_connect: New Connection", 0);
            }
*/
            if (rc == CACHE_SUCCESS) {
               break;
            }
         }
         /* see if we can retry connecting */
         con_retry_no ++;
         if (pcon->psrv->con_retry_no > 0 && con_retry_no > pcon->psrv->con_retry_no) {
            con_retry_no = pcon->psrv->con_retry_no;
            sprintf(bufferx, "Cannot connect to DB Server %s after %d %s", pcon->psrv->name, con_retry_no, con_retry_no == 1 ? "attempt" : "attempts");
            mg_log_event(pweb->plog, pweb, bufferx, "mg_connect: new connection attempt", 0);
            break;
         }
         else if (pcon->psrv->con_retry_time > 0) {
            time_now = time(NULL);
            con_retry_time = (int) difftime(time_now, time_start);
            if (con_retry_time > pcon->psrv->con_retry_time) {
               sprintf(bufferx, "Cannot connect to DB Server %s after %d seconds", pcon->psrv->name, con_retry_time);
               mg_log_event(pweb->plog, pweb, bufferx, "mg_connect: new connection attempt", 0);
               break;
            }
         }
         else {
            sprintf(bufferx, "Cannot connect to DB Server %s after %d %s", pcon->psrv->name, con_retry_no, con_retry_no == 1 ? "attempt" : "attempts");
            mg_log_event(pweb->plog, pweb, bufferx, "mg_connect: new connection attempt", 0);
            break;
         }
         /* backoff slightly after each retry */
         if (con_retry_no < 5) {
            mg_sleep((unsigned long) con_retry_no * 1000);
         }
         else {
            mg_sleep((unsigned long) 5000);
         }
      }

      if (rc != CACHE_SUCCESS) {
         pweb->mg_connect_failed = 1;
         pcon->alloc = 0;
         pcon->closed = 1;
         rc = CACHE_NOCON;
         mg_error_message(pweb, rc);
         return rc;
      }
 
      pcon->alloc = 1;
      pcon->net_connection = 1; /* network connection */
      return rc;
   }

   if (!pcon->psrv->shdir) {
      strcpy(pweb->error, "Unable to determine the path to the database installation");
      rc = CACHE_NOCON;
      return rc;
   }

   pcon->use_db_mutex = 1;

   DBX_LOCK(0);
   if (pcon->psrv->dbtype != DBX_DBTYPE_YOTTADB) {
      rc = isc_open(pweb);

      if (rc == CACHE_SUCCESS) {
         pcon->alloc = 1;
      }
      else {
         pcon->alloc = 0;
      }
   }
   else {
      rc = ydb_open(pweb);

      if (rc == CACHE_SUCCESS) {
         pcon->alloc = 1;
      }
      else {
         pcon->alloc = 0;
      }
   }
   DBX_UNLOCK();

   if (pcon->alloc) {
      return CACHE_SUCCESS;
   }
   else {
      pweb->mg_connect_failed = 1;
      return CACHE_NOCON;
   }

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_connect: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_NOCON;
}
#endif
}


int mg_release_connection(MGWEB *pweb, int close_connection)
{
   DBX_TRACE_INIT(0)
   int rc;
   DBXCON *pcon;

#ifdef _WIN32
__try {
#endif

   pcon = pweb->pcon;

   if (!pcon) {
      return CACHE_FAILURE;
   }

   rc = CACHE_SUCCESS;
   if (close_connection == 0) {
      pcon->inuse = 0;
      return rc;
   }

   if (pcon->net_connection) {
      rc = netx_tcp_disconnect(pweb, close_connection);
   }
   else if (pcon->psrv->dbtype == DBX_DBTYPE_YOTTADB) {
      if (pcon->p_ydb_so->loaded) {
         rc = pcon->p_ydb_so->p_ydb_exit();
         /* printf("\r\np_ydb_exit=%d\r\n", rc); */
      }

      strcpy(pweb->error, "");

      if (pcon->p_ydb_so) { /* v2.8.39 */
/*
         mg_dso_unload(pcon->p_ydb_so->p_library); 
         pcon->p_ydb_so->p_library = NULL;
         pcon->p_ydb_so->loaded = 0;
*/
         strcpy(pcon->p_ydb_so->libdir, "");
         strcpy(pcon->p_ydb_so->libnam, "");
      }
   }
   else {
      if (pcon->p_isc_so->loaded) {
         DBX_LOCK(0);
         rc = pcon->p_isc_so->p_CacheEnd();
         DBX_UNLOCK();
      }

      strcpy(pweb->error, "");

      if (pcon->p_isc_so) { /* v2.8.39 */
         mg_dso_unload(pcon->p_isc_so->p_library); 

         pcon->p_isc_so->p_library = NULL;
         pcon->p_isc_so->loaded = 0;

         strcpy(pcon->p_isc_so->libdir, "");
         strcpy(pcon->p_isc_so->libnam, "");
      }
   }

   pcon->net_connection = 0;
   pcon->closed = 1;
   pcon->inuse = 0;
   pcon->alloc = 0;

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_release_connection: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


MGWEB * mg_obtain_request_memory(void *pweb_server, unsigned long request_clen, int request_chunked, int wstype)
{
   DBX_TRACE_INIT(0)
   unsigned int len_alloc;
   MGWEB *pweb;

#ifdef _WIN32
__try {
#endif

   /* v2.8.37 */
/*
   {
      char buffer[256];
      sprintf(buffer, "mg_obtain_request_memory: request_clen=%lu; request_chunked=%d;", request_clen, request_chunked);
      mg_log_event(mg_system.plog, NULL, buffer, "mg_obtain_request_memory", 0);
   }
*/

   len_alloc = 128000 + request_clen;
   if (len_alloc < mg_system.request_buffer_size) { /* v2.1.10 */
      len_alloc = mg_system.request_buffer_size;
   }

   if (len_alloc > DBX_LS_MAXSIZE_ISC) { /* v2.2.18 */
      len_alloc = DBX_LS_BUFFER_ISC;
   }

   pweb = (MGWEB *) mg_malloc(pweb_server, sizeof(MGWEB) + (len_alloc + 32), 2); /* v2.5.31 */
   if (!pweb) {
      return NULL;
   }

   memset((void *) pweb, 0, sizeof(MGWEB) + DBX_IBUFFER_OFFSET); /* v2.3.22 */

   pweb->input_buf.buf_addr = ((char *) pweb) + sizeof(MGWEB);
   pweb->input_buf.buf_addr += DBX_IBUFFER_OFFSET;
   pweb->input_buf.len_alloc = len_alloc;
   pweb->input_buf.len_used = 0;

   pweb->output_val.svalue.buf_addr = pweb->input_buf.buf_addr;
   pweb->output_val.svalue.len_alloc = pweb->input_buf.len_alloc;
   pweb->output_val.svalue.len_used = pweb->input_buf.len_used;
   pweb->output_val.pnext = NULL;
   pweb->poutput_val_last = NULL;
   pweb->request_cookie = NULL;

   pweb->wstype = wstype; /* v2.7.33 */
   pweb->evented = 0;
   pweb->wserver_chunks_response = 0;

   pweb->error[0] = '\0';
   pweb->error_code = 0;
   pweb->error_no = 0;

   pweb->pcon = NULL;
   pweb->psrv = NULL;
   pweb->ppath = NULL;

   pweb->request_clen = (int) request_clen;
   pweb->request_chunked = (int) request_chunked; /* v2.8.37 */
   pweb->request_read_status = 0;
/*
   {
      char buffer[256];
      sprintf(buffer, "mg_obtain_request_memory: pweb=%p; request_clen=%lu;", pweb, pweb->request_clen);
      mg_log_event(mg_system.plog, pweb, buffer, "mg_obtain_request_memory", 0);
   }
*/
   return pweb;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_obtain_request_memory: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return NULL;
}
#endif
}


DBXVAL * mg_extend_response_memory(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   unsigned int len_alloc;
   DBXVAL *pval;

#ifdef _WIN32
__try {
#endif

   len_alloc = 128000;

   pval = (DBXVAL *) mg_malloc(pweb->pweb_server, sizeof(DBXVAL) + (len_alloc + 32), 3); /* v2.5.31 */
   if (!pval) {
      return NULL;
   }
   memset((void *) pval, 0, sizeof(DBXVAL));

   pval->svalue.buf_addr = ((char *) pval) + sizeof(DBXVAL);
   pval->svalue.buf_addr += DBX_IBUFFER_OFFSET;
   pval->svalue.len_alloc = len_alloc;
   pval->svalue.len_used = 0;
   pval->pnext = NULL;

   if (pweb->poutput_val_last) {
      pweb->poutput_val_last->pnext = pval;
   }
   else {
      pweb->output_val.pnext = pval;
   }
   pweb->poutput_val_last = pval;

   return pval;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_extend_response_memory: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return NULL;
}
#endif
}


int mg_release_request_memory(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   DBXVAL *pval, *pvalnext;

#ifdef _WIN32
__try {
#endif

   if (!pweb) {
      return 0;
   }

   pval = pweb->output_val.pnext;
   while (pval) {
      pvalnext = pval->pnext;
      mg_free(pweb->pweb_server, (void *) pval, 1); /* v2.5.31 */
      pval = pvalnext; 
   }
   if (pweb->request_cookie) { /* v2.1.10 */
      mg_free(pweb->pweb_server, (void *) pweb->request_cookie, 2); /* v2.5.31 */
   }

   mg_free(pweb->pweb_server, pweb, 3);

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_release_request_memory: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}

/* v2.4.24 */
int mg_find_sa_variable(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, n, nb, server_no, len, name_len;
   char *p;

#ifdef _WIN32
__try {
#endif

   server_no = -1;

/*
{
   char bufferx[1024];
   sprintf(bufferx, "HTTP Request: pweb=%p; psrv=%p; ppath=%p;", (void *) pweb, (void *) pweb->psrv, (void *) pweb->ppath);
   mg_log_buffer(pweb->plog, pweb, pweb->input_buf.buf_addr, pweb->input_buf.len_used, bufferx, 0);
}
*/
/*
{
   char bufferx[1024];
   sprintf(bufferx, "HTTP Request: pweb=%p; psrv=%p; ppath=%p;", (void *) pweb, (void *) pweb->psrv, (void *) pweb->ppath);
   mg_log_buffer(pweb->plog, pweb, pweb->request_content, pweb->request_clen, bufferx, 0);
}
*/

   DBX_TRACE(1)
   for (n = 0; pweb->ppath->sa_variables[n]; n ++) {
      name_len = (int) strlen(pweb->ppath->sa_variables[n]);
      if (pweb->query_string && pweb->query_string_len) {
         DBX_TRACE(2)
         server_no = mg_find_sa_variable_ex(pweb, pweb->ppath->sa_variables[n], name_len, (unsigned char *) pweb->query_string, pweb->query_string_len);
         if (server_no != -1) {
            break;
         }
      }
      if (pweb->request_content && pweb->request_clen) {
         DBX_TRACE(10)
         rc = MG_CGI_SUCCESS;
         if (!pweb->request_content_type[0]) {
            len = 1020;
            DBX_TRACE(11)
            rc = mg_get_cgi_variable(pweb, "CONTENT_TYPE", pweb->request_content_type, &len);
            DBX_TRACE(12)
            if (rc == MG_CGI_SUCCESS) { /* v2.1.15 */
               p = strstr(pweb->request_content_type, "boundary=");
               if (p) {
                  p += 9;
                  if ((*p) == '\"') {
                     p ++;
                  }
                  nb = 0;
                  while ((*p)) {
                     if ((*p) == '\"' || (*p) == ';' || (*p) == ' ') {
                        pweb->boundary[nb] = '\0';
                        break;
                     }
                     else {
                        pweb->boundary[nb ++] = *p ++;
                     }
                  }
               }
            }
         }

         DBX_TRACE(13)
         if (rc == MG_CGI_SUCCESS) {
            if (strstr(pweb->request_content_type, "application/x-www-form-urlencoded")) {
               DBX_TRACE(20)
               server_no = mg_find_sa_variable_ex(pweb, pweb->ppath->sa_variables[n], name_len, (unsigned char *) pweb->request_content, pweb->request_clen);
               if (server_no != -1) {
                  break;
               }
            }
            else if (pweb->boundary[0]) { /* v2.1.15 */
               DBX_TRACE(30)
               server_no = mg_find_sa_variable_ex_mp(pweb, pweb->ppath->sa_variables[n], name_len, (unsigned char *) pweb->request_content, pweb->request_clen);
               if (server_no != -1) {
                  break;
               }
            }
         }
      }
   }

   DBX_TRACE(40)
   if (server_no != -1) {
      pweb->server_no = server_no;
   }

   return server_no;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_find_sa_variable: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return -1;
}
#endif
}

/* v2.4.24 */
int mg_find_sa_variable_ex(MGWEB *pweb, char *name, int name_len, unsigned char *nvpairs, int nvpairs_len)
{
   DBX_TRACE_INIT(0)
   int server_no, server_no0, n, len, found;
   unsigned char cnvz;
   char *pn, *pv, *pz;
   char sname[64];

#ifdef _WIN32
__try {
#endif

   cnvz = nvpairs[nvpairs_len];
   nvpairs[nvpairs_len] = '\0';
   server_no = -1;

   *sname = '\0'; 
   found = 0;
   DBX_TRACE(1)
   pn = strstr((char *) nvpairs, name);
   while (pn) {
      DBX_TRACE(2)
      if (pn == (char *) nvpairs || *(pn - 1) == '&') {
         if (*(pn + name_len) == '=') {
            pv = pn + name_len + 1;
            pz = strstr(pv, "&");
            if (pz) {
               *pz = '\0';
            }
            len = (int) strlen(pv);
            found = 1;

            if (((int) *pv) >= 48 && ((int) *pv) <= 57) {
               DBX_TRACE(3)
               server_no0 = (int) strtol(pv, NULL, 10);
               if (server_no0 < 32 && pweb->ppath->servers[server_no0].name) { /* v2.3.23 */
                  server_no = server_no0;
                  strcpy(sname, pweb->ppath->servers[server_no].lcname);
               }
            }
            else if (len < 60) { /* v2.1.17 */
               /* v2.1.17 */
               DBX_TRACE(10)
               strcpy(sname, pv);
               mg_lcase(sname);
               for (n = 0; pweb->ppath->servers[n].name; n ++) {
                  DBX_TRACE(11)
                  if (!strcmp(pweb->ppath->servers[n].lcname, sname)) {
                     server_no = n;
                     break;
                  }
               }
            }

            if (pz) {
               *pz = '&';
            }
         }
      }
      if (server_no != -1) {
         break;
      }
      DBX_TRACE(12)
      pn = strstr(pn + name_len, name);
   }

   DBX_TRACE(20)
   nvpairs[nvpairs_len] = cnvz;

   if (pweb->plog->log_verbose) { /* v2.3.23 */
      int len;
      char bufferx[256];

      DBX_TRACE(50)
      len = nvpairs_len;
      if (nvpairs_len > 511) {
         len = 511;
      }
      if (found) {
         if (server_no != -1) {
            sprintf(bufferx, "mg_web:mg_find_sa_variable_ex: Server Affinity Variable '%s' found; Server number to use: %d (%s)%s", name, server_no, sname, (len == nvpairs_len) ? "" : " First 511 Bytes of request content ...");
            mg_log_buffer(pweb->plog, pweb, (char *) nvpairs, len, bufferx, 0);
         }
         else {
            sprintf(bufferx, "mg_web:mg_find_sa_variable_ex: Server Affinity Variable '%s' found but set to a non-valid value%s", name, (len == nvpairs_len) ? "" : " First 511 Bytes of request content ...");
            mg_log_buffer(pweb->plog, pweb, (char *) nvpairs, len, bufferx, 0);
         }
      }
      else {
         sprintf(bufferx, "mg_web:mg_find_sa_variable_ex: Server Affinity Variable '%s' not found%s", name, (len == nvpairs_len) ? "" : " First 511 Bytes of request content ...");
         mg_log_buffer(pweb->plog, pweb, (char *) nvpairs, len, bufferx, 0);
      }
   }

   DBX_TRACE(100)
   return server_no;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_find_sa_variable_ex: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return -1;
}
#endif
}


/* v2.1.15 v2.4.24 */
int mg_find_sa_variable_ex_mp(MGWEB *pweb, char *name, int name_len, unsigned char *content, int content_len)
{
   DBX_TRACE_INIT(0)
   int server_no, server_no0, n, nc, nn, blen, len, found;
   char *ph, *phz, *pn, *pv, *pz;
   char cname[64], sname[64];

#ifdef _WIN32
__try {
#endif

/*
   mg_log_buffer(pweb->plog, pweb, (char *) content, content_len, "Multipart Payload", 0);
*/
   *sname = '\0';
   found = 0;
   server_no = -1;
   DBX_TRACE(1)
   blen = (int) strlen(pweb->boundary);
   for (nc = 0; nc < (content_len - 3); nc ++) {
      DBX_TRACE(2)
      if (content[nc] == '-' && content[nc + 1] == '-') {
         DBX_TRACE(3)
         if (!strncmp((char *) (content + (nc + 2)), pweb->boundary, blen)) {
            DBX_TRACE(4)
            ph = (char *) (content + (nc + blen + 4)); /* section header starts here */
            phz = strstr(ph, "\r\n\r\n"); /* section header ends here */
            if (phz) {
               DBX_TRACE(5)
               *phz = '\0';
               pv = (phz + 4); /* section value starts here */
               pz = strstr(pv, "\r\n"); /* section value ends here */
               if (pz) {
                  *pz = '\0';
                  pn = strstr(ph, "name=");
                  if (pn) {
                     pn += 5;
                     if ((*pn) == '\"') {
                        pn ++;
                     }
                     nn = 0;
                     cname[0] = '\0';
                     while ((*pn)) {
                        if ((*pn) == '\"' || (*pn) == ';' || (*pn) == ' ' || nn > 60) {
                           cname[nn] = '\0';
                           break;
                        }
                        else {
                           cname[nn ++] = *pn ++;
                        }
                     }
/*
                     {
                        char bufferx[1024];
                        sprintf(bufferx, "A section header=%s; name=%s", ph, cname);
                        mg_log_event(pweb->plog, pweb, (char *) pv, bufferx, 0);
                     }
*/
                     DBX_TRACE(6)
                     if (!strcmp(name, cname)) {
                        found = 1;
                        len = (int) strlen(pv);
                        if (((int) *pv) >= 48 && ((int) *pv) <= 57) {
                           DBX_TRACE(10)
                           server_no0 = (int) strtol(pv, NULL, 10);
                           if (server_no0 < 32 && pweb->ppath->servers[server_no0].name) { /* v2.3.23 */
                              server_no = server_no0;
                              strcpy(sname, pweb->ppath->servers[server_no].lcname);
                           }
                        }
                        else if (len < 60) { /* v2.1.17 */
                           DBX_TRACE(20)
                           strcpy(sname, pv);
                           mg_lcase(sname);
                           for (n = 0; pweb->ppath->servers[n].name; n ++) {
                              if (!strcmp(pweb->ppath->servers[n].lcname, sname)) {
                                 server_no = n;
                                 break;
                              }
                           }
                        }
                     }
                  }
                  *pz = '\r';
               }
               *phz = '\r';
            }
            if (server_no != -1) {
               break;
            }
            nc += blen;
         }
      }
   }

/*
   {
      char bufferx[256];
      if (server_no == -1) {
         sprintf(bufferx, "mg_find_sa_variable_ex_mp: name=%s; server_no=%d (not found)", name, server_no);
         mg_log_buffer(pweb->plog, pweb, (char *) content, content_len, bufferx, 0);
      }
      else {
         sprintf(bufferx, "mg_find_sa_variable_ex_mp: name=%s; server_no=%d", name, server_no);
         mg_log_buffer(pweb->plog, pweb, (char *) content, content_len, bufferx, 0);
      }
   }
*/
   DBX_TRACE(30)
   if (pweb->plog->log_verbose) { /* v2.3.23 */
      int len;
      char bufferx[256];

      DBX_TRACE(50)
      len = content_len;
      if (content_len > 511) {
         len = 511;
      }
      if (found) {
         if (server_no != -1) {
            sprintf(bufferx, "mg_web:mg_find_sa_variable_ex_mp: Server Affinity Variable '%s' found; Server number to use: %d (%s)%s", name, server_no, sname, (len == content_len) ? "" : " First 511 Bytes of request content ...");
            mg_log_buffer(pweb->plog, pweb, (char *) content, len, bufferx, 0);
         }
         else {
            sprintf(bufferx, "mg_web:mg_find_sa_variable_ex_mp: Server Affinity Variable '%s' found but set to a non-valid value%s", name, (len == content_len) ? "" : " First 511 Bytes of request content ...");
            mg_log_buffer(pweb->plog, pweb, (char *) content, len, bufferx, 0);
         }
      }
      else {
         sprintf(bufferx, "mg_web:mg_find_sa_variable_ex_mp: Server Affinity Variable '%s' not found%s", name, (len == content_len) ? "" : " First 511 Bytes of request content ...");
         mg_log_buffer(pweb->plog, pweb, (char *) content, len, bufferx, 0);
      }
   }

   DBX_TRACE(100)
   return server_no;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_find_sa_variable_ex_mp: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return -1;
}
#endif
}

/* v2.4.24 */
int mg_find_sa_cookie(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int server_no, server_no0, n, rc, len, name_len, found;
   char *pn, *pv, *pz;
   char sname[64];

#ifdef _WIN32
__try {
#endif

   server_no = -1;

   name_len = (int) strlen(pweb->ppath->sa_cookie);

   *sname = '\0';
   found = 0;
   len = 4096;
   pweb->request_cookie = (char *) mg_malloc(pweb->pweb_server, len + 32, 4); /* v2.5.31 */
   DBX_TRACE(1)
   if (!pweb->request_cookie) {
      return server_no;
   }
   for (n = 0; n < 3; n ++) {
      DBX_TRACE(2)
      rc = mg_get_cgi_variable(pweb, "HTTP_COOKIE", pweb->request_cookie, &len);

      DBX_TRACE(3)
      if (rc == MG_CGI_TOOLONG) {
         DBX_TRACE(4)
         mg_free(NULL, (void *) pweb->request_cookie, 4);
         len = (n + 2) * 4096;
         pweb->request_cookie = (char *) mg_malloc(pweb->pweb_server, len + 32, 5);
         if (!pweb->request_cookie) {
            break;
         }
         continue;
      }
      break;
   }
   if (rc != MG_CGI_SUCCESS || !pweb->request_cookie) {
      return server_no;
   }

   DBX_TRACE(5)
   pn = strstr(pweb->request_cookie, pweb->ppath->sa_cookie);
   while (pn) {
      DBX_TRACE(10)
      if (pn == pweb->request_cookie || *(pn - 1) == ';' || *(pn - 1) == ' ') {
         DBX_TRACE(11)
         if (*(pn + name_len) == '=') {
            DBX_TRACE(12)
            pv = pn + name_len + 1;
            pz = strstr(pv, ";");
            if (pz) {
               *pz = '\0';
            }

            found = 1;
            len = (int) strlen(pv);
            if (((int) *pv) >= 48 && ((int) *pv) <= 57) {
               DBX_TRACE(20)
               server_no0 = (int) strtol(pv, NULL, 10);
               if (server_no0 < 32 && pweb->ppath->servers[server_no0].name) { /* v2.3.23 */
                  server_no = server_no0;
                  strcpy(sname, pweb->ppath->servers[server_no].lcname);
               }
            }
            else if (len < 60) { /* v2.3.23 */
               DBX_TRACE(30)
               strcpy(sname, pv);
               mg_lcase(sname);
               for (n = 0; pweb->ppath->servers[n].name; n ++) {
                  if (!strcmp(pweb->ppath->servers[n].lcname, sname)) {
                     server_no = n;
                     break;
                  }
               }
            }

            if (pz) {
               *pz = ';';
            }
         }
      }
      if (server_no != -1) {
         break;
      }
      DBX_TRACE(40)
      pn = strstr(pn + name_len, pweb->ppath->sa_cookie);
   }

/*
   {
      char bufferx[256];
      if (server_no == -1) {
         sprintf(bufferx, "mg_find_sa_cookie: name=%s; server_no=%d (not found)", pweb->ppath->sa_cookie, server_no);
         mg_log_buffer(pweb->plog, pweb, pweb->request_cookie, (int) strlen(pweb->request_cookie), bufferx, 0);
      }
      else {
         sprintf(bufferx, "mg_find_sa_cookie: name=%s; server_no=%d", pweb->ppath->sa_cookie, server_no);
         mg_log_buffer(pweb->plog, pweb, pweb->request_cookie, (int) strlen(pweb->request_cookie), bufferx, 0);
      }
   }
*/

   if (pweb->plog->log_verbose) { /* v2.3.23 */
      char bufferx[256];

      DBX_TRACE(50)
      if (found) {
         if (server_no != -1) {
            sprintf(bufferx, "mg_web:mg_find_sa_cookie: Server Affinity Cookie '%s' found; Server number to use: %d (%s)", pweb->ppath->sa_cookie, server_no, sname);
            mg_log_buffer(pweb->plog, pweb, (char *) pweb->request_cookie, (int) strlen(pweb->request_cookie), bufferx, 0);
         }
         else {
            sprintf(bufferx, "mg_web:mg_find_sa_cookie: Server Affinity Cookie '%s' found but set to a non-valid value", pweb->ppath->sa_cookie);
            mg_log_buffer(pweb->plog, pweb, (char *) pweb->request_cookie, (int) strlen(pweb->request_cookie), bufferx, 0);
         }
      }
      else {
         sprintf(bufferx, "mg_web:mg_find_sa_cookie: Server Affinity Cookie '%s' not found", pweb->ppath->sa_cookie);
         mg_log_buffer(pweb->plog, pweb, (char *) pweb->request_cookie, (int) strlen(pweb->request_cookie), bufferx, 0);
      }
   }

   DBX_TRACE(100)
   return server_no;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_find_sa_cookie: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(pweb->plog, pweb, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return -1;
}
#endif
}


int mg_worker_init()
{
   DBX_TRACE_INIT(0)
#if defined(_WIN32)
   int n;
#else
   char uuser[32], ugroup[32];
   uid_t uid;
   gid_t gid;
   struct passwd *pwd;
   struct group *grp;
#endif
   int len, lenx;
   unsigned int size, size_default;
   unsigned long count;
   char *pa, *pz;
   char buffer[2048];
   FILE *fp;

#ifdef _WIN32
__try {
#endif

#if defined(_WIN32)
   if (!mg_system.config_file[0]) {
      lenx = 0;
      if (mg_system.module_file[0]) {
         len = (int) strlen(mg_system.module_file);
         for (n = len; n > 0; n --) {
            if (mg_system.module_file[n] == '\\' || mg_system.module_file[n] == '/') {
               lenx = n;
               break;
            }
         }
      }
      if (lenx) {
         strncpy(mg_system.config_file, mg_system.module_file, lenx);
         mg_system.config_file[lenx] = '\0';
         strcat(mg_system.config_file, "/");

         strcpy(mg_system.log.log_file, mg_system.config_file);

         strcat(mg_system.config_file, "mgweb.conf");
         strcat(mg_system.log.log_file, "mgweb.log");
      }
   }
#endif

   /* v2.7.35 */
#if defined(_WIN32)
   sprintf(buffer, "configuration: %s", mg_system.config_file);
#else
   *uuser = '\0';
   *ugroup = '\0';

   uid = getuid();
   gid = getgid();
   if ((pwd = getpwuid(uid)) != NULL) {
      strncpy(uuser, pwd->pw_name, 30);
      uuser[30] = '\0';
   }
   if ((grp = getgrgid(gid)) != NULL) {
      strncpy(ugroup, grp->gr_name, 30);
      ugroup[30] = '\0';
   }
   sprintf(buffer, "configuration: %s (user=%s; group=%s)", mg_system.config_file, uuser, ugroup);
#endif

   mg_log_event(&(mg_system.log), NULL, buffer, "mg_web: worker initialization", 0);

   mg_system.chunking = 1; /* 2.0.8 */
   strncpy(mg_system.cgi_base, DBX_CGI_BASE, 60);
   mg_system.cgi_base[60] = '\0';
   mg_system.cgi_max = 0;
   pa = mg_system.cgi_base;
   pz = strstr(pa, "\n");
   while (pz && *pa) {
      *pz = '\0';
      mg_system.cgi[mg_system.cgi_max ++] = pa;
      if (mg_system.cgi_max > 8) {
         break;
      }
      pa = (pz + 1);
      pz = strstr(pa, "\n");
   }

   size = mg_file_size(mg_system.config_file);
   if (size > 64000) {
      sprintf(mg_system.config_error, "Oversize configuration file (%d Bytes)", size);
      mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
      size = 0;
   }
   size_default = (unsigned int) strlen(MG_DEFAULT_CONFIG);
/*
{
   char bufferx[256];
   sprintf(bufferx, "config file size: %d; default: %d", size, size_default);
   mg_log_event(&(mg_system.log), NULL, bufferx, "worker_init: config file size", 0);
}
*/
   mg_init_critical_section((void *) &mg_global_mutex);

   mg_system.config = (char *) mg_malloc(NULL, size + size_default + 32, 6);
   if (!mg_system.config) {
      mg_system.plog = &(mg_system.log);
      mg_system.log.req_no = 0;
      mg_system.log.fun_no = 0;
      strcpy(mg_system.config_error, "Memory allocation error (char *:mg_system.config)");
      return 0;
   }
   memset((void *) mg_system.config, 0, size + size_default + 32);

   if (size) {
      fp = fopen(mg_system.config_file, "r");
      if (fp) {
         len = 0;
         lenx = 0;
         count = 0;
         while (fgets(buffer, 512, fp) != NULL) {
            len = (int) strlen(buffer);
            if (len) {
               if ((len + lenx) <= (int) size) {
                  strcat(mg_system.config, buffer);
               }
               lenx += len;
               mg_system.config[lenx] = '\0';
            }
            count ++;
            if (count > 100000) {
               sprintf(mg_system.config_error, "Possible infinite loop reading the configuration file (%s)", mg_system.config_file);
               mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
               break;
            }
         }
         fclose(fp);
      }
      else {
         sprintf(mg_system.config_error, "Cannot read the configuration file (%s)", mg_system.config_file);
         mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
         strcpy(mg_system.config, MG_DEFAULT_CONFIG);
         size = size_default;
      }
   }
   else {
      strcpy(mg_system.config, MG_DEFAULT_CONFIG);
      size = size_default;
   }

   if (mg_system.config_error[0]) {
      return -1;
   }

   mg_system.config_size = size;

   mg_system.plog = &(mg_system.log);
   mg_system.log.req_no = 0;
   mg_system.log.fun_no = 0;

   mg_parse_config();

   if (!mg_system.config_error[0]) {
      mg_verify_config();
   }
/*
   {
      int size;
      char *p;
      char buffer[256];

      size = 5000000;
      p = (char *) mg_malloc(NULL, size, 999);
      sprintf(buffer, "Reserve large memory block; size=%d, p=%p", size, p);
      mg_log_event(&(mg_system.log), NULL, buffer, "mg_web: memory test", 0);
   }
*/

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_worker_init: %x:%d", code, DBX_TRACE_VAR);
      strcpy(mg_system.config_error, bufferx);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif

}


int mg_worker_exit()
{
   DBX_TRACE_INIT(0)
   int rc;
   char title[128], buffer[128];
   MGWEB web;
   DBXCON *pcon, *pcon_next;
   MGTLS *ptls, *ptls_next;
   MGWSMAP *pwsmap, *pwsmap_next;
   MGPATH *ppath, *ppath_next;
   MGSRV *psrv, *psrv_next;

#ifdef _WIN32
__try {
#endif

   rc = CACHE_SUCCESS;
/*
   {
      char buffer[256];
      sprintf(buffer, "mg_connection=%p;", mg_connection);
      mg_log_event(&(mg_system.log), NULL, buffer, "mg_worker_exit", 0);
   }
*/

   /* v2.8.39 close down conections and free associated global memory */
   pcon = mg_connection;
   while (pcon) {
      pcon_next = pcon->pnext;
      memset((void *) &web, 0, sizeof(MGWEB));
      web.pcon = pcon;
      mg_release_connection(&web, 1);
      mg_free(NULL, (void *) pcon, 5);
      pcon = pcon_next;
   }
   mg_connection = NULL;

   strcpy(title, "mg_web: web server worker closing");
   sprintf(buffer, "exit status code: %d", rc);
   if (mg_ydb_so_global) {
      rc = mg_ydb_so_global->p_ydb_exit();
      mg_ydb_so_global = NULL;
      sprintf(buffer, "YottaDB process exit status code: %d", rc);
   }
   if (mg_gtm_so_global) {
      rc = mg_gtm_so_global->p_gtm_exit();
      mg_gtm_so_global = NULL;
      sprintf(buffer, "GTM process exit status code: %d", rc);
   }
   if (mg_isc_so_global) {
      rc = mg_isc_so_global->p_CacheEnd();
      mg_isc_so_global = NULL;
      sprintf(buffer, "ISC process exit status code: %d", rc);
   }

   mg_log_event(&(mg_system.log), NULL, buffer, title, 0);

   mg_delete_critical_section((void *) &mg_global_mutex);

   /* v2.8.39 free global memory used to hold configuration */
   ptls = mg_tls;
   while (ptls) {
      ptls_next = ptls->pnext;
      mg_free(NULL, (void *) ptls, 6);
      ptls = ptls_next;
   }
   mg_tls = NULL;

   ppath = mg_path;
   while (ppath) {
      ppath_next = ppath->pnext;
      pwsmap = ppath->pwsmap;
      while (pwsmap) {
         pwsmap_next = pwsmap->pnext;
         mg_free(NULL, (void *) pwsmap, 7);
         pwsmap = pwsmap_next;
      }
      ppath->pwsmap = NULL;
      mg_free(NULL, (void *) ppath, 8);
      ppath = ppath_next;
   }
   mg_path = NULL;

   psrv = mg_server;
   while (psrv) {
      psrv_next = psrv->pnext;
      mg_free(NULL, (void *) psrv, 9);
      psrv = psrv_next;
   }
   mg_server = NULL;

   if (mg_system.config) {
      mg_free(NULL, (void *) mg_system.config, 10);
      mg_system.config = NULL;
   }

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_worker_exit: %x:%d", code, DBX_TRACE_VAR);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}



int mg_parse_config()
{
   DBX_TRACE_INIT(0)
   int wn, vn, ln, n, len, lenx, line_len, size, inserver, inpath, intls, incgi, inenv, eos, ex;
   char *pa, *pz, *peol, *p, *pprop;
   char *word[256];
   char line[1024], buffer[256];
   MGSRV *psrv, *psrv_prev;
   MGPATH *ppath, *ppath_prev;
   MGTLS *ptls, *ptls_prev;
   MGWSMAP *pwsmap, *pwsmap_prev;

#ifdef _WIN32
__try {
#endif

   psrv = NULL;
   psrv_prev = NULL;
   ppath = NULL;
   ppath_prev = NULL;
   ptls = NULL;
   ptls_prev = NULL;
   pwsmap = NULL;
   pwsmap_prev = NULL;
   inserver = 0;
   inpath = 0;
   intls = 0;
   incgi = 0;
   inenv = 0;
   line_len = 0;
   mg_system.config_error[0] = '\0';
   size = 0;
   ln = 0;
   pa = mg_system.config;
   while (pa) {
      ln ++;
      if (ln > 1000) {
         sprintf(mg_system.config_error, "Possible infinite loop parsing the configuration file (%s)", mg_system.config_file);
         mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
         break;
      }
      if (*pa == '\0' || size > mg_system.config_size || mg_system.config_error[0]) {
         break;
      }

      wn = 0;
      peol = strstr(pa, "\n");
      if (peol) {
         *peol = '\0';
      }

      /* Get file line minus any leading spaces or tabs */
      pz = pa;
      while (*pz == ' ' || *pz == '\x09')
         pz ++;
      strncpy(line, pz, 1000);
      line[1000] = '\0';
      line_len = (int) strlen(line);

      pz = pa;
      word[0] = NULL;
      while (*pz) {
         if (*pz == ' ' || *pz == '\x09') {
            *pz = '\0';
         }
         else {
            if (pz == pa || *(pz - 1) == '\0') {
               if (wn < 32) {
                  word[wn ++] = pz;
                  word[wn] = NULL;
               }
            }
         }
         size ++;
         if (size > 100000) {
            sprintf(mg_system.config_error, "Possible infinite loop parsing a line in the configuration file (%s)", mg_system.config_file);
            lenx = (int) strlen(mg_system.config_error);
            if (lenx < 500) {
               strncpy(mg_system.config_error + lenx, line, 500 - lenx);
               mg_system.config_error[500] = '\0';
            }
            mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
            break;
         }
         pz ++;
      }
      if (peol)
         pa = (peol + 1);
      else
         pa = NULL;
/*
      {
         char bufferx[1024];
         sprintf(bufferx, "wn=%d; ", wn);
         for (n = 0; n < wn; n ++) {
            strcat(bufferx, word[n]);
            strcat(bufferx, ";");
         }
         mg_log_event(&(mg_system.log), NULL, bufferx, "mg_parse_config: config file parsed line", 0);
      }
*/
      if (wn && word[0] && word[0][0] != '#') {
         if (word[0][0] == '<') {
            eos = 0;

            mg_lcase(word[0]);
            if (strstr(word[0], "/"))
               eos = 1;
            if (strstr(word[0], "server")) {
               inserver = eos ? 0 : 1;
               if (!eos && word[1]) {
                  len = (int) strlen(word[1]);
                  if (word[1][len - 1] == '>') {
                     len --;
                     word[1][len] = '\0';
                  }
                  psrv = (MGSRV *) mg_malloc(NULL, sizeof(MGSRV), 7);
                  if (!psrv) {
                     strcpy(mg_system.config_error, "Memory allocation error (MGSRV)");
                     break;
                  }
                  memset((void *) psrv, 0, sizeof(MGSRV));
                  psrv->offline = 0;
                  /* v2.2.20 */
                  psrv->time_offline = 0;
                  /* v2.4.26 */
                  psrv->idle_timeout = 0;
                  psrv->health_check = 0;
                  /* v2.4.25 */
                  psrv->con_retry_no = 0;
                  psrv->con_retry_time = 0;
                  psrv->max_connections = 0; /* v2.5.30 */
                  psrv->ptls = NULL; /* v2.3.21 */
                  if (psrv_prev) {
                     psrv_prev->pnext = psrv;
                  }
                  else {
                     mg_server = psrv;
                  }
                  psrv_prev = psrv;
                  psrv->name = word[1];
                  psrv->name_len = (int) strlen(psrv->name);
                  /* v2.1.17 */
                  strncpy(psrv->lcname, psrv->name, 60);
                  psrv->lcname[60] = '\0';
                  mg_lcase(psrv->lcname);
               }
            }
            else if (strstr(word[0], "location")) {
               inpath = eos ? 0 : 1;
               if (!eos && word[1]) {
                  len = (int) strlen(word[1]);
                  if (word[1][len - 1] == '>') {
                     len --;
                     word[1][len] = '\0';
                  }
                  if (word[1][len - 1] != '/') {
                     word[1][len ++] = '/';
                     word[1][len] = '\0';
                  }
                  ppath = (MGPATH *) mg_malloc(NULL, sizeof(MGPATH), 8);
                  if (!ppath) {
                     strcpy(mg_system.config_error, "Memory allocation error (MGPATH)");
                     break;
                  }
                  memset((void *) ppath, 0, sizeof(MGPATH));
                  for (n = 0; n < 32; n ++) { /* v2.1.16 */
                     ppath->servers[n].name = NULL;
                     ppath->servers[n].psrv = NULL;
                     ppath->servers[n].exclusive = 0;
                  }
                  ppath->pwsmap = NULL;
                  pwsmap_prev = NULL;
                  ppath->admin = 0;
                  ppath->load_balancing = 0;
                  ppath->sa_cookie = NULL;
                  ppath->sa_order = 0;
                  ppath->sa_variables[0] = NULL;
                  ppath->server_no = 0;
                  ppath->srv_max = 0;
                  if (ppath_prev) {
                     ppath_prev->pnext = ppath;
                  }
                  else {
                     mg_path = ppath;
                  }
                  ppath_prev = ppath;
                  ppath->name = word[1];
                  ppath->name_len = (int) strlen(ppath->name);
                  mg_lcase(ppath->name);
               }
            }
            else if (strstr(word[0], "tls")) { /* v2.3.21 */
               intls = eos ? 0 : 1;
               if (!eos && word[1]) {
                  len = (int) strlen(word[1]);
                  if (word[1][len - 1] == '>') {
                     len --;
                     word[1][len] = '\0';
                  }
                  ptls = (MGTLS *) mg_malloc(NULL, sizeof(MGTLS), 9);
                  if (!ptls) {
                     strcpy(mg_system.config_error, "Memory allocation error (MGTLS)");
                     break;
                  }
                  memset((void *) ptls, 0, sizeof(MGTLS));
                  for (n = 0; n < 8; n ++) {
                     ptls->protocols[n] = NULL;
                  }
                  ptls->verify_peer = 0;
                  ptls->key_type = 0;
                  ptls->name = NULL;
                  ptls->cert_file = NULL;
                  ptls->key_file = NULL;
                  ptls->password = NULL;
                  ptls->ca_file = NULL;
                  ptls->ca_path = NULL;
                  ptls->cipher_list = NULL;
                  if (ptls_prev) {
                     ptls_prev->pnext = ptls;
                  }
                  else {
                     mg_tls = ptls;
                  }
                  ptls_prev = ptls;
                  ptls->name = word[1];
                  mg_lcase(ptls->name);
               }
            }
            else if (strstr(word[0], "cgi")) {
               incgi = eos ? 0 : 1;
            }
            else if (strstr(word[0], "env")) {
               inenv = eos ? 0 : 1;
            }
            else {
               sprintf(mg_system.config_error, "Configuration file syntax error on line %d", ln);
            }
         }
         else {
            if (inserver) {
               if (inenv) {
                  if (!psrv->penv) {
                     psrv->penv = (MGBUF *) mg_malloc(NULL, sizeof(MGBUF), 10);
                     if (!psrv->penv) {
                        strcpy(mg_system.config_error, "Memory allocation error (MGBUF:ENV)");
                        break;
                     }
                     mg_buf_init(psrv->penv, 1024, 1024);
                  }
                  if (psrv->penv && line_len) {
                     mg_buf_cat(psrv->penv, line, (unsigned long) line_len);
                     mg_buf_cat(psrv->penv, "\n", (unsigned long) 1);
                  }
               }
               else if (wn > 1) {
                  mg_lcase(word[0]);
                  if (!strcmp(word[0], "type")) {
                     psrv->dbtype_name = word[1];
                  }
                  else if (!strcmp(word[0], "path")) {
                     psrv->shdir = word[1];
                  }
                  else if (!strcmp(word[0], "host")) {
                     psrv->ip_address = word[1];
                  }
                  else if (!strcmp(word[0], "tcp_port")) {
                     psrv->port = (int) strtol(word[1], NULL, 10);
                  }
                  else if (!strcmp(word[0], "username")) {
                     psrv->username = word[1];
                  }
                  else if (!strcmp(word[0], "password")) {
                     psrv->password = word[1];
                  }
                  else if (!strcmp(word[0], "namespace")) {
                     psrv->uci = word[1];
                  }
                  else if (!strcmp(word[0], "input_device")) {
                     psrv->input_device = word[1];
                  }
                  else if (!strcmp(word[0], "output_device")) {
                     psrv->output_device = word[1];
                  }
                  else if (!strcmp(word[0], "idle_timeout")) { /* v2.4.26 */
                     psrv->idle_timeout = (int) strtol(word[1], NULL, 10);
                  }
                  else if (!strcmp(word[0], "health_check")) { /* v2.2.20 */
                     psrv->health_check = (int) strtol(word[1], NULL, 10);
                  }
                  else if (!strcmp(word[0], "connection_retries")) { /* v2.4.25 */
                     psrv->con_retry_no = (int) strtol(word[1], NULL, 10);
                     p = strstr(word[1], "/");
                     if (p) {
                        psrv->con_retry_time = (int) strtol(++ p, NULL, 10);
                     }
                  }
                  else if (!strcmp(word[0], "max_connections")) { /* v2.5.30 */
                     psrv->max_connections = (int) strtol(word[1], NULL, 10);
                  }
                  else if (!strcmp(word[0], "tls")) { /* v2.3.21 */
                     psrv->tls_name = word[1];
                  }
                  else {
                     sprintf(mg_system.config_error, "Invalid 'server' parameter '%s' on line %d", word[0], ln); 
                  }
               }
            }
            else if (inpath) {
               if (incgi) {
                  for (n = 0; n < wn; n ++) {
                     if (ppath->cgi_max < 120) {
                        ppath->cgi[ppath->cgi_max ++] = word[n];
                     }
                  }
               }
               if (wn > 1) {
                  mg_lcase(word[0]);
                  if (!strcmp(word[0], "function")) {
                     ppath->function = word[1];
                  }
                  else if (!strcmp(word[0], "websocket")) { /* v2.6.32 */
                     if (wn > 2) {
                        pwsmap = (MGWSMAP *) mg_malloc(NULL, sizeof(MGWSMAP), 11);
                        if (pwsmap) {
                           pwsmap->name = word[1];
                           pwsmap->name_len = (int) strlen(pwsmap->name);
                           pwsmap->function = word[2];
                           pwsmap->function_len = (int) strlen(pwsmap->function);
                           pwsmap->pnext = NULL;
                           if (!ppath->pwsmap) {
                              ppath->pwsmap = pwsmap;
                           }
                           if (pwsmap_prev) {
                              pwsmap_prev->pnext = pwsmap;
                           }
                           pwsmap_prev = pwsmap;
                        }
                     }
                  }
                  else if (!strcmp(word[0], "servers")) {
                     for (n = 1; n < wn; n ++) {
                        if (ppath->srv_max < 30) {
                           /* v2.1.16 */
                           ex = 0;
                           pprop = strstr(word[n], ":");
                           if (pprop) {
                              *pprop = '\0';
                              pprop ++;
                              strncpy(buffer, pprop, 32);
                              buffer[32] = '\0';
                              mg_lcase(buffer);
                              if (strstr(buffer, "ex")) {
                                  ex = 1;
                              }
                           }
                           if (word[n][0]) {
                              ppath->servers[ppath->srv_max].name = word[n];
                              /* v2.1.17 */
                              strncpy(ppath->servers[ppath->srv_max].lcname, ppath->servers[ppath->srv_max].name, 60);
                              ppath->servers[ppath->srv_max].lcname[60] = '\0';
                              mg_lcase(ppath->servers[ppath->srv_max].lcname);
                              ppath->servers[ppath->srv_max].exclusive = ex;
                              ppath->srv_max ++;
                           }
                        }
                     }
                  }
                  else if (!strcmp(word[0], "load_balancing")) {
                     ppath->load_balancing = 0;
                     mg_lcase(word[1]);
                     if (!strcmp(word[1], "on")) {
                        ppath->load_balancing = 1;
                     }
                  }
                  else if (!strcmp(word[0], "server_affinity")) {
                     ppath->sa_order = 0;
                     for (n = 1; n < wn; n ++) {
                        p = strstr(word[n], ":");
                        if (p) {
                           *p = '\0';
                           mg_lcase(word[n]);
                           if (!strcmp("variable", word[n])) {
                              if (!ppath->sa_order) {
                                 ppath->sa_order = 1; /* use sa variables first */
                              }
                              p ++;
                              vn = 0;
                              while (p && *p) {
                                 if (vn < 4)
                                    ppath->sa_variables[vn ++] = p;
                                 p = strstr(p, ",");
                                 if (p) {
                                    *p = '\0';
                                    p ++;
                                 }
                              }
                              ppath->sa_variables[vn] = NULL;
                           }
                           else if (!strcmp("cookie", word[n])) {
                              if (!ppath->sa_order) {
                                 ppath->sa_order = 2; /* use sa cookie first */
                              }
                              ppath->sa_cookie = p + 1;
                              if (!strlen(ppath->sa_cookie)) {
                                 ppath->sa_cookie = NULL;
                              }
                           }
                           else {
                              sprintf(mg_system.config_error, "Invalid 'location' parameter '%s' on line %d", word[0], ln); 
                           }
                        }
                     }
                  }
                  else if (!strcmp(word[0], "administrator")) { /* v2.4.24 */
                     ppath->admin = 1;
                     mg_lcase(word[1]);
                     if (!strcmp(word[1], "on")) {
                        ppath->admin = 2;
                     }
                     else if (!strcmp(word[1], "off")) { /* v2.7.33 */
                        ppath->admin = 0;
                     }
                  }
                  else {
                     sprintf(mg_system.config_error, "Invalid 'location' parameter '%s' on line %d", word[0], ln); 
                  }
               }
            }
            else if (intls) { /* v2.3.21 */
               if (wn > 1) {
                  mg_lcase(word[0]);
                  if (!strcmp(word[0], "path")) {
                     ptls->libpath = word[1];
                  }
                  else if (!strcmp(word[0], "certificate_file")) {
                     ptls->cert_file = word[1];
                  }
                  else if (!strcmp(word[0], "key_file")) {
                     ptls->key_file = word[1];
                  }
                  else if (!strcmp(word[0], "password")) {
                     ptls->password = word[1];
                  }
                  else if (!strcmp(word[0], "ca_certificate_file")) {
                     ptls->ca_file = word[1];
                  }
                  else if (!strcmp(word[0], "ca_certificate_path")) {
                     ptls->ca_path = word[1];
                  }
                  else if (!strcmp(word[0], "cipher_list")) {
                     ptls->cipher_list = word[1];
                  }
                  else if (!strcmp(word[0], "protocols")) {
                     for (n = 1; n < 8; n ++) {
                        if (!word[n]) {
                           break;
                        }
                        mg_lcase(word[n]);
                        if (!strncmp(word[n], "sslv", 4) || !strncmp(word[n], "tlsv", 4)) {
                           double vers;
                           vers = strtod(word[n] + 4, NULL);
                           if (vers == 0) {
                              sprintf(mg_system.config_error, "Invalid value (%s) for'tls' parameter 'protocols' on line %d", word[n], ln);
                              break;
                           }
                        }
                        else {
                           sprintf(mg_system.config_error, "Invalid value (%s) for'tls' parameter 'protocols' on line %d", word[n], ln);
                           break;
                        }
                        ptls->protocols[n] = word[n];
                     }
                     if (mg_system.config_error[0]) {
                        break;
                     }
                  }
                  else if (!strcmp(word[0], "key_type")) {
                     mg_lcase(word[1]);
                     if (!strcmp(word[1], "rsa"))
                        ptls->key_type = 0;
                     else if (!strcmp(word[1], "dsa"))
                        ptls->key_type = 1;
                     else
                        sprintf(mg_system.config_error, "Invalid value (%s) for'tls' parameter 'key_type' on line %d", word[1], ln); 
                  }
                  else if (!strcmp(word[0], "verify_peer")) {
                     mg_lcase(word[1]);
                     if (!strcmp(word[1], "on") || !strcmp(word[1], "yes") || !strcmp(word[1], "1"))
                        ptls->verify_peer = 1;
                     else if (!strcmp(word[1], "off") || !strcmp(word[1], "no") || !strcmp(word[1], "0"))
                        ptls->verify_peer = 0;
                     else
                        sprintf(mg_system.config_error, "Invalid value (%s) for'tls' parameter 'verify_peer' on line %d", word[1], ln); 
                  }
                  else {
                     sprintf(mg_system.config_error, "Invalid 'tls' parameter '%s' on line %d", word[0], ln); 
                  }
               }
               else {
                  sprintf(mg_system.config_error, "Invalid 'tls' directive '%s' on line %d", word[0], ln); 
               }
            }
            else if (incgi) {
               for (n = 0; n < wn; n ++) {
                  if (mg_system.cgi_max < 120) {
                     mg_system.cgi[mg_system.cgi_max ++] = word[n];
                  }
               }
            }
            else { /* global scope */
               if (wn > 1) {
                  mg_lcase(word[0]);
                  if (!strcmp(word[0], "timeout")) {
                     mg_system.timeout = (int) strtol(word[1], NULL, 10);
                     if (!mg_system.timeout) {
                        mg_system.timeout = NETX_TIMEOUT;
                     }
                  }
                  else if (!strcmp(word[0], "log_level")) {
                     for (n = 1; n < wn; n ++) {
                        mg_set_log_level(&(mg_system.log), word[n], n);
                     }
                  }
                  else if (!strcmp(word[0], "chunking")) { /* 2.0.8 */
                     if (wn > 1 && word[1]) {
                        mg_lcase(word[1]);
                        if (strstr(word[1], "off")) {
                           mg_system.chunking = 0;
                        }
                        else {
                           mg_system.chunking = (unsigned long) strtol(word[1], NULL, 10);
                           if (mg_system.chunking) {
                              if (strstr(word[1], "k"))
                                 mg_system.chunking *= 1000;
                              else if (strstr(word[1], "m"))
                                 mg_system.chunking *= (1000 * 1000);
                              else if (strstr(word[1], "g"))
                                 mg_system.chunking *= (1000 * 1000 * 1000);
                           }
                           else { /* invalid value */
                              mg_system.chunking = 1;
                           }
                        }
                     }
                  }
                  else if (!strcmp(word[0], "request_buffer_size")) { /* 2.1.10 */
                     if (wn > 1 && word[1]) {
                        mg_lcase(word[1]);
                        mg_system.request_buffer_size = (unsigned long) strtol(word[1], NULL, 10);
                        if (strstr(word[1], "k"))
                           mg_system.request_buffer_size *= 1000;
                        else if (strstr(word[1], "m"))
                           mg_system.request_buffer_size *= 1000000;
                     }
                  }
                  else if (!strcmp(word[0], "custompage_dbserver_unavailable") && wn > 1) { /* 2.0.8 */
                     mg_system.custompage_dbserver_unavailable = word[1];
                  }
                  else if (!strcmp(word[0], "custompage_dbserver_busy") && wn > 1) { /* 2.0.8 */
                     mg_system.custompage_dbserver_busy = word[1];
                  }
                  else if (!strcmp(word[0], "custompage_dbserver_disabled") && wn > 1) { /* 2.0.8 */
                     mg_system.custompage_dbserver_disabled = word[1];
                  }
                  else if (!strcmp(word[0], "custompage_dbserver_timeout") && wn > 1) { /* v2.7.33 */
                     mg_system.custompage_dbserver_timeout = word[1];
                  }
                  else {
                     sprintf(mg_system.config_error, "Invalid 'global' parameter '%s' on line %d", word[0], ln); 
                  }
               }
            }
         }
      }
   }
   if (mg_system.config_error[0]) {
      mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
   }

   if (!mg_system.timeout) {
      mg_system.timeout = NETX_TIMEOUT;
   }

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_parse_config: %x:%d", code, DBX_TRACE_VAR);
      strcpy(mg_system.config_error, bufferx);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int mg_set_log_level(DBXLOG *plog, char *word, int wn)
{
   if (wn < 2) {
      plog->log_errors = 0;
      plog->log_verbose = 0;
      plog->log_frames = 0;
      plog->log_transmissions = 0;
      plog->log_transmissions_to_webserver = 0;
      plog->log_tls = 0;
   }
   if (strstr(word, "e") || strstr(word, "E"))
      plog->log_errors = 1;
   if (strstr(word, "v") || strstr(word, "V"))
      plog->log_verbose = 1;
   if (strstr(word, "f") || strstr(word, "F"))
      plog->log_frames = 1;
   if (strstr(word, "t") || strstr(word, "T"))
      plog->log_transmissions = 1;
   if (strstr(word, "w") || strstr(word, "W"))
      plog->log_transmissions_to_webserver = 1;
   if (strstr(word, "s")) /* v2.3.21 */
      plog->log_tls = 1;
   if (strstr(word, "S"))
      plog->log_tls = 2;

   return CACHE_SUCCESS;
}


int mg_verify_config()
{
   DBX_TRACE_INIT(0)
   int n;
   char *pbuf;
   char buffer[256];
   MGSRV *psrv;
   MGPATH *ppath;
   MGTLS *ptls;
   MGWSMAP *pwsmap;

#ifdef _WIN32
__try {
#endif

   pbuf = (char *) mg_malloc(NULL, 8192, 12);
   
   psrv = mg_server;
   if (!psrv) {
      strcpy(mg_system.config_error, "No DB Server configurations found");
      goto mg_verify_config_exit;
   }

   ppath = mg_path;
   if (!ppath) {
      strcpy(mg_system.config_error, "No Location configurations found");
      goto mg_verify_config_exit;
   }

   if (pbuf) {
      sprintf(pbuf, "response timeout=%d; CGI Variables requested=%d; chunking=%lu;", mg_system.timeout, mg_system.cgi_max, mg_system.chunking);
      mg_log_event(&(mg_system.log), NULL, pbuf, "mg_web: configuration: global section", 0);
   }

   while (psrv) {
      psrv->nagle_algorithm = 0;
      if (!psrv->dbtype_name) {
         sprintf(mg_system.config_error, "Missing database type from DB Server '%s'", psrv->name);
         break;
      }
      else {
         /* v2.2.18 */
         mg_lcase(psrv->dbtype_name);
         if (!strcmp(psrv->dbtype_name, "iris")) {
            psrv->dbtype = DBX_DBTYPE_IRIS;
            psrv->max_string_size = DBX_LS_BUFFER_ISC;
         }
         else if (!strcmp(psrv->dbtype_name, "cache")) {
            psrv->dbtype = DBX_DBTYPE_CACHE;
            psrv->max_string_size = DBX_LS_BUFFER_ISC;
         }
         else if (!strcmp(psrv->dbtype_name, "yottadb")) {
            psrv->dbtype = DBX_DBTYPE_YOTTADB;
            psrv->max_string_size = DBX_LS_BUFFER_YDB;
         }
         /* v2.5.31 */
         else if (!strcmp(psrv->dbtype_name, "gt.m") || !strcmp(psrv->dbtype_name, "gtm")) {
            psrv->dbtype = DBX_DBTYPE_GTM;
            psrv->max_string_size = DBX_LS_BUFFER_YDB;
         }
         else if (!strcmp(psrv->dbtype_name, "node.js") || !strcmp(psrv->dbtype_name, "nodejs") || !strcmp(psrv->dbtype_name, "node")) {
            psrv->dbtype = DBX_DBTYPE_NODEJS;
            psrv->max_string_size = DBX_LS_BUFFER_ISC;
         }
         else if (!strcmp(psrv->dbtype_name, "bun")) {
            psrv->dbtype = DBX_DBTYPE_BUN;
            psrv->max_string_size = DBX_LS_BUFFER_YDB;
         }
         else {
            sprintf(mg_system.config_error, "Unrecognized database name (%s) specified for DB Server '%s'", psrv->dbtype_name, psrv->name);
         }
      }
      if (!psrv->timeout) {
         psrv->timeout = mg_system.timeout;
      }
      if (psrv->shdir) {
         psrv->net_connection = 0;
         if (psrv->dbtype != DBX_DBTYPE_YOTTADB) {
            if (!psrv->username) {
               sprintf(mg_system.config_error, "Missing username from DB Server '%s'", psrv->name);
               break;
            }
            if (!psrv->password) {
               sprintf(mg_system.config_error, "Missing password from DB Server '%s'", psrv->name);
               break;
            }
         }
      }
      else {
         psrv->net_connection = 1;
         if (!psrv->ip_address) {
            sprintf(mg_system.config_error, "Missing host from DB Server '%s'", psrv->name);
            break;
         }
         if (!psrv->port) {
            sprintf(mg_system.config_error, "Missing tcp_port from DB Server '%s'", psrv->name);
            break;
         }
      }
      if (psrv->tls_name) { /* v2.3.21 */
         ptls = mg_tls;
         while (ptls) {
            if (!strcmp(ptls->name, psrv->tls_name)) {
               break;
            }
            ptls = ptls->pnext;
         }
         if (!ptls) {
            sprintf(mg_system.config_error, "Cannot find the tls configuration named '%s' for DB Server'%s'", psrv->tls_name, psrv->name);
            break;
         }
#if DBX_WITH_TLS >= 1
         psrv->ptls = ptls;
#else
         psrv->ptls = NULL;
         sprintf(mg_system.config_error, "This mg_web installation does not contain TLS functionality (DB Server'%s')", psrv->name);
#endif
      }

      if (psrv->penv) {
         mg_buf_cat(psrv->penv, "\n", 1);
      }

      if (pbuf) { /* v2.4.25 */
         sprintf(pbuf, "server name=%s; type=%s; path=%s; host=%s; port=%d; username=%s; password=%s; idle_timeout=%d; health_check=%d; connection_retries=%d/%d; max_connections=%d; tls=%s;", psrv->name, psrv->dbtype_name ? psrv->dbtype_name : "null", psrv->shdir ? psrv->shdir : "null", psrv->ip_address ? psrv->ip_address : "null", psrv->port, psrv->username ? psrv->username : "null", psrv->password ? psrv->password : "null", psrv->idle_timeout, psrv->health_check, psrv->con_retry_no, psrv->con_retry_time, psrv->max_connections, psrv->tls_name ? psrv->tls_name : "null");
         mg_log_event(&(mg_system.log), NULL, pbuf, "mg_web: configuration: DB Server", 0);
         if (psrv->penv) {
            sprintf(pbuf, "mg_web: configuration: DB Server: environment variables for DB Server name=%s;", psrv->name);
            mg_log_buffer(&(mg_system.log), NULL, (char *) psrv->penv->p_buffer, (int) psrv->penv->data_size, pbuf, 0);
         }
      }

      psrv = psrv->pnext;
   }

   if (mg_system.config_error[0]) {
      mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration: error", 0);
      goto mg_verify_config_exit;
   }

   while (ppath) {
      if (ppath->admin) {
         if (pbuf) {
            sprintf(pbuf, "location name=%s; path reserved for administrator functions", ppath->name);
            mg_log_event(&(mg_system.log), NULL, pbuf, "mg_web: configuration: location", 0);
         }
      }
      else {
         if (!ppath->function) {
            sprintf(mg_system.config_error, "Missing database function from Location '%s'", ppath->name);
            break;
         }
         if (!ppath->servers[0].name) {
            sprintf(mg_system.config_error, "No DB Servers defined for Location '%s'", ppath->name);
            break;
         }
         for (n = 0; ppath->servers[n].name; n ++) {
            psrv = mg_server;
            while (psrv) {
               if (!strcmp(ppath->servers[n].name, psrv->name)) {
                  ppath->servers[n].psrv = psrv;
                  break;
               }
               psrv = psrv->pnext;
            }
            if (!psrv) {
               sprintf(mg_system.config_error, "DB Server configuration for '%s' not found for Location '%s'", ppath->servers[n].name, ppath->name);
               break;
            }
         }
         if (mg_system.config_error[0]) {
            break;
         }
         if (!ppath->servers[0].psrv || !ppath->servers[0].name) {
            sprintf(mg_system.config_error, "No DB Servers found for Location '%s'", ppath->name);
            break;
         }

         if (pbuf) {
            sprintf(pbuf, "location name=%s; function=%s; load balancing=%d; SA precedence=%d; SA cookie=%s", ppath->name, ppath->function ? ppath->function : "null", ppath->load_balancing, ppath->sa_order, ppath->sa_cookie ? ppath->sa_cookie : "null");
            if (ppath->sa_variables[0]) {
               for (n = 0; ppath->sa_variables[n]; n ++) {
                  if (!n) {
                     strcat(pbuf, "; SA variable=");
                     strcat(pbuf, ppath->sa_variables[n]);
                  }
                  else {
                     strcat(pbuf, " and ");
                     strcat(pbuf, ppath->sa_variables[n]);
                  }
               }
            }
            else {
               strcat(pbuf, "; SA variable=null");
            }
            for (n = 0; ppath->servers[n].name; n ++) {
               sprintf(buffer, "; server %d=", n);
               strcat(pbuf, buffer);
               strcat(pbuf, ppath->servers[n].name);
               if (ppath->servers[n].exclusive) {
                  strcat(pbuf, "(ex)");
               }
            }
            mg_log_event(&(mg_system.log), NULL, pbuf, "mg_web: configuration: location", 0);
         }
      }
      /* v2.6.32 */
      pwsmap = ppath->pwsmap;
      while (pwsmap) {
         if (pbuf) {
            sprintf(pbuf, "location name=%s; mapped file name=%s; function=%s;", ppath->name, pwsmap->name ? pwsmap->name : "null", pwsmap->function ? pwsmap->function : "null");
            mg_log_event(&(mg_system.log), NULL, pbuf, "mg_web: configuration: location websocket", 0);
         }
         pwsmap = pwsmap->pnext;
      }
      ppath = ppath->pnext;
   }

mg_verify_config_exit:

   if (mg_system.config_error[0]) {
      mg_log_event(&(mg_system.log), NULL, mg_system.config_error, "mg_web: configuration error", 0);
   }

   if (pbuf) {
      mg_free(NULL, pbuf, 11);
   }

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_verify_config: %x:%d", code, DBX_TRACE_VAR);
      strcpy(mg_system.config_error, bufferx);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return 0;
}
#endif
}


int isc_load_library(MGWEB *pweb)
{
   int n, len, result;
   char primlib[DBX_ERROR_SIZE], primerr[DBX_ERROR_SIZE];
   char verfile[256], fun[64];
   char *libnam[16];
   DBXCON *pcon;

   pcon = pweb->pcon;

   strcpy(pcon->p_isc_so->libdir, pcon->psrv->shdir);
   strcpy(pcon->p_isc_so->funprfx, "Cache");
   strcpy(pcon->p_isc_so->dbname, "Cache");

   strcpy(verfile, pcon->psrv->shdir);
   len = (int) strlen(pcon->p_isc_so->libdir);
   if (pcon->p_isc_so->libdir[len - 1] == '/' || pcon->p_isc_so->libdir[len - 1] == '\\') {
      pcon->p_isc_so->libdir[len - 1] = '\0';
      len --;
   }
   for (n = len - 1; n > 0; n --) {
      if (pcon->p_isc_so->libdir[n] == '/') {
         strcpy((pcon->p_isc_so->libdir + (n + 1)), "bin/");
         break;
      }
      else if (pcon->p_isc_so->libdir[n] == '\\') {
         strcpy((pcon->p_isc_so->libdir + (n + 1)), "bin\\");
         break;
      }
   }

   /* printf("version=%s;\n", pcon->p_zv->version); */

   n = 0;
   if (pcon->psrv->dbtype == DBX_DBTYPE_IRIS) {
#if defined(_WIN32)
      libnam[n ++] = (char *) DBX_IRIS_DLL;
      libnam[n ++] = (char *) DBX_CACHE_DLL;
#else
#if defined(MACOSX)
      libnam[n ++] = (char *) DBX_ISCIRIS_DYLIB;
      libnam[n ++] = (char *) DBX_IRIS_DYLIB;
      libnam[n ++] = (char *) DBX_ISCCACHE_DYLIB;
      libnam[n ++] = (char *) DBX_CACHE_DYLIB;
      libnam[n ++] = (char *) DBX_ISCIRIS_SO;
      libnam[n ++] = (char *) DBX_IRIS_SO;
      libnam[n ++] = (char *) DBX_ISCCACHE_SO;
      libnam[n ++] = (char *) DBX_CACHE_SO;
#else
      libnam[n ++] = (char *) DBX_ISCIRIS_SO;
      libnam[n ++] = (char *) DBX_IRIS_SO;
      libnam[n ++] = (char *) DBX_ISCCACHE_SO;
      libnam[n ++] = (char *) DBX_CACHE_SO;
#endif
#endif
   }
   else {
#if defined(_WIN32)
      libnam[n ++] = (char *) DBX_CACHE_DLL;
      libnam[n ++] = (char *) DBX_IRIS_DLL;
#else
#if defined(MACOSX)
      libnam[n ++] = (char *) DBX_ISCCACHE_DYLIB;
      libnam[n ++] = (char *) DBX_CACHE_DYLIB;
      libnam[n ++] = (char *) DBX_ISCIRIS_DYLIB;
      libnam[n ++] = (char *) DBX_IRIS_DYLIB;
      libnam[n ++] = (char *) DBX_ISCCACHE_SO;
      libnam[n ++] = (char *) DBX_CACHE_SO;
      libnam[n ++] = (char *) DBX_ISCIRIS_SO;
      libnam[n ++] = (char *) DBX_IRIS_SO;
#else
      libnam[n ++] = (char *) DBX_ISCCACHE_SO;
      libnam[n ++] = (char *) DBX_CACHE_SO;
      libnam[n ++] = (char *) DBX_ISCIRIS_SO;
      libnam[n ++] = (char *) DBX_IRIS_SO;
#endif
#endif
   }

   libnam[n ++] = NULL;
   strcpy(pcon->p_isc_so->libnam, pcon->p_isc_so->libdir);
   len = (int) strlen(pcon->p_isc_so->libnam);

   for (n = 0; libnam[n]; n ++) {
      strcpy(pcon->p_isc_so->libnam + len, libnam[n]);
      if (!n) {
         strcpy(primlib, pcon->p_isc_so->libnam);
      }

      pcon->p_isc_so->p_library = mg_dso_load(pcon->p_isc_so->libnam);

      if (pcon->p_isc_so->p_library) {
         if (strstr(libnam[n], "iris")) {
            pcon->p_isc_so->iris = 1;
            strcpy(pcon->p_isc_so->funprfx, "Iris");
            strcpy(pcon->p_isc_so->dbname, "IRIS");
         }
         strcpy(pweb->error, "");
         pweb->error_code = 0;
         break;
      }

      if (!n) {
         int len1, len2;
         char *p;
#if defined(_WIN32)
         DWORD errorcode;
         LPVOID lpMsgBuf;

         lpMsgBuf = NULL;
         errorcode = GetLastError();
         sprintf(pweb->error, "Error loading %s Library: %s; Error Code : %ld", pcon->p_isc_so->dbname, primlib, errorcode);
         len2 = (int) strlen(pweb->error);
         len1 = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        errorcode,
                        /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), */
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL 
                        );
         if (lpMsgBuf && len1 > 0 && (DBX_ERROR_SIZE - len2) > 30) {
            strncpy(primerr, (const char *) lpMsgBuf, DBX_ERROR_SIZE - 1);
            p = strstr(primerr, "\r\n");
            if (p)
               *p = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            p = strstr(primerr, "%1");
            if (p) {
               *p = 'I';
               *(p + 1) = 't';
            }
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
         if (lpMsgBuf)
            LocalFree(lpMsgBuf);
#else
         p = (char *) dlerror();
         sprintf(primerr, "Cannot load %s library: Error Code: %d", pcon->p_isc_so->dbname, errno);
         len2 = strlen(pweb->error);
         if (p) {
            strncpy(primerr, p, DBX_ERROR_SIZE - 1);
            primerr[DBX_ERROR_SIZE - 1] = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
#endif
      }
   }

   if (!pcon->p_isc_so->p_library) {
      goto isc_load_library_exit;
   }

   sprintf(fun, "%sSetDir", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheSetDir = (int (*) (char *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheSetDir) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }

   sprintf(fun, "%sSecureStartA", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheSecureStartA = (int (*) (CACHE_ASTRP, CACHE_ASTRP, CACHE_ASTRP, unsigned long, int, CACHE_ASTRP, CACHE_ASTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheSecureStartA) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }

   sprintf(fun, "%sEnd", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheEnd = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheEnd) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }

   sprintf(fun, "%sExStrNew", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExStrNew = (unsigned char * (*) (CACHE_EXSTRP, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheExStrNew) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sExStrNewW", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExStrNewW = (unsigned short * (*) (CACHE_EXSTRP, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheExStrNewW) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sExStrNewH", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExStrNewH = (wchar_t * (*) (CACHE_EXSTRP, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
/*
   if (!pcon->p_isc_so->p_CacheExStrNewH) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
*/
   sprintf(fun, "%sPushExStr", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushExStr = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushExStr) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushExStrW", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushExStrW = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushExStrW) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushExStrH", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushExStrH = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sPopExStr", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopExStr = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopExStr) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopExStrW", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopExStrW = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopExStrW) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopExStrH", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopExStrH = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sExStrKill", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExStrKill = (int (*) (CACHE_EXSTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheExStrKill) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushStr", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushStr = (int (*) (int, Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushStr) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushStrW", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushStrW = (int (*) (int, short *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushStrW) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushStrH", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushStrH = (int (*) (int, wchar_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sPopStr", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopStr = (int (*) (int *, Callin_char_t **)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopStr) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopStrW", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopStrW = (int (*) (int *, short **)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopStrW) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopStrH", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopStrH = (int (*) (int *, wchar_t **)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sPushDbl", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushDbl = (int (*) (double)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushDbl) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushIEEEDbl", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushIEEEDbl = (int (*) (double)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushIEEEDbl) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopDbl", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopDbl = (int (*) (double *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopDbl) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushInt", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushInt = (int (*) (int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushInt) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopInt", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopInt = (int (*) (int *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopInt) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }

   pcon->p_isc_so->p_CachePushInt64 = (int (*) (long long)) NULL;
   pcon->p_isc_so->p_CachePopInt64 = (int (*) (long long *)) NULL;

/*
   sprintf(fun, "%sPushInt64", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushInt64 = (int (*) (CACHE_INT64)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushInt64) {
      pcon->p_isc_so->p_CachePushInt64 = (int (*) (CACHE_INT64)) pcon->p_isc_so->p_CachePushInt;
   }
   sprintf(fun, "%sPushInt64", pcon->p_isc_so->funprfx);
   if (!pcon->p_isc_so->p_CachePushInt64) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPopInt64", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopInt64 = (int (*) (CACHE_INT64 *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePopInt64) {
      pcon->p_isc_so->p_CachePopInt64 = (int (*) (CACHE_INT64 *)) pcon->p_isc_so->p_CachePopInt;
   }
   if (!pcon->p_isc_so->p_CachePopInt64) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
*/

   sprintf(fun, "%sPushGlobal", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushGlobal = (int (*) (int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushGlobal) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushGlobalX", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushGlobalX = (int (*) (int, const Callin_char_t *, int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushGlobalX) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalGet", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalGet = (int (*) (int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalGet) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalSet", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalSet = (int (*) (int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalSet) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalData", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalData = (int (*) (int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalData) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalKill", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalKill = (int (*) (int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalKill) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalOrder", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalOrder = (int (*) (int, int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalOrder) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalQuery", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalQuery = (int (*) (int, int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalQuery) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalIncrement", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalIncrement = (int (*) (int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalIncrement) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sGlobalRelease", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGlobalRelease = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheGlobalRelease) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sAcquireLock", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheAcquireLock = (int (*) (int, int, int, int *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheAcquireLock) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sReleaseAllLocks", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheReleaseAllLocks = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheReleaseAllLocks) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sReleaseLock", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheReleaseLock = (int (*) (int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CacheReleaseLock) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }
   sprintf(fun, "%sPushLock", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushLock = (int (*) (int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   if (!pcon->p_isc_so->p_CachePushLock) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_isc_so->dbname, pcon->p_isc_so->libnam, fun);
      goto isc_load_library_exit;
   }

   sprintf(fun, "%sAddGlobal", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheAddGlobal = (int (*) (int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sAddGlobalDescriptor", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheAddGlobalDescriptor = (int (*) (int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sAddSSVN", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheAddSSVN = (int (*) (int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sAddSSVNDescriptor", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheAddSSVNDescriptor = (int (*) (int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sMerge", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheMerge = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   /* printf("pcon->p_isc_so->p_CacheAddGlobal=%p; pcon->p_isc_so->p_CacheAddGlobalDescriptor=%p; pcon->p_isc_so->p_CacheAddSSVN=%p; pcon->p_isc_so->p_CacheAddSSVNDescriptor=%p; pcon->p_isc_so->p_CacheMerge=%p;",  pcon->p_isc_so->p_CacheAddGlobal, pcon->p_isc_so->p_CacheAddGlobalDescriptor, pcon->p_isc_so->p_CacheAddSSVN, pcon->p_isc_so->p_CacheAddSSVNDescriptor, pcon->p_isc_so->p_CacheMerge); */

   if (pcon->p_isc_so->p_CacheAddGlobal && pcon->p_isc_so->p_CacheAddGlobalDescriptor && pcon->p_isc_so->p_CacheAddSSVN && pcon->p_isc_so->p_CacheAddSSVNDescriptor && pcon->p_isc_so->p_CacheMerge) {
      pcon->p_isc_so->merge_enabled = 1;
   }

   sprintf(fun, "%sPushFunc", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushFunc = (int (*) (unsigned int *, int, const Callin_char_t *, int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sExtFun", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExtFun = (int (*) (unsigned int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPushRtn", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushRtn = (int (*) (unsigned int *, int, const Callin_char_t *, int, const Callin_char_t *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sDoFun", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheDoFun = (int (*) (unsigned int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sDoRtn", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheDoRtn = (int (*) (unsigned int, int)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sCloseOref", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheCloseOref = (int (*) (unsigned int oref)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sIncrementCountOref", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheIncrementCountOref = (int (*) (unsigned int oref)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPopOref", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePopOref = (int (*) (unsigned int * orefp)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPushOref", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushOref = (int (*) (unsigned int oref)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sInvokeMethod", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheInvokeMethod = (int (*) (int narg)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPushMethod", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushMethod = (int (*) (unsigned int oref, int mlen, const Callin_char_t * mptr, int flg)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sInvokeClassMethod", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheInvokeClassMethod = (int (*) (int narg)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPushClassMethod", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushClassMethod = (int (*) (int clen, const Callin_char_t * cptr, int mlen, const Callin_char_t * mptr, int flg)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sGetProperty", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheGetProperty = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sSetProperty", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheSetProperty = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sPushProperty", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CachePushProperty = (int (*) (unsigned int oref, int plen, const Callin_char_t * pptr)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sType", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheType = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sEvalA", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheEvalA = (int (*) (CACHE_ASTRP volatile)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sExecuteA", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheExecuteA = (int (*) (CACHE_ASTRP volatile)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sConvert", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheConvert = (int (*) (unsigned long, void *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sErrorA", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheErrorA = (int (*) (CACHE_ASTRP, CACHE_ASTRP, int *)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);
   sprintf(fun, "%sErrxlateA", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheErrxlateA = (int (*) (int, CACHE_ASTRP)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   sprintf(fun, "%sEnableMultiThread", pcon->p_isc_so->funprfx);
   pcon->p_isc_so->p_CacheEnableMultiThread = (int (*) (void)) mg_dso_sym(pcon->p_isc_so->p_library, (char *) fun);

   pcon->pid = mg_current_process_id();

   pcon->p_isc_so->loaded = 1;

isc_load_library_exit:

   if (pweb->error[0]) {
      pcon->p_isc_so->loaded = 0;
      pweb->error_code = 1009;
      result = CACHE_NOCON;
      return result;
   }

   return CACHE_SUCCESS;
}


int isc_authenticate(MGWEB *pweb)
{
   unsigned char reopen;
   int termflag, timeout;
   char buffer[256];
	CACHESTR pin, *ppin;
	CACHESTR pout, *ppout;
	CACHESTR pusername;
	CACHESTR ppassword;
	CACHESTR pexename;
	int rc;
   DBXCON *pcon;

   pcon = pweb->pcon;

   reopen = 0;
   termflag = 0;
	timeout = 15;

isc_authenticate_reopen:

   pweb->error_code = 0;
   *pweb->error = '\0';
	strcpy((char *) pexename.str, "mg_web");
	pexename.len = (unsigned short) strlen((char *) pexename.str);
/*
	strcpy((char *) pin.str, "//./nul");
	strcpy((char *) pout.str, "//./nul");
	pin.len = (unsigned short) strlen((char *) pin.str);
	pout.len = (unsigned short) strlen((char *) pout.str);
*/
   ppin = NULL;
   if (pcon->psrv->input_device) {
	   strcpy(buffer, pcon->psrv->input_device);
      mg_lcase(buffer);
      if (!strcmp(buffer, "stdin")) {
	      strcpy((char *) pin.str, "");
         ppin = &pin;
      }
      else if (strcmp(pcon->psrv->input_device, DBX_NULL_DEVICE)) {
	      strcpy((char *) pin.str, pcon->psrv->input_device);
         ppin = &pin;
      }
      if (ppin)
	      ppin->len = (unsigned short) strlen((char *) ppin->str);
   }
   ppout = NULL;
   if (pcon->psrv->output_device) {
	   strcpy(buffer, pcon->psrv->output_device);
      mg_lcase(buffer);
      if (!strcmp(buffer, "stdout")) {
	      strcpy((char *) pout.str, "");
         ppout = &pout;
      }
      else if (strcmp(pcon->psrv->output_device, DBX_NULL_DEVICE)) {
	      strcpy((char *) pout.str, pcon->psrv->output_device);
         ppout = &pout;
      }
      if (ppout)
	      ppout->len = (unsigned short) strlen((char *) ppout->str);
   }

   if (ppin && ppout) { 
      termflag = CACHE_TTALL|CACHE_PROGMODE;
   }
   else {
      termflag = CACHE_TTNEVER|CACHE_PROGMODE;
   }

	strcpy((char *) pusername.str, pcon->psrv->username);
	strcpy((char *) ppassword.str, pcon->psrv->password);

	pusername.len = (unsigned short) strlen((char *) pusername.str);
	ppassword.len = (unsigned short) strlen((char *) ppassword.str);

#if !defined(_WIN32)

   signal(SIGUSR1, SIG_IGN);

#endif

	rc = pcon->p_isc_so->p_CacheSecureStartA(&pusername, &ppassword, &pexename, termflag, timeout, ppin, ppout);

	if (rc != CACHE_SUCCESS) {
      pweb->error_code = rc;
	   if (rc == CACHE_ACCESSDENIED) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : Access Denied : Check the audit log for the real authentication error (%d)\n", rc);
	      return 0;
	   }
	   if (rc == CACHE_CHANGEPASSWORD) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : Password Change Required (%d)\n", rc);
	      return 0;
	   }
	   if (rc == CACHE_ALREADYCON) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : Already Connected (%d)\n", rc);
	      return 1;
	   }
	   if (rc == CACHE_CONBROKEN) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : Connection was formed and then broken by the DB Server. (%d)\n", rc);
	      return 0;
	   }

	   if (rc == CACHE_FAILURE) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : An unexpected error has occurred. (%d)\n", rc);
	      return 0;
	   }
	   if (rc == CACHE_STRTOOLONG) {
	      sprintf(pweb->error, "Authentication: CacheSecureStart() : prinp or prout is too long. (%d)\n", rc);
	      return 0;
	   }
	   sprintf(pweb->error, "Authentication: CacheSecureStart() : Failed (%d)\n", rc);
	   return 0;
   }

   if (pcon->p_isc_so->p_CacheEvalA && pcon->p_isc_so->p_CacheConvert) {
      CACHE_ASTR retval;
      CACHE_ASTR expr;

      strcpy((char *) expr.str, "$ZVersion");
      expr.len = (unsigned short) strlen((char *) expr.str);
      rc = pcon->p_isc_so->p_CacheEvalA(&expr);

      if (rc == CACHE_CONBROKEN)
         reopen = 1;
      if (rc == CACHE_SUCCESS) {
         retval.len = 256;
         rc = pcon->p_isc_so->p_CacheConvert(CACHE_ASTRING, &retval);

         if (rc == CACHE_CONBROKEN)
            reopen = 1;
         if (rc == CACHE_SUCCESS) {
            isc_parse_zv((char *) retval.str, pcon->p_zv);
            sprintf(pcon->p_zv->version, "%d.%d.b%d", pcon->p_zv->majorversion, pcon->p_zv->minorversion, pcon->p_zv->mg_build);
         }
      }
   }

   if (reopen) {
      goto isc_authenticate_reopen;
   }

   rc = isc_change_namespace(pcon, pcon->psrv->uci);

   if (pcon->p_isc_so && pcon->p_isc_so->p_CacheEnableMultiThread) {
      rc = pcon->p_isc_so->p_CacheEnableMultiThread();
   }
   else {
      rc = -1;
   }

   return 1;
}


int isc_open(MGWEB *pweb)
{
   int rc, error_code, result;
   DBXCON *pcon;

   pcon = pweb->pcon;

   error_code = 0;
   rc = CACHE_SUCCESS;

   if (!pcon->p_isc_so) {
      pcon->p_isc_so = (DBXISCSO *) mg_malloc(NULL, sizeof(DBXISCSO), 13);
      if (!pcon->p_isc_so) {
         strcpy(pweb->error, "No Memory");
         pweb->error_code = 1009;
         result = CACHE_NOCON;
         return result;
      }
      memset((void *) pcon->p_isc_so, 0, sizeof(DBXISCSO));
      pcon->p_isc_so->loaded = 0;
   }

   if (pcon->p_isc_so->loaded == 2) {
      strcpy(pweb->error, "Cannot create multiple connections to the database");
      pweb->error_code = 1009; 
      rc = CACHE_NOCON;
      goto isc_open_exit;
   }

   if (!pcon->p_isc_so->loaded) {
      pcon->p_isc_so->merge_enabled = 0;
   }

   if (!pcon->p_isc_so->loaded) {
      rc = isc_load_library(pweb);
      if (rc != CACHE_SUCCESS) {
         goto isc_open_exit;
      }
   }

   rc = pcon->p_isc_so->p_CacheSetDir(pcon->psrv->shdir);

   if (!isc_authenticate(pweb)) {
      pweb->error_code = error_code;
      rc = CACHE_NOCON;
   }
   else {
      pcon->p_isc_so->loaded = 2;
      mg_isc_so_global = pcon->p_isc_so;
      rc = CACHE_SUCCESS;
   }

isc_open_exit:

   return rc;
}


int isc_parse_zv(char *zv, DBXZV * p_isc_sv)
{
   int result;
   double mg_version;
   char *p, *p1, *p2;

   p_isc_sv->mg_version = 0;
   p_isc_sv->majorversion = 0;
   p_isc_sv->minorversion = 0;
   p_isc_sv->mg_build = 0;
   p_isc_sv->vnumber = 0;

   result = 0;

   p = strstr(zv, "Cache");
   if (p) {
      p_isc_sv->product = DBX_DBTYPE_CACHE;
   }
   else {
      p_isc_sv->product = DBX_DBTYPE_IRIS;
   }

   p = zv;
   mg_version = 0;
   while (*(++ p)) {
      if (*(p - 1) == ' ' && isdigit((int) (*p))) {
         mg_version = strtod(p, NULL);
         if (*(p + 1) == '.' && mg_version >= 1.0 && mg_version <= 5.2)
            break;
         else if (*(p + 4) == '.' && mg_version >= 2000.0)
            break;
         mg_version = 0;
      }
   }

   if (mg_version > 0) {
      p_isc_sv->mg_version = mg_version;
      p_isc_sv->majorversion = (int) strtol(p, NULL, 10);
      p1 = strstr(p, ".");
      if (p1) {
         p_isc_sv->minorversion = (int) strtol(p1 + 1, NULL, 10);
      }
      p2 = strstr(p, "Build ");
      if (p2) {
         p_isc_sv->mg_build = (int) strtol(p2 + 6, NULL, 10);
      }

      if (p_isc_sv->majorversion >= 2007)
         p_isc_sv->vnumber = (((p_isc_sv->majorversion - 2000) * 100000) + (p_isc_sv->minorversion * 10000) + p_isc_sv->mg_build);
      else
         p_isc_sv->vnumber = ((p_isc_sv->majorversion * 100000) + (p_isc_sv->minorversion * 10000) + p_isc_sv->mg_build);

      result = 1;
   }

   return result;
}


int isc_change_namespace(DBXCON *pcon, char *nspace)
{
   int rc, len;
   CACHE_ASTR expr;

   len = (int) strlen(nspace);
   if (len == 0 || len > 64) {
      return CACHE_ERNAMSP;
   }
   if (pcon->p_isc_so->p_CacheExecuteA == NULL) {
      return CACHE_ERNAMSP;
   }

   sprintf((char *) expr.str, "ZN \"%s\"", nspace); /* changes namespace */
   expr.len = (unsigned short) strlen((char *) expr.str);

   mg_mutex_lock(pcon->p_db_mutex, 0);

   rc = pcon->p_isc_so->p_CacheExecuteA(&expr);

   mg_mutex_unlock(pcon->p_db_mutex);

   return rc;
}


int isc_pop_value(DBXCON *pcon, DBXVAL *value, int required_type)
{
   int rc, ex, ctype, oref;

   ex = 0;
   ctype = CACHE_ASTRING;

   if (pcon->p_isc_so->p_CacheType) {
      ctype = pcon->p_isc_so->p_CacheType();

      if (ctype == CACHE_OREF) {
         rc = pcon->p_isc_so->p_CachePopOref((unsigned int *) &oref);

         value->type = DBX_DTYPE_OREF;
         value->num.oref = oref;
         sprintf((char *) value->svalue.buf_addr, "%d", oref);
         value->svalue.len_used += (int) strlen((char *) value->svalue.buf_addr);
         mg_add_block_size((unsigned char *) value->svalue.buf_addr, 0, (unsigned long) value->svalue.len_used, DBX_DSORT_DATA, DBX_DTYPE_OREF);

         return rc;
      }
   }
   else {
      ctype = CACHE_ASTRING;
   }

   if (ex) {
      rc = pcon->p_isc_so->p_CachePopExStr(&(value->cvalue.zstr));
      value->api_size = value->cvalue.zstr.len;
      value->num.str = (char *) value->cvalue.zstr.str.ch;
      if (!value->cvalue.pstr) {
         value->cvalue.pstr = (void *) value->num.str;
      }
   }
   else {
      rc = pcon->p_isc_so->p_CachePopStr((int *) &(value->api_size), (Callin_char_t **) &(value->num.str));
      value->cvalue.pstr = NULL;
   }

   /* v2.2.18 */
/*
{
   char bufferx[256];
   sprintf(bufferx, "isc_pop_value: rc=%d; ex=%d; value->api_size=%d; value->num.str=%p; value->cvalue.pstr=%p;", rc, ex, value->api_size, value->num.str, value->cvalue.pstr);
   mg_log_event(mg_system.plog, NULL, bufferx, "isc_pop_value", 0);
   if (value->num.str) {
      mg_log_buffer(mg_system.plog, NULL, value->num.str, 16, "isc_pop_value", 0);
   }
}
*/

   return rc;
}


int isc_error_message(MGWEB *pweb, int error_code)
{
   int size, size1, len;
   CACHE_ASTR *pcerror;
   DBXCON *pcon;

   pcon = pweb->pcon;

   size = DBX_ERROR_SIZE;

   if (pcon) {
      if (error_code < 0) {
         pweb->error_code = 900 + (error_code * -1);
      }
      else {
         pweb->error_code = error_code;
      }
   }
   else {
      return 0;
   }

   if (pweb->error[0]) {
      goto isc_error_message_exit;
   }

   pweb->error[0] = '\0';

   size1 = size;

   if (pcon->p_isc_so && pcon->p_isc_so->p_CacheErrxlateA) {
      pcerror = (CACHE_ASTR *) mg_malloc(NULL, sizeof(CACHE_ASTR), 14);
      if (pcerror) {
         pcerror->str[0] = '\0';
         pcerror->len = 50;
         pcon->p_isc_so->p_CacheErrxlateA(error_code, pcerror);

         pcerror->str[50] = '\0';

         if (pcerror->len > 0) {
            len = pcerror->len;
            if (len >= DBX_ERROR_SIZE) {
               len = DBX_ERROR_SIZE - 1;
            }
            strncpy(pweb->error, (char *) pcerror->str, len);
            pweb->error[len] = '\0';
         }
         mg_free(NULL, (void *) pcerror, 12);
         size1 -= (int) strlen(pweb->error);
      }
   }

   switch (error_code) {
      case CACHE_SUCCESS:
         strncat(pweb->error, "Operation completed successfully!", size1 - 1);
         break;
      case CACHE_ACCESSDENIED:
         strncat(pweb->error, "Authentication has failed. Check the audit log for the real authentication error.", size1 - 1);
         break;
      case CACHE_ALREADYCON:
         strncat(pweb->error, "Connection already existed. Returned if you call CacheSecureStartH from a $ZF function.", size1 - 1);
         break;
      case CACHE_CHANGEPASSWORD:
         strncat(pweb->error, "Password change required. This return value is only returned if you are using Cach\xe7 authentication.", size1 - 1);
         break;
      case CACHE_CONBROKEN:
         strncat(pweb->error, "Connection was broken by the DB Server. Check arguments for validity.", size1 - 1);
         break;
      case CACHE_FAILURE:
         strncat(pweb->error, "An unexpected error has occurred.", size1 - 1);
         break;
      case CACHE_STRTOOLONG:
         strncat(pweb->error, "String is too long.", size1 - 1);
         break;
      case CACHE_NOCON:
         strncat(pweb->error, "No connection has been established.", size1 - 1);
         break;
      case CACHE_ERSYSTEM:
         strncat(pweb->error, "Either the Cache engine generated a <SYSTEM> error, or callin detected an internal data inconsistency.", size1 - 1);
         break;
      case CACHE_ERARGSTACK:
         strncat(pweb->error, "Argument stack overflow.", size1 - 1);
         break;
      case CACHE_ERSTRINGSTACK:
         strncat(pweb->error, "String stack overflow.", size1 - 1);
         break;
      case CACHE_ERPROTECT:
         strncat(pweb->error, "Protection violation.", size1 - 1);
         break;
      case CACHE_ERUNDEF:
         strncat(pweb->error, "Global node is undefined", size1 - 1);
         break;
      case CACHE_ERUNIMPLEMENTED:
         strncat(pweb->error, "String is undefined OR feature is not implemented.", size1 - 1);
         break;
      case CACHE_ERSUBSCR:
         strncat(pweb->error, "Subscript error in Global node (subscript null/empty or too long)", size1 - 1);
         break;
      case CACHE_ERNOROUTINE:
         strncat(pweb->error, "Routine does not exist", size1 - 1);
         break;
      case CACHE_ERNOLINE:
         strncat(pweb->error, "Function does not exist in routine", size1 - 1);
         break;
      case CACHE_ERPARAMETER:
         strncat(pweb->error, "Function arguments error", size1 - 1);
         break;
      case CACHE_BAD_GLOBAL:
         strncat(pweb->error, "Invalid global name", size1 - 1);
         break;
      case CACHE_BAD_NAMESPACE:
         strncat(pweb->error, "Invalid NameSpace name", size1 - 1);
         break;
      case CACHE_BAD_FUNCTION:
         strncat(pweb->error, "Invalid function name", size1 - 1);
         break;
      case CACHE_BAD_CLASS:
         strncat(pweb->error, "Invalid class name", size1 - 1);
         break;
      case CACHE_BAD_METHOD:
         strncat(pweb->error, "Invalid method name", size1 - 1);
         break;
      case CACHE_ERNOCLASS:
         strncat(pweb->error, "Class does not exist", size1 - 1);
         break;
      case CACHE_ERBADOREF:
         strncat(pweb->error, "Invalid Object Reference", size1 - 1);
         break;
      case CACHE_ERNOMETHOD:
         strncat(pweb->error, "Method does not exist", size1 - 1);
         break;
      case CACHE_ERNOPROPERTY:
         strncat(pweb->error, "Property does not exist", size1 - 1);
         break;
      case CACHE_ETIMEOUT:
         strncat(pweb->error, "Operation timed out", size1 - 1);
         break;
      case CACHE_BAD_STRING:
         strncat(pweb->error, "Invalid string", size1 - 1);
         break;
      case CACHE_ERNAMSP:
         strncat(pweb->error, "Invalid Namespace", size1 - 1);
         break;
      default:
         strncat(pweb->error, "DB Server Error", size1 - 1);
         break;
   }
   pweb->error[size - 1] = '\0';

isc_error_message_exit:

   return 0;
}


int ydb_load_library(MGWEB *pweb)
{
   int n, len, result;
   char primlib[DBX_ERROR_SIZE], primerr[DBX_ERROR_SIZE];
   char verfile[256], fun[64];
   char *libnam[16];
   DBXCON *pcon;

   pcon = pweb->pcon;

   strcpy(pcon->p_ydb_so->libdir, pcon->psrv->shdir);
   strcpy(pcon->p_ydb_so->funprfx, "ydb");
   strcpy(pcon->p_ydb_so->dbname, "YottaDB");

   strcpy(verfile, pcon->psrv->shdir);
   len = (int) strlen(pcon->p_ydb_so->libdir);
   if (pcon->p_ydb_so->libdir[len - 1] != '/' && pcon->p_ydb_so->libdir[len - 1] != '\\') {
      pcon->p_ydb_so->libdir[len] = '/';
      len ++;
   }

   n = 0;
#if defined(_WIN32)
   libnam[n ++] = (char *) DBX_YDB_DLL;
#else
#if defined(MACOSX)
   libnam[n ++] = (char *) DBX_YDB_DYLIB;
   libnam[n ++] = (char *) DBX_YDB_SO;
#else
   libnam[n ++] = (char *) DBX_YDB_SO;
   libnam[n ++] = (char *) DBX_YDB_DYLIB;
#endif
#endif

   libnam[n ++] = NULL;
   strcpy(pcon->p_ydb_so->libnam, pcon->p_ydb_so->libdir);
   len = (int) strlen(pcon->p_ydb_so->libnam);

   for (n = 0; libnam[n]; n ++) {
      strcpy(pcon->p_ydb_so->libnam + len, libnam[n]);
      if (!n) {
         strcpy(primlib, pcon->p_ydb_so->libnam);
      }

      pcon->p_ydb_so->p_library = mg_dso_load(pcon->p_ydb_so->libnam);
      if (pcon->p_ydb_so->p_library) {
         break;
      }

      if (!n) {
         int len1, len2;
         char *p;
#if defined(_WIN32)
         DWORD errorcode;
         LPVOID lpMsgBuf;

         lpMsgBuf = NULL;
         errorcode = GetLastError();
         sprintf(pweb->error, "Error loading %s Library: %s; Error Code : %ld",  pcon->p_ydb_so->dbname, primlib, errorcode);
         len2 = (int) strlen(pweb->error);
         len1 = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        errorcode,
                        /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), */
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL 
                        );
         if (lpMsgBuf && len1 > 0 && (DBX_ERROR_SIZE - len2) > 30) {
            strncpy(primerr, (const char *) lpMsgBuf, DBX_ERROR_SIZE - 1);
            p = strstr(primerr, "\r\n");
            if (p)
               *p = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            p = strstr(primerr, "%1");
            if (p) {
               *p = 'I';
               *(p + 1) = 't';
            }
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
         if (lpMsgBuf)
            LocalFree(lpMsgBuf);
#else
         p = (char *) dlerror();
         sprintf(primerr, "Cannot load %s library: Error Code: %d", pcon->p_ydb_so->dbname, errno);
         len2 = strlen(pweb->error);
         if (p) {
            strncpy(primerr, p, DBX_ERROR_SIZE - 1);
            primerr[DBX_ERROR_SIZE - 1] = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
#endif
      }
   }

   if (!pcon->p_ydb_so->p_library) {
      goto ydb_load_library_exit;
   }

   sprintf(fun, "%s_init", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_init = (int (*) (void)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_init) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_exit", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_exit = (int (*) (void)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_exit) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_malloc", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_malloc = (int (*) (size_t)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_malloc) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_free", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_free = (int (*) (void *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_free) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_data_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_data_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, unsigned int *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_data_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_delete_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_delete_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, int)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_delete_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_set_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_set_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_set_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_get_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_get_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_get_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_subscript_next_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_subscript_next_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_subscript_next_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_subscript_previous_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_subscript_previous_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_subscript_previous_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_node_next_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_node_next_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, int *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_node_next_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_node_previous_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_node_previous_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, int *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_node_previous_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_incr_s", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_incr_s = (int (*) (ydb_buffer_t *, int, ydb_buffer_t *, ydb_buffer_t *, ydb_buffer_t *)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_incr_s) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_ci", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_ci = (int (*) (const char *, ...)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_ci) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }
   sprintf(fun, "%s_cip", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_cip = (int (*) (ci_name_descriptor *, ...)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);
   if (!pcon->p_ydb_so->p_ydb_cip) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_ydb_so->dbname, pcon->p_ydb_so->libnam, fun);
      goto ydb_load_library_exit;
   }

   sprintf(fun, "%s_zstatus", pcon->p_ydb_so->funprfx);
   pcon->p_ydb_so->p_ydb_zstatus = (void (*) (ydb_char_t *, ydb_long_t)) mg_dso_sym(pcon->p_ydb_so->p_library, (char *) fun);

   pcon->pid = mg_current_process_id();

   pcon->p_ydb_so->loaded = 1;

ydb_load_library_exit:

   if (pweb->error[0]) {
      pcon->p_ydb_so->loaded = 0;
      pweb->error_code = 1009;
      result = CACHE_NOCON;
      return result;
   }

   return CACHE_SUCCESS;
}


int ydb_open(MGWEB *pweb)
{
   int rc, result;
   char buffer[256], buffer1[256];
   ydb_buffer_t zv, data;
   DBXCON *pcon;

   pcon = pweb->pcon;

   pweb->error_code = 0;
   rc = CACHE_SUCCESS;

   if (!pcon->p_ydb_so) {
      pcon->p_ydb_so = (DBXYDBSO *) mg_malloc(NULL, sizeof(DBXYDBSO), 15);
      if (!pcon->p_ydb_so) {
         strcpy(pweb->error, "No Memory");
         pweb->error_code = 1009; 
         result = CACHE_NOCON;
         return result;
      }
      memset((void *) pcon->p_ydb_so, 0, sizeof(DBXYDBSO));
      pcon->p_ydb_so->loaded = 0;
   }

   if (pcon->p_ydb_so->loaded == 2) {
      strcpy(pweb->error, "Cannot create multiple connections to the database");
      pweb->error_code = 1009; 
      rc = CACHE_NOCON;
      goto ydb_open_exit;
   }

   if (!pcon->p_ydb_so->loaded) {
      rc = ydb_load_library(pweb);
      if (rc != CACHE_SUCCESS) {
         goto ydb_open_exit;
      }
   }

   rc = pcon->p_ydb_so->p_ydb_init();

   if (rc != YDB_OK) { /* v2.7.35 */
      if (pcon->p_ydb_so->p_ydb_zstatus) {
         pcon->p_ydb_so->p_ydb_zstatus((ydb_char_t *) pweb->error, (ydb_long_t) DBX_ERROR_SIZE - 1);
         mg_log_event(pweb->plog, pweb, pweb->error, "ydb_init: YottaDB API error text", 0);
      }
      goto ydb_open_exit;
   }

   strcpy(buffer, "$zv");
   zv.buf_addr = buffer;
   zv.len_used = (int) strlen(buffer);
   zv.len_alloc = 255;

   data.buf_addr = buffer1;
   data.len_used = 0;
   data.len_alloc = 255;

   rc = pcon->p_ydb_so->p_ydb_get_s(&zv, 0, NULL, &data);

   if (data.len_used > 0) {
      data.buf_addr[data.len_used] = '\0';
   }

   if (rc == YDB_OK) {
      ydb_parse_zv(data.buf_addr, &(pcon->zv));
      sprintf(pcon->p_zv->version, "%d.%d.b%d", pcon->p_zv->majorversion, pcon->p_zv->minorversion, pcon->p_zv->mg_build);
      mg_ydb_so_global = pcon->p_ydb_so;
   }

ydb_open_exit:

   return rc;
}


int ydb_parse_zv(char *zv, DBXZV * p_ydb_sv)
{
   int result;
   double mg_version;
   char *p, *p1, *p2;

   p_ydb_sv->mg_version = 0;
   p_ydb_sv->majorversion = 0;
   p_ydb_sv->minorversion = 0;
   p_ydb_sv->mg_build = 0;
   p_ydb_sv->vnumber = 0;

   result = 0;
   /* GT.M V6.3-004 Linux x86_64 */

   p_ydb_sv->product = DBX_DBTYPE_YOTTADB;

   p = zv;
   mg_version = 0;
   while (*(++ p)) {
      if (*(p - 1) == 'V' && isdigit((int) (*p))) {
         mg_version = strtod(p, NULL);
         break;
      }
   }

   if (mg_version > 0) {
      p_ydb_sv->mg_version = mg_version;
      p_ydb_sv->majorversion = (int) strtol(p, NULL, 10);
      p1 = strstr(p, ".");
      if (p1) {
         p_ydb_sv->minorversion = (int) strtol(p1 + 1, NULL, 10);
      }
      p2 = strstr(p, "-");
      if (p2) {
         p_ydb_sv->mg_build = (int) strtol(p2 + 1, NULL, 10);
      }

      p_ydb_sv->vnumber = ((p_ydb_sv->majorversion * 100000) + (p_ydb_sv->minorversion * 10000) + p_ydb_sv->mg_build);

      result = 1;
   }
/*
   printf("\r\n ydb_parse_zv : p_ydb_sv->majorversion=%d; p_ydb_sv->minorversion=%d; p_ydb_sv->mg_build=%d; p_ydb_sv->mg_version=%f;", p_ydb_sv->majorversion, p_ydb_sv->minorversion, p_ydb_sv->mg_build, p_ydb_sv->mg_version);
*/
   return result;
}


int ydb_error_message(MGWEB *pweb, int error_code)
{
   int rc;
   char buffer[256], buffer1[256];
   ydb_buffer_t zstatus, data;
   DBXCON *pcon;

   pcon = pweb->pcon;
   rc = 0;
   if (pcon->p_ydb_so && pcon->p_ydb_so->p_ydb_get_s) {
      strcpy(buffer, "$zstatus");
      zstatus.buf_addr = buffer;
      zstatus.len_used = (int) strlen(buffer);
      zstatus.len_alloc = 255;

      data.buf_addr = buffer1;
      data.len_used = 0;
      data.len_alloc = 255;

      rc = pcon->p_ydb_so->p_ydb_get_s(&zstatus, 0, NULL, &data);

      if (data.len_used > 0) {
         data.buf_addr[data.len_used] = '\0';
      }

      strcpy(pweb->error, data.buf_addr);
   }
   else {
      if (!pweb->error[0]) {
         strcpy(pweb->error, "No connection has been established");
      }
   }

   return rc;
}


int gtm_load_library(MGWEB *pweb)
{
   int n, len, result;
   char primlib[DBX_ERROR_SIZE], primerr[DBX_ERROR_SIZE];
   char verfile[256], fun[64];
   char *libnam[16];
   DBXCON *pcon;

   pcon = pweb->pcon;

   strcpy(pcon->p_gtm_so->libdir, pcon->psrv->shdir);
   strcpy(pcon->p_gtm_so->funprfx, "gtm");
   strcpy(pcon->p_gtm_so->dbname, "GT.M");

   strcpy(verfile, pcon->psrv->shdir);
   len = (int) strlen(pcon->p_gtm_so->libdir);
   if (pcon->p_gtm_so->libdir[len - 1] != '/' && pcon->p_gtm_so->libdir[len - 1] != '\\') {
      pcon->p_gtm_so->libdir[len] = '/';
      len ++;
   }

   n = 0;
#if defined(_WIN32)
   libnam[n ++] = (char *) DBX_GTM_DLL;
#else
#if defined(MACOSX)
   libnam[n ++] = (char *) DBX_GTM_DYLIB;
   libnam[n ++] = (char *) DBX_GTM_SO;
#else
   libnam[n ++] = (char *) DBX_GTM_SO;
   libnam[n ++] = (char *) DBX_GTM_DYLIB;
#endif
#endif

   libnam[n ++] = NULL;
   strcpy(pcon->p_gtm_so->libnam, pcon->p_gtm_so->libdir);
   len = (int) strlen(pcon->p_gtm_so->libnam);

   for (n = 0; libnam[n]; n ++) {
      strcpy(pcon->p_gtm_so->libnam + len, libnam[n]);
      if (!n) {
         strcpy(primlib, pcon->p_gtm_so->libnam);
      }

      pcon->p_gtm_so->p_library = mg_dso_load(pcon->p_gtm_so->libnam);
      if (pcon->p_gtm_so->p_library) {
         break;
      }

      if (!n) {
         int len1, len2;
         char *p;
#if defined(_WIN32)
         DWORD errorcode;
         LPVOID lpMsgBuf;

         lpMsgBuf = NULL;
         errorcode = GetLastError();
         sprintf(pweb->error, "Error loading %s Library: %s; Error Code : %ld",  pcon->p_gtm_so->dbname, primlib, errorcode);
         len2 = (int) strlen(pweb->error);
         len1 = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        errorcode,
                        /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), */
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL 
                        );
         if (lpMsgBuf && len1 > 0 && (DBX_ERROR_SIZE - len2) > 30) {
            strncpy(primerr, (const char *) lpMsgBuf, DBX_ERROR_SIZE - 1);
            p = strstr(primerr, "\r\n");
            if (p)
               *p = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            p = strstr(primerr, "%1");
            if (p) {
               *p = 'I';
               *(p + 1) = 't';
            }
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
         if (lpMsgBuf)
            LocalFree(lpMsgBuf);
#else
         p = (char *) dlerror();
         sprintf(primerr, "Cannot load %s library: Error Code: %d", pcon->p_gtm_so->dbname, errno);
         len2 = strlen(pweb->error);
         if (p) {
            strncpy(primerr, p, DBX_ERROR_SIZE - 1);
            primerr[DBX_ERROR_SIZE - 1] = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            primerr[len1] = '\0';
            strcat(pweb->error, " (");
            strcat(pweb->error, primerr);
            strcat(pweb->error, ")");
         }
#endif
      }
   }

   if (!pcon->p_gtm_so->p_library) {
      goto gtm_load_library_exit;
   }

   sprintf(fun, "%s_init", pcon->p_gtm_so->funprfx);
   pcon->p_gtm_so->p_gtm_init = (int (*) (void)) mg_dso_sym(pcon->p_gtm_so->p_library, (char *) fun);
   if (!pcon->p_gtm_so->p_gtm_init) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_gtm_so->dbname, pcon->p_gtm_so->libnam, fun);
      goto gtm_load_library_exit;
   }
   sprintf(fun, "%s_exit", pcon->p_gtm_so->funprfx);
   pcon->p_gtm_so->p_gtm_exit = (int (*) (void)) mg_dso_sym(pcon->p_gtm_so->p_library, (char *) fun);
   if (!pcon->p_gtm_so->p_gtm_exit) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_gtm_so->dbname, pcon->p_gtm_so->libnam, fun);
      goto gtm_load_library_exit;
   }

   sprintf(fun, "%s_ci", pcon->p_gtm_so->funprfx);
   pcon->p_gtm_so->p_gtm_ci = (int (*) (const char *, ...)) mg_dso_sym(pcon->p_gtm_so->p_library, (char *) fun);
   if (!pcon->p_gtm_so->p_gtm_ci) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_gtm_so->dbname, pcon->p_gtm_so->libnam, fun);
      goto gtm_load_library_exit;
   }

   sprintf(fun, "%s_zstatus", pcon->p_gtm_so->funprfx);
   pcon->p_gtm_so->p_gtm_ci = (int (*) (const char *, ...)) mg_dso_sym(pcon->p_gtm_so->p_library, (char *) fun);
   if (!pcon->p_gtm_so->p_gtm_ci) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", pcon->p_gtm_so->dbname, pcon->p_gtm_so->libnam, fun);
      goto gtm_load_library_exit;
   }

   pcon->pid = mg_current_process_id();

   pcon->p_gtm_so->loaded = 1;

gtm_load_library_exit:

   if (pweb->error[0]) {
      pcon->p_gtm_so->loaded = 0;
      pweb->error_code = 1009;
      result = CACHE_NOCON;
      return result;
   }

   return CACHE_SUCCESS;
}


int gtm_open(MGWEB *pweb)
{
   int rc, result;
   char buffer[256];
   DBXCON *pcon;

   pcon = pweb->pcon;

   pweb->error_code = 0;
   rc = CACHE_SUCCESS;

   if (!pcon->p_gtm_so) {
      pcon->p_gtm_so = (DBXGTMSO *) mg_malloc(NULL, sizeof(DBXGTMSO), 16);
      if (!pcon->p_gtm_so) {
         strcpy(pweb->error, "No Memory");
         pweb->error_code = 1009; 
         result = CACHE_NOCON;
         return result;
      }
      memset((void *) pcon->p_gtm_so, 0, sizeof(DBXGTMSO));
      pcon->p_gtm_so->loaded = 0;
   }

   if (pcon->p_gtm_so->loaded == 2) {
      strcpy(pweb->error, "Cannot create multiple connections to the database");
      pweb->error_code = 1009; 
      rc = CACHE_NOCON;
      goto gtm_open_exit;
   }

   if (!pcon->p_gtm_so->loaded) {
      rc = gtm_load_library(pweb);
      if (rc != CACHE_SUCCESS) {
         goto gtm_open_exit;
      }
   }

   rc = pcon->p_gtm_so->p_gtm_init();

   rc = (int) pcon->p_gtm_so->p_gtm_ci("ifc_zmgsis", buffer, "0", "", "$zv");

   if (rc == CACHE_SUCCESS) {
      gtm_parse_zv(buffer, &(pcon->zv));
      sprintf(pcon->p_zv->version, "%d.%d.b%d", pcon->p_zv->majorversion, pcon->p_zv->minorversion, pcon->p_zv->mg_build);
   }

gtm_open_exit:

   return rc;
}


int gtm_parse_zv(char *zv, DBXZV * p_gtm_sv)
{
   int result;
   double mg_version;
   char *p, *p1, *p2;

   p_gtm_sv->mg_version = 0;
   p_gtm_sv->majorversion = 0;
   p_gtm_sv->minorversion = 0;
   p_gtm_sv->mg_build = 0;
   p_gtm_sv->vnumber = 0;

   result = 0;
   /* GT.M V6.3-004 Linux x86_64 */

   p_gtm_sv->product = DBX_DBTYPE_YOTTADB;

   p = zv;
   mg_version = 0;
   while (*(++ p)) {
      if (*(p - 1) == 'V' && isdigit((int) (*p))) {
         mg_version = strtod(p, NULL);
         break;
      }
   }

   if (mg_version > 0) {
      p_gtm_sv->mg_version = mg_version;
      p_gtm_sv->majorversion = (int) strtol(p, NULL, 10);
      p1 = strstr(p, ".");
      if (p1) {
         p_gtm_sv->minorversion = (int) strtol(p1 + 1, NULL, 10);
      }
      p2 = strstr(p, "-");
      if (p2) {
         p_gtm_sv->mg_build = (int) strtol(p2 + 1, NULL, 10);
      }

      p_gtm_sv->vnumber = ((p_gtm_sv->majorversion * 100000) + (p_gtm_sv->minorversion * 10000) + p_gtm_sv->mg_build);

      result = 1;
   }
/*
   printf("\r\n gtm_parse_zv : p_gtm_sv->majorversion=%d; p_gtm_sv->minorversion=%d; p_gtm_sv->mg_build=%d; p_gtm_sv->mg_version=%f;", p_gtm_sv->majorversion, p_gtm_sv->minorversion, p_gtm_sv->mg_build, p_gtm_sv->mg_version);
*/
   return result;
}


int gtm_error_message(MGWEB *pweb, int error_code)
{
   int rc;
   char buffer[256];
   DBXCON *pcon;

   pcon = pweb->pcon;
   if (pcon->p_gtm_so && pcon->p_gtm_so->p_gtm_zstatus) {
      pcon->p_gtm_so->p_gtm_zstatus(buffer, 255);
      strcpy(pweb->error, buffer);
      rc = CACHE_SUCCESS;
   }
   else {
      if (!pweb->error[0]) {
         strcpy(pweb->error, "No connection has been established");
      }
      rc = CACHE_NOCON;
   }

   return rc;
}


int mg_add_block_size(unsigned char *block, unsigned long offset, unsigned long data_len, int dsort, int dtype)
{
   mg_set_size((unsigned char *) block + offset, data_len);
   block[offset + 4] = (unsigned char) ((dsort * 20) + dtype);

   return 1;
}


unsigned long mg_get_block_size(unsigned char *block, unsigned long offset, int *dsort, int *dtype)
{
   unsigned long data_len;
   unsigned char uc;

   data_len = 0;
   uc = (unsigned char) block[offset + 4];
   *dtype = uc % 20;
   *dsort = uc / 20;
   if (*dsort != DBX_DSORT_STATUS) {
      data_len = mg_get_size((unsigned char *) block + offset);
   }

   /* printf("\r\n mg_get_block_size %x:%x:%x:%x dlen=%lu; offset=%lu; type=%d (%x);\r\n", block->str[offset + 0], block->str[offset + 1], block->str[offset + 2], block->str[offset + 3], data_len, offset, *type, block->str[offset + 4]); */

   if (!DBX_DSORT_ISVALID(*dsort)) {
      *dsort = DBX_DSORT_INVALID;
   }

   return data_len;
}


int mg_set_size(unsigned char *str, unsigned long data_len)
{
   str[0] = (unsigned char) (data_len >> 0);
   str[1] = (unsigned char) (data_len >> 8);
   str[2] = (unsigned char) (data_len >> 16);
   str[3] = (unsigned char) (data_len >> 24);

   return 0;
}


unsigned long mg_get_size(unsigned char *str)
{
   unsigned long size;

   size = ((unsigned char) str[0]) | (((unsigned char) str[1]) << 8) | (((unsigned char) str[2]) << 16) | (((unsigned char) str[3]) << 24);
   return size;
}


int mg_buf_init(MGBUF *p_buf, int size, int increment_size)
{
   int result;

   p_buf->p_buffer = (unsigned char *) mg_malloc(NULL, sizeof(char) * (size + 1), 17);
   if (p_buf->p_buffer) {
      *(p_buf->p_buffer) = '\0';
      result = 1;
   }
   else {
      result = 0;
      p_buf->p_buffer = (unsigned char *) mg_malloc(NULL, sizeof(char), 18);
      if (p_buf->p_buffer) {
         *(p_buf->p_buffer) = '\0';
         size = 1;
      }
      else
         size = 0;
   }

   p_buf->size = size;
   p_buf->increment_size = increment_size;
   p_buf->data_size = 0;

   return result;
}


int mg_buf_resize(MGBUF *p_buf, unsigned long size)
{
   if (size < DBX_BUFFER)
      return 1;

   if (size < p_buf->size)
      return 1;

   p_buf->p_buffer = (unsigned char *) mg_realloc(NULL, (void *) p_buf->p_buffer, 0, sizeof(char) * size, 1);
   p_buf->size = size;

   return 1;
}


int mg_buf_free(MGBUF *p_buf)
{
   if (!p_buf)
      return 0;

   if (p_buf->p_buffer)
      mg_free(NULL, (void *) p_buf->p_buffer, 13);

   p_buf->p_buffer = NULL;
   p_buf->size = 0;
   p_buf->increment_size = 0;
   p_buf->data_size = 0;

   return 1;
}


int mg_buf_cpy(MGBUF *p_buf, char *buffer, unsigned long size)
{
   unsigned long  result, req_size, csize, increment_size;

   result = 1;

   if (size == 0)
      size = (unsigned long) strlen(buffer);

   if (size == 0) {
      p_buf->data_size = 0;
      p_buf->p_buffer[p_buf->data_size] = '\0';
      return result;
   }

   req_size = size;
   if (req_size > p_buf->size) {
      csize = p_buf->size;
      increment_size = p_buf->increment_size;
      while (req_size > csize)
         csize = csize + p_buf->increment_size;
      mg_buf_free(p_buf);
      result = mg_buf_init(p_buf, (int) size, (int) increment_size);
   }
   if (result) {
      memcpy((void *) p_buf->p_buffer, (void *) buffer, size);
      p_buf->data_size = req_size;
      p_buf->p_buffer[p_buf->data_size] = '\0';
   }

   return result;
}


int mg_buf_cat(MGBUF *p_buf, char *buffer, unsigned long size)
{
   unsigned long int result, req_size, csize, tsize, increment_size;
   unsigned char *p_temp;

   result = 1;

   if (size == 0)
      size = (unsigned long ) strlen(buffer);

   if (size == 0)
      return result;

   p_temp = NULL;
   req_size = (size + p_buf->data_size);
   tsize = p_buf->data_size;
   if (req_size > p_buf->size) {
      csize = p_buf->size;
      increment_size = p_buf->increment_size;
      while (req_size > csize)
         csize = csize + p_buf->increment_size;
      p_temp = p_buf->p_buffer;
      result = mg_buf_init(p_buf, (int) csize, (int) increment_size);
      if (result) {
         if (p_temp) {
            memcpy((void *) p_buf->p_buffer, (void *) p_temp, tsize);
            p_buf->data_size = tsize;
            mg_free(NULL, (void *) p_temp, 14);
         }
      }
      else
         p_buf->p_buffer = p_temp;
   }
   if (result) {
      memcpy((void *) (p_buf->p_buffer + tsize), (void *) buffer, size);
      p_buf->data_size = req_size;
      p_buf->p_buffer[p_buf->data_size] = '\0';
   }

   return result;
}


void * mg_malloc(void *pweb_server, int size, short id)
{
   void *p;

   if (mg_ext_malloc && pweb_server) {
      p = (void *) mg_ext_malloc(pweb_server, (unsigned long) size);
   }
   else {
#if defined(_WIN32)
      p = (void *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 32);
#else
      p = (void *) malloc(size);
#endif
   }
/* v2.8.38 */
/*
   {
      char buffer[256];
      sprintf(buffer, "pweb_server=%p; p=%p; size=%d; id=%d;", pweb_server, p, size, id);
      mg_log_event(&(mg_system.log), NULL, buffer, "mg_web: mg_malloc", 0);
   }
*/
   /* printf("\nmg_malloc: size=%d; id=%d; p=%p;", size, id, p); */

   return p;
}


void * mg_realloc(void *pweb_server, void *p, int curr_size, int new_size, short id)
{
/* v2.8.38 */
/*
   {
      char buffer[256];
      sprintf(buffer, "pweb_server=%p; p=%p; curr_size=%d; new_size=%d; id=%d;", pweb_server, p, curr_size, new_size, id);
      mg_log_event(&(mg_system.log), NULL, buffer, "mg_web: mg_realloc", 0);
   }
*/
   if (mg_ext_realloc && pweb_server) {
      p = (void *) mg_ext_realloc(pweb_server, (void *) p, (unsigned long) new_size);
   }
   else {
      if (new_size >= curr_size) {
         if (p) {
            mg_free(NULL, (void *) p, 15);
         }

#if defined(_WIN32)
         p = (void *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, new_size + 32);
#else
         p = (void *) mg_malloc(NULL, new_size, id);
#endif
      }
   }

   /* printf("\r\n curr_size=%d; new_size=%d;\r\n", curr_size, new_size); */

   return p;
}


int mg_free(void *pweb_server, void *p, short id)
{
   /* printf("\nmg_free: id=%d; p=%p;", id, p); */

/* v2.8.38 */
/*
{
   char buffer[256];
   sprintf(buffer, "pweb_server=%p; p=%p; id=%d;", pweb_server, p, id);
   mg_log_event(&(mg_system.log), NULL, buffer, "mg_web: mg_free", 0);
}
*/

   if (mg_ext_free && pweb_server) {
      mg_ext_free(pweb_server, (void *) p);
   }
   else {
#if defined(_WIN32)
      HeapFree(GetProcessHeap(), 0, p);
#else
      free((void *) p);
#endif
   }

   return 0;
}


/* copy with possible overlap */
int mg_memcpy(void * to, void *from, size_t size)
{
   int n;
   char *p1, *p2;

   p1 = (char *) to;
   p2 = (char *) from;

   for (n = 0; n < (int) size; n ++) {
      *p1 ++ = *p2 ++;
   }
   return 0;
}


int mg_ucase(char *string)
{
#ifdef _UNICODE

   CharUpperA(string);
   return 1;

#else

   int n, chr;

   n = 0;
   while (string[n] != '\0') {
      chr = (int) string[n];
      if (chr >= 97 && chr <= 122)
         string[n] = (char) (chr - 32);
      n ++;
   }
   return 1;

#endif
}


int mg_lcase(char *string)
{
#ifdef _UNICODE

   CharLowerA(string);
   return 1;

#else

   int n, chr;

   n = 0;
   while (string[n] != '\0') {
      chr = (int) string[n];
      if (chr >= 65 && chr <= 90)
         string[n] = (char) (chr + 32);
      n ++;
   }
   return 1;

#endif
}


int mg_ccase(char *string)
{
   int n, chr;

   n = 0;
   while (string[n] != '\0') {
      chr = (int) string[n];
      if (!n || string[n - 1] == '-') {
         if (chr >= 97 && chr <= 122) {
            string[n] = (char) (chr - 32);
         }
      }
      else {
         if (chr >= 65 && chr <= 90) {
            string[n] = (char) (chr + 32);
         }
      }
      n ++;
   }
   return 1;
}


int mg_log_init(DBXLOG *p_log)
{
   p_log->log_errors = 0;
   p_log->log_verbose = 0;
   p_log->log_frames = 0;
   p_log->log_transmissions = 0;
   p_log->log_transmissions_to_webserver = 0;
   p_log->log_tls = 0;
   p_log->log_file[0] = '\0';
   p_log->log_level[0] = '\0';
   p_log->log_filter[0] = '\0';
   p_log->fun_no = 0;
   p_log->req_no = 0;

   p_log->product[0] = '\0';
   p_log->product_version[0] = '\0';

   return 0;
}


int mg_log_event(DBXLOG *p_log, MGWEB *pweb, char *message, char *title, int level)
{
   int len, n;
   char timestr[64], heading[1024], buffer[2048];
   char *p_buffer;
   time_t now = 0;
#if defined(_WIN32)
   HANDLE hLogfile = 0;
   DWORD dwPos = 0, dwBytesWritten = 0;
#else
   FILE *fp = NULL;
   struct flock lock;
#endif

#ifdef _WIN32
__try {
#endif

   now = time(NULL);
   sprintf(timestr, "%s", ctime(&now));
   for (n = 0; timestr[n] != '\0'; n ++) {
      if ((unsigned int) timestr[n] < 32) {
         timestr[n] = '\0';
         break;
      }
   }
/*
   sprintf(heading, ">>> Time: %s; Build: %s pid=%lu;tid=%lu;req_no=%lu;fun_no=%lu", timestr, (char *) DBX_VERSION, (unsigned long) mg_current_process_id(), (unsigned long) mg_current_thread_id(), p_log->req_no, p_log->fun_no);
*/

   if (pweb)
      sprintf(heading, ">>> Time: %s; Build: %s pid=%lu;tid=%lu;script_name=%s;", timestr, (char *) DBX_VERSION, (unsigned long) mg_current_process_id(), (unsigned long) mg_current_thread_id(), pweb->script_name_lc);
   else
      sprintf(heading, ">>> Time: %s; Build: %s pid=%lu;tid=%lu;", timestr, (char *) DBX_VERSION, (unsigned long) mg_current_process_id(), (unsigned long) mg_current_thread_id());

   len = (int) strlen(heading) + (int) strlen(title) + (int) strlen(message) + 20;

   if (len < 2000)
      p_buffer = buffer;
   else
      p_buffer = (char *) malloc(sizeof(char) * len);

   if (p_buffer == NULL)
      return 0;

   p_buffer[0] = '\0';
   strcpy(p_buffer, heading);
   strcat(p_buffer, "\r\n    ");
   strcat(p_buffer, title);
   strcat(p_buffer, "\r\n    ");
   strcat(p_buffer, message);
   len = (int) strlen(p_buffer) * sizeof(char);

#if defined(_WIN32)

   strcat(p_buffer, "\r\n");
   len = len + (2 * sizeof(char));
   hLogfile = CreateFileA(p_log->log_file, GENERIC_WRITE, FILE_SHARE_WRITE,
                         (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
   dwPos = SetFilePointer(hLogfile, 0, (LPLONG) NULL, FILE_END);
   LockFile(hLogfile, dwPos, 0, dwPos + len, 0);
   WriteFile(hLogfile, (LPTSTR) p_buffer, len, &dwBytesWritten, NULL);
   UnlockFile(hLogfile, dwPos, 0, dwPos + len, 0);
   CloseHandle(hLogfile);

#else /* UNIX or VMS */

   strcat(p_buffer, "\n");
   fp = fopen(p_log->log_file, "a");
   if (fp) {

      lock.l_type = F_WRLCK;
      lock.l_start = 0;
      lock.l_whence = SEEK_SET;
      lock.l_len = 0;
      n = fcntl(fileno(fp), F_SETLKW, &lock);

      fputs(p_buffer, fp);

      lock.l_type = F_UNLCK;
      lock.l_start = 0;
      lock.l_whence = SEEK_SET;
      lock.l_len = 0;
      n = fcntl(fileno(fp), F_SETLK, &lock);

      fclose(fp); /* v2.7.33 */
   }

#endif

   if (p_buffer != buffer)
      free((void *) p_buffer);

   return 1;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {
      return 0;
}

#endif

}


int mg_log_buffer(DBXLOG *p_log, MGWEB *pweb, char *buffer, int buffer_len, char *title, int level)
{
   unsigned int c, len, strt;
   int n, n1, nc, size;
   char tmp[16];
   char *p;

#ifdef _WIN32
__try {
#endif

   for (n = 0, nc = 0; n < buffer_len; n ++) {
      c = (unsigned int) buffer[n];
      if (c < 32 || c > 126)
         nc ++;
   }

   size = buffer_len + (nc * 4) + 32;
   p = (char *) malloc(sizeof(char) * size);
   if (!p)
      return 0;

   if (nc) {

      for (n = 0, nc = 0; n < buffer_len; n ++) {
         c = (unsigned int) buffer[n];
         if (c < 32 || c > 126) {
            sprintf((char *) tmp, "%02x", c);
            len = (int) strlen(tmp);
            if (len > 2)
               strt = len - 2;
            else
               strt = 0;
            p[nc ++] = '\\';
            p[nc ++] = 'x';
            for (n1 = strt; tmp[n1]; n1 ++)
               p[nc ++] = tmp[n1];
         }
         else
            p[nc ++] = buffer[n];
      }
      p[nc] = '\0';
   }
   else {
      strncpy(p, buffer, buffer_len);
      p[buffer_len] = '\0';
   }

   mg_log_event(p_log, pweb, (char *) p, title, level);

   free((void *) p);

   return 1;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {
      return 0;
}

#endif

}


DBXPLIB mg_dso_load(char * library)
{
   DBXPLIB p_library;

#if defined(_WIN32)
   p_library = LoadLibraryA(library);
#else
   p_library = dlopen(library, RTLD_NOW);
#endif

   return p_library;
}


DBXPROC mg_dso_sym(DBXPLIB p_library, char * symbol)
{
   DBXPROC p_proc;

#if defined(_WIN32)
   p_proc = GetProcAddress(p_library, symbol);
#else
   p_proc  = (void *) dlsym(p_library, symbol);
#endif

   return p_proc;
}



int mg_dso_unload(DBXPLIB p_library)
{

#if defined(_WIN32)
   FreeLibrary(p_library);
#else
   dlclose(p_library); 
#endif

   return 1;
}


int mg_thread_create(DBXTHR *pthr, DBX_THR_FUNCTION function, void * arg)
{
#if defined(_WIN32)
   int rc;
   DWORD creation_flags, stack_size;
   LPSECURITY_ATTRIBUTES thread_attributes;

   thread_attributes = NULL;
   stack_size = 0;
   creation_flags = 0;

   pthr->thread_handle = CreateThread(thread_attributes, stack_size, (DBX_THR_FUNCTION) function, (LPVOID) arg, creation_flags, (LPDWORD) &(pthr->thread_id));
   if (pthr->thread_handle)
      rc = CACHE_SUCCESS;
   else
      rc = CACHE_FAILURE;

   return rc;

#else

   int rc;
   size_t stack_size, new_stack_size;
   pthread_attr_t attr;

   stack_size = 0;
   new_stack_size = 0;

   pthread_attr_init(&attr);
   pthread_attr_getstacksize(&attr, &stack_size);
   new_stack_size = 0x60000; /* 393216 */

   pthread_attr_setstacksize(&attr, new_stack_size);

   rc = pthread_create(&(pthr->thread_id), &attr, (DBX_THR_FUNCTION) function, (void *) arg);

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

   return rc;
#endif

}


int mg_thread_terminate(DBXTHR *pthr)
{
   int rc;

#if defined(_WIN32)
   rc = -1;
   if (pthr->thread_handle) {
      if (TerminateThread(pthr->thread_handle, 0)) {
         rc = 0;
      }
   }
#else
#if defined(AIX) || defined(AIX5)
   rc = 0;
   if (pthread_kill(pthr->thread_id, SIGUSR1)) {
      rc = -1;
   }
#else
   rc = 0;
   if (pthread_cancel(pthr->thread_id)) {
      rc = -1;
   }
#endif
#endif

   return rc;
}


int mg_thread_detach(void)
{
   int rc;

#if defined(_WIN32)
   rc = 0;
#else
   rc = pthread_detach(pthread_self());
#endif

   return rc;
}


int mg_thread_join(DBXTHR *pthr)
{
   int rc;

   rc = 0;
#if defined(_WIN32)
   rc = (int) WaitForSingleObject(pthr->thread_handle, INFINITE);
#else
   rc = pthread_join(pthr->thread_id, NULL);
#endif

   return rc;
}


int mg_thread_exit(void)
{
#if defined(_WIN32)
   ExitThread(0);
#else
   pthread_exit(NULL);
#endif
}


DBXTHID mg_current_thread_id(void)
{
#if defined(_WIN32)
   return (DBXTHID) GetCurrentThreadId();
#else
   return (DBXTHID) pthread_self();
#endif
}


unsigned long mg_current_process_id(void)
{
#if defined(_WIN32)
   return (unsigned long) GetCurrentProcessId();
#else
   return ((unsigned long) getpid());
#endif
}


int mg_error_message(MGWEB *pweb, int error_code)
{
   int rc;

   if (pweb->pcon->psrv->dbtype == DBX_DBTYPE_YOTTADB) {
      rc = ydb_error_message(pweb, error_code);
   }
   else {
      rc = isc_error_message(pweb, error_code);
   }

   return rc;
}


int mg_cleanup(MGWEB *pweb)
{
   int rc;
   DBXCON *pcon;

   pcon = pweb->pcon;
   rc = CACHE_SUCCESS;

   if (pcon->net_connection) {
      return rc;
   }
   if (pcon->psrv->dbtype == DBX_DBTYPE_YOTTADB) {
      return rc;
   }
   else {
      if (pweb->output_val.cvalue.pstr) {
         DBX_LOCK(0);
         pcon->p_isc_so->p_CacheExStrKill(&(pweb->output_val.cvalue.zstr));
         DBX_UNLOCK();
         pweb->output_val.cvalue.pstr = NULL;
      }
   }

   return rc;
}


int mg_mutex_create(DBXMUTEX *p_mutex)
{
   int result;

   result = 0;
   if (p_mutex->created) {
      return result;
   }

#if defined(_WIN32)
   p_mutex->h_mutex = CreateMutex(NULL, FALSE, NULL);
   result = 0;
#else
   result = pthread_mutex_init(&(p_mutex->h_mutex), NULL);
#endif

   p_mutex->created = 1;
   p_mutex->stack = 0;
   p_mutex->thid = 0;

   return result;
}



int mg_mutex_lock(DBXMUTEX *p_mutex, int timeout)
{
   int result;
   DBXTHID tid;
#ifdef _WIN32
   DWORD result_wait;
#endif

   result = 0;

   if (!p_mutex->created) {
      return -1;
   }

   tid = mg_current_thread_id();
   if (p_mutex->thid == tid) {
      p_mutex->stack ++;
      /* printf("\r\n thread already owns lock : thid=%lu; stack=%d;\r\n", (unsigned long) tid, p_mutex->stack); */
      return 0; /* success - thread already owns lock */
   }

#if defined(_WIN32)
   if (timeout == 0) {
      result_wait = WaitForSingleObject(p_mutex->h_mutex, INFINITE);
   }
   else {
      result_wait = WaitForSingleObject(p_mutex->h_mutex, (timeout * 1000));
   }

   if (result_wait == WAIT_OBJECT_0) { /* success */
      result = 0;
   }
   else if (result_wait == WAIT_ABANDONED) {
      printf("\r\nmg_mutex_lock: Returned WAIT_ABANDONED state");
      result = -1;
   }
   else if (result_wait == WAIT_TIMEOUT) {
      printf("\r\nmg_mutex_lock: Returned WAIT_TIMEOUT state");
      result = -1;
   }
   else if (result_wait == WAIT_FAILED) {
      printf("\r\nmg_mutex_lock: Returned WAIT_FAILED state: Error Code: %d", GetLastError());
      result = -1;
   }
   else {
      printf("\r\nmg_mutex_lock: Returned Unrecognized state: %d", result_wait);
      result = -1;
   }
#else
   result = pthread_mutex_lock(&(p_mutex->h_mutex));
#endif

   p_mutex->thid = tid;
   p_mutex->stack = 0;

   return result;
}


int mg_mutex_unlock(DBXMUTEX *p_mutex)
{
   int result;
   DBXTHID tid;

   result = 0;

   if (!p_mutex->created) {
      return -1;
   }

   tid = mg_current_thread_id();
   if (p_mutex->thid == tid && p_mutex->stack) {
      /* printf("\r\n thread has stacked locks : thid=%lu; stack=%d;\r\n", (unsigned long) tid, p_mutex->stack); */
      p_mutex->stack --;
      return 0;
   }
   p_mutex->thid = 0;
   p_mutex->stack = 0;

#if defined(_WIN32)
   ReleaseMutex(p_mutex->h_mutex);
   result = 0;
#else
   result = pthread_mutex_unlock(&(p_mutex->h_mutex));
#endif /* #if defined(_WIN32) */

   return result;
}


int mg_mutex_destroy(DBXMUTEX *p_mutex)
{
   int result;

   if (!p_mutex->created) {
      return -1;
   }

#if defined(_WIN32)
   CloseHandle(p_mutex->h_mutex);
   result = 0;
#else
   result = pthread_mutex_destroy(&(p_mutex->h_mutex));
#endif

   p_mutex->created = 0;

   return result;
}


int mg_init_critical_section(void *p_crit)
{
#if defined(_WIN32)
   InitializeCriticalSection((LPCRITICAL_SECTION) p_crit);
#endif

   return 0;
}


int mg_delete_critical_section(void *p_crit)
{
#if defined(_WIN32)
   DeleteCriticalSection((LPCRITICAL_SECTION) p_crit);
#endif

   return 0;
}


int mg_enter_critical_section(void *p_crit)
{
   int result;

#if defined(_WIN32)
   EnterCriticalSection((LPCRITICAL_SECTION) p_crit);
   result = 0;
#else
   result = pthread_mutex_lock((pthread_mutex_t *) p_crit);
#endif
   return result;
}


int mg_leave_critical_section(void *p_crit)
{
   int result;

#if defined(_WIN32)
   LeaveCriticalSection((LPCRITICAL_SECTION) p_crit);
   result = 0;
#else
   result = pthread_mutex_unlock((pthread_mutex_t *) p_crit);
#endif
   return result;
}


int mg_sleep(unsigned long msecs)
{
#if defined(_WIN32)

   Sleep((DWORD) msecs);

#else

#if 1
   unsigned int secs, msecs_rem;

   secs = (unsigned int) (msecs / 1000);
   msecs_rem = (unsigned int) (msecs % 1000);

   /* printf("\n   ===> msecs=%ld; secs=%ld; msecs_rem=%ld", msecs, secs, msecs_rem); */

   if (secs > 0) {
      sleep(secs);
   }
   if (msecs_rem > 0) {
      usleep((useconds_t) (msecs_rem * 1000));
   }

#else
   unsigned int secs;

   secs = (unsigned int) (msecs / 1000);
   if (secs == 0)
      secs = 1;
   sleep(secs);

#endif

#endif

   return 0;
}


unsigned int mg_file_size(char *file)
{
   unsigned int size;

#if defined(_WIN32)
   BOOL ok;
   WIN32_FILE_ATTRIBUTE_DATA file_info;

   ok = GetFileAttributesEx(file, GetFileExInfoStandard, (void*) &file_info);
   if (ok)
      size = (unsigned int) (file_info.nFileSizeLow + ((UINT64) file_info.nFileSizeHigh << 32));
   else
      size = 0;

#else
   FILE *fp;

   fp = fopen(file, "r");
   if (fp) {
      fseek(fp, 0, SEEK_END);
      size = (unsigned int) ftell(fp);
      if (size < 1)
         size = 0;
      fclose(fp);
   }
   else
      size = 0;
#endif

   return size;
}


int netx_load_winsock(MGWEB *pweb, int context)
{
#if defined(_WIN32)
   int result, mem_locked;
   char buffer[1024];

   result = 0;
   mem_locked = 0;
   *buffer = '\0';
   netx_so.version_requested = 0;

   if (netx_so.load_attempted) {
      return result;
   }

   if (netx_so.load_attempted) {
      goto netx_load_winsock_no_so;
   }

   netx_so.sock = 0;

   /* Try to Load the Winsock 2 library */

   netx_so.winsock = 2;
   strcpy(netx_so.libnam, "WS2_32.DLL");

   netx_so.plibrary = mg_dso_load(netx_so.libnam);

   if (!netx_so.plibrary) {
      netx_so.winsock = 1;
      strcpy(netx_so.libnam, "WSOCK32.DLL");
      netx_so.plibrary = mg_dso_load(netx_so.libnam);

      if (!netx_so.plibrary) {
         goto netx_load_winsock_no_so;
      }
   }

   netx_so.p_WSASocket             = (MG_LPFN_WSASOCKET)              mg_dso_sym(netx_so.plibrary, "WSASocketA");
   netx_so.p_WSAGetLastError       = (MG_LPFN_WSAGETLASTERROR)        mg_dso_sym(netx_so.plibrary, "WSAGetLastError");
   netx_so.p_WSAStartup            = (MG_LPFN_WSASTARTUP)             mg_dso_sym(netx_so.plibrary, "WSAStartup");
   netx_so.p_WSACleanup            = (MG_LPFN_WSACLEANUP)             mg_dso_sym(netx_so.plibrary, "WSACleanup");
   netx_so.p_WSAFDIsSet            = (MG_LPFN_WSAFDISSET)             mg_dso_sym(netx_so.plibrary, "__WSAFDIsSet");
   netx_so.p_WSARecv               = (MG_LPFN_WSARECV)                mg_dso_sym(netx_so.plibrary, "WSARecv");
   netx_so.p_WSASend               = (MG_LPFN_WSASEND)                mg_dso_sym(netx_so.plibrary, "WSASend");

#if defined(NETX_IPV6)
   netx_so.p_WSAStringToAddress    = (MG_LPFN_WSASTRINGTOADDRESS)     mg_dso_sym(netx_so.plibrary, "WSAStringToAddressA");
   netx_so.p_WSAAddressToString    = (MG_LPFN_WSAADDRESSTOSTRING)     mg_dso_sym(netx_so.plibrary, "WSAAddressToStringA");
   netx_so.p_getaddrinfo           = (MG_LPFN_GETADDRINFO)            mg_dso_sym(netx_so.plibrary, "getaddrinfo");
   netx_so.p_freeaddrinfo          = (MG_LPFN_FREEADDRINFO)           mg_dso_sym(netx_so.plibrary, "freeaddrinfo");
   netx_so.p_getnameinfo           = (MG_LPFN_GETNAMEINFO)            mg_dso_sym(netx_so.plibrary, "getnameinfo");
   netx_so.p_getpeername           = (MG_LPFN_GETPEERNAME)            mg_dso_sym(netx_so.plibrary, "getpeername");
   netx_so.p_inet_ntop             = (MG_LPFN_INET_NTOP)              mg_dso_sym(netx_so.plibrary, "InetNtop");
   netx_so.p_inet_pton             = (MG_LPFN_INET_PTON)              mg_dso_sym(netx_so.plibrary, "InetPton");
#else
   netx_so.p_WSAStringToAddress    = NULL;
   netx_so.p_WSAAddressToString    = NULL;
   netx_so.p_getaddrinfo           = NULL;
   netx_so.p_freeaddrinfo          = NULL;
   netx_so.p_getnameinfo           = NULL;
   netx_so.p_getpeername           = NULL;
   netx_so.p_inet_ntop             = NULL;
   netx_so.p_inet_pton             = NULL;
#endif

   netx_so.p_closesocket           = (MG_LPFN_CLOSESOCKET)            mg_dso_sym(netx_so.plibrary, "closesocket");
   netx_so.p_gethostname           = (MG_LPFN_GETHOSTNAME)            mg_dso_sym(netx_so.plibrary, "gethostname");
   netx_so.p_gethostbyname         = (MG_LPFN_GETHOSTBYNAME)          mg_dso_sym(netx_so.plibrary, "gethostbyname");
   netx_so.p_getservbyname         = (MG_LPFN_GETSERVBYNAME)          mg_dso_sym(netx_so.plibrary, "getservbyname");
   netx_so.p_gethostbyaddr         = (MG_LPFN_GETHOSTBYADDR)          mg_dso_sym(netx_so.plibrary, "gethostbyaddr");
   netx_so.p_htons                 = (MG_LPFN_HTONS)                  mg_dso_sym(netx_so.plibrary, "htons");
   netx_so.p_htonl                 = (MG_LPFN_HTONL)                  mg_dso_sym(netx_so.plibrary, "htonl");
   netx_so.p_ntohl                 = (MG_LPFN_NTOHL)                  mg_dso_sym(netx_so.plibrary, "ntohl");
   netx_so.p_ntohs                 = (MG_LPFN_NTOHS)                  mg_dso_sym(netx_so.plibrary, "ntohs");
   netx_so.p_connect               = (MG_LPFN_CONNECT)                mg_dso_sym(netx_so.plibrary, "connect");
   netx_so.p_inet_addr             = (MG_LPFN_INET_ADDR)              mg_dso_sym(netx_so.plibrary, "inet_addr");
   netx_so.p_inet_ntoa             = (MG_LPFN_INET_NTOA)              mg_dso_sym(netx_so.plibrary, "inet_ntoa");

   netx_so.p_socket                = (MG_LPFN_SOCKET)                 mg_dso_sym(netx_so.plibrary, "socket");
   netx_so.p_setsockopt            = (MG_LPFN_SETSOCKOPT)             mg_dso_sym(netx_so.plibrary, "setsockopt");
   netx_so.p_getsockopt            = (MG_LPFN_GETSOCKOPT)             mg_dso_sym(netx_so.plibrary, "getsockopt");
   netx_so.p_getsockname           = (MG_LPFN_GETSOCKNAME)            mg_dso_sym(netx_so.plibrary, "getsockname");

   netx_so.p_select                = (MG_LPFN_SELECT)                 mg_dso_sym(netx_so.plibrary, "select");
   netx_so.p_recv                  = (MG_LPFN_RECV)                   mg_dso_sym(netx_so.plibrary, "recv");
   netx_so.p_send                  = (MG_LPFN_SEND)                   mg_dso_sym(netx_so.plibrary, "send");
   netx_so.p_shutdown              = (MG_LPFN_SHUTDOWN)               mg_dso_sym(netx_so.plibrary, "shutdown");
   netx_so.p_bind                  = (MG_LPFN_BIND)                   mg_dso_sym(netx_so.plibrary, "bind");
   netx_so.p_listen                = (MG_LPFN_LISTEN)                 mg_dso_sym(netx_so.plibrary, "listen");
   netx_so.p_accept                = (MG_LPFN_ACCEPT)                 mg_dso_sym(netx_so.plibrary, "accept");

   if (   (netx_so.p_WSASocket              == NULL && netx_so.winsock == 2)
       ||  netx_so.p_WSAGetLastError        == NULL
       ||  netx_so.p_WSAStartup             == NULL
       ||  netx_so.p_WSACleanup             == NULL
       ||  netx_so.p_WSAFDIsSet             == NULL
       || (netx_so.p_WSARecv                == NULL && netx_so.winsock == 2)
       || (netx_so.p_WSASend                == NULL && netx_so.winsock == 2)

#if defined(NETX_IPV6)
       || (netx_so.p_WSAStringToAddress     == NULL && netx_so.winsock == 2)
       || (netx_so.p_WSAAddressToString     == NULL && netx_so.winsock == 2)
       ||  netx_so.p_getpeername            == NULL
#endif

       ||  netx_so.p_closesocket            == NULL
       ||  netx_so.p_gethostname            == NULL
       ||  netx_so.p_gethostbyname          == NULL
       ||  netx_so.p_getservbyname          == NULL
       ||  netx_so.p_gethostbyaddr          == NULL
       ||  netx_so.p_htons                  == NULL
       ||  netx_so.p_htonl                  == NULL
       ||  netx_so.p_ntohl                  == NULL
       ||  netx_so.p_ntohs                  == NULL
       ||  netx_so.p_connect                == NULL
       ||  netx_so.p_inet_addr              == NULL
       ||  netx_so.p_inet_ntoa              == NULL
       ||  netx_so.p_socket                 == NULL
       ||  netx_so.p_setsockopt             == NULL
       ||  netx_so.p_getsockopt             == NULL
       ||  netx_so.p_getsockname            == NULL
       ||  netx_so.p_select                 == NULL
       ||  netx_so.p_recv                   == NULL
       ||  netx_so.p_send                   == NULL
       ||  netx_so.p_shutdown               == NULL
       ||  netx_so.p_bind                   == NULL
       ||  netx_so.p_listen                 == NULL
       ||  netx_so.p_accept                 == NULL
      ) {

      sprintf(buffer, "Cannot use Winsock library (WSASocket=%p; WSAGetLastError=%p; WSAStartup=%p; WSACleanup=%p; WSAFDIsSet=%p; WSARecv=%p; WSASend=%p; WSAStringToAddress=%p; WSAAddressToString=%p; closesocket=%p; gethostname=%p; gethostbyname=%p; getservbyname=%p; gethostbyaddr=%p; getaddrinfo=%p; freeaddrinfo=%p; getnameinfo=%p; getpeername=%p; htons=%p; htonl=%p; ntohl=%p; ntohs=%p; connect=%p; inet_addr=%p; inet_ntoa=%p; socket=%p; setsockopt=%p; getsockopt=%p; getsockname=%p; select=%p; recv=%p; p_send=%p; shutdown=%p; bind=%p; listen=%p; accept=%p;)",
            netx_so.p_WSASocket,
            netx_so.p_WSAGetLastError,
            netx_so.p_WSAStartup,
            netx_so.p_WSACleanup,
            netx_so.p_WSAFDIsSet,
            netx_so.p_WSARecv,
            netx_so.p_WSASend,

            netx_so.p_WSAStringToAddress,
            netx_so.p_WSAAddressToString,

            netx_so.p_closesocket,
            netx_so.p_gethostname,
            netx_so.p_gethostbyname,
            netx_so.p_getservbyname,
            netx_so.p_gethostbyaddr,

            netx_so.p_getaddrinfo,
            netx_so.p_freeaddrinfo,
            netx_so.p_getnameinfo,
            netx_so.p_getpeername,

            netx_so.p_htons,
            netx_so.p_htonl,
            netx_so.p_ntohl,
            netx_so.p_ntohs,
            netx_so.p_connect,
            netx_so.p_inet_addr,
            netx_so.p_inet_ntoa,
            netx_so.p_socket,
            netx_so.p_setsockopt,
            netx_so.p_getsockopt,
            netx_so.p_getsockname,
            netx_so.p_select,
            netx_so.p_recv,
            netx_so.p_send,
            netx_so.p_shutdown,
            netx_so.p_bind,
            netx_so.p_listen,
            netx_so.p_accept
            );
      mg_dso_unload((DBXPLIB) netx_so.plibrary);
   }
   else {
      netx_so.sock = 1;
   }

   if (netx_so.sock)
      result = 0;
   else
      result = -1;

   netx_so.load_attempted = 1;

   if (netx_so.p_getaddrinfo == NULL ||  netx_so.p_freeaddrinfo == NULL ||  netx_so.p_getnameinfo == NULL)
      netx_so.ipv6 = 0;

netx_load_winsock_no_so:

   if (result == 0) {

      if (netx_so.winsock == 2)
         netx_so.version_requested = MAKEWORD(2, 2);
      else
         netx_so.version_requested = MAKEWORD(1, 1);

      netx_so.wsastartup = NETX_WSASTARTUP(netx_so.version_requested, &(netx_so.wsadata));

      if (netx_so.wsastartup != 0 && netx_so.winsock == 2) {
         netx_so.version_requested = MAKEWORD(2, 0);
         netx_so.wsastartup = NETX_WSASTARTUP(netx_so.version_requested, &(netx_so.wsadata));
         if (netx_so.wsastartup != 0) {
            netx_so.winsock = 1;
            netx_so.version_requested = MAKEWORD(1, 1);
            netx_so.wsastartup = NETX_WSASTARTUP(netx_so.version_requested, &(netx_so.wsadata));
         }
      }
      if (netx_so.wsastartup == 0) {
         if ((netx_so.winsock == 2 && LOBYTE(netx_so.wsadata.wVersion) != 2)
               || (netx_so.winsock == 1 && (LOBYTE(netx_so.wsadata.wVersion) != 1 || HIBYTE(netx_so.wsadata.wVersion) != 1))) {
  
            sprintf(pweb->error, "Initialization Error: Wrong version of Winsock library (%s) (%d.%d)", netx_so.libnam, LOBYTE(netx_so.wsadata.wVersion), HIBYTE(netx_so.wsadata.wVersion));
            NETX_WSACLEANUP();
            netx_so.wsastartup = -1;
         }
         else { /* v2.1.13 */
            if (strlen(netx_so.libnam))
               sprintf(mg_system.info, "Initialization: Windows Sockets library loaded (%s) Version: %d.%d", netx_so.libnam, LOBYTE(netx_so.wsadata.wVersion), HIBYTE(netx_so.wsadata.wVersion));
            else
               sprintf(mg_system.info, "Initialization: Windows Sockets library Version: %d.%d", LOBYTE(netx_so.wsadata.wVersion), HIBYTE(netx_so.wsadata.wVersion));
            netx_so.winsock_ready = 1;
         }
      }
      else {
         strcpy(pweb->error, "Initialization Error: Unusable Winsock library");
      }
   }

   return result;

#else

   return 1;

#endif /* #if defined(_WIN32) */

}


int netx_tcp_connect(MGWEB *pweb, int context)
{
   short physical_ip, ipv6, connected, getaddrinfo_ok;
   int rc, n, errorno;
   unsigned long inetaddr;
   DWORD spin_count;
   struct sockaddr_in srv_addr, cli_addr;
   struct hostent *hp;
   struct in_addr **pptr;
   DBXCON *pcon;

   pcon = pweb->pcon;

   pcon->net_connection = 0;
   pweb->error_no = 0;
   connected = 0;
   getaddrinfo_ok = 0;
   spin_count = 0;

   ipv6 = 1;
#if !defined(NETX_IPV6)
   ipv6 = 0;
#endif

#if defined(_WIN32)

   if (!netx_so.load_attempted) {
      n = netx_load_winsock(pweb, 0);
      if (n != 0) {
         return CACHE_NOCON;
      }
   }
   if (!netx_so.winsock_ready) {
      strcpy(pweb->error, (char *) "DLL Load Error: Unusable Winsock Library");
      return CACHE_NOCON;
   }

   n = netx_so.wsastartup;
   if (n != 0) {
      strcpy(pweb->error, (char *) "DLL Load Error: Unusable Winsock Library");
      return n;
   }

#endif /* #if defined(_WIN32) */

   if (pweb->psrv->ptls) {
      pweb->error[0] = '\0';
      rc = mgtls_crypt_load_library(pweb);
      if (rc != CACHE_SUCCESS) {
         return rc;
      }
      rc = mgtls_tls_load_library(pweb);
      if (rc != CACHE_SUCCESS) {
         return rc;
      }
   }

#if defined(NETX_IPV6)

   if (ipv6) {
      short mode;
      struct addrinfo hints, *res;
      struct addrinfo *ai;
      char port_str[32];

      res = NULL;
      sprintf(port_str, "%d", pcon->psrv->port);
      connected = 0;
      pweb->error_no = 0;

      for (mode = 0; mode < 3; mode ++) {

         if (res) {
            NETX_FREEADDRINFO(res);
            res = NULL;
         }

         memset(&hints, 0, sizeof hints);
         hints.ai_family = AF_UNSPEC;     /* Use IPv4 or IPv6 */
         hints.ai_socktype = SOCK_STREAM;
         /* hints.ai_flags = AI_PASSIVE; */
         if (mode == 0)
            hints.ai_flags = AI_NUMERICHOST | AI_CANONNAME;
         else if (mode == 1)
            hints.ai_flags = AI_CANONNAME;
         else if (mode == 2) {
            /* Apparently an error can occur with AF_UNSPEC (See RJW1564) */
            /* This iteration will return IPV6 addresses if any */
            hints.ai_flags = AI_CANONNAME;
            hints.ai_family = AF_INET6;
         }
         else
            break;

         n = NETX_GETADDRINFO(pcon->psrv->ip_address, port_str, &hints, &res);

         if (n != 0) {
            continue;
         }

         getaddrinfo_ok = 1;
         spin_count = 0;
         for (ai = res; ai != NULL; ai = ai->ai_next) {

            spin_count ++;

	         if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6) {
               continue;
            }

	         /* Open a socket with the correct address family for this address. */
	         pcon->cli_socket = NETX_SOCKET(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

            /* NETX_BIND(pcon->cli_socket, ai->ai_addr, (int) (ai->ai_addrlen)); */
            /* NETX_CONNECT(pcon->cli_socket, ai->ai_addr, (int) (ai->ai_addrlen)); */

            if (netx_so.nagle_algorithm == 0) {

               int flag = 1;
               int result;

               result = NETX_SETSOCKOPT(pcon->cli_socket, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));

               if (result < 0) {
                  strcpy(pweb->error, "Connection Error: Unable to disable the Nagle Algorithm");
               }

            }

            pweb->error_no = 0;
            n = netx_tcp_connect_ex(pweb, (xLPSOCKADDR) ai->ai_addr, (socklen_netx) (ai->ai_addrlen), pcon->timeout);
            if (n == -2) {
               pweb->error_no = n;
               n = -737;
               continue;
            }
            if (SOCK_ERROR(n)) {
               errorno = (int) netx_get_last_error(0);
               pweb->error_no = errorno;
               netx_tcp_disconnect(pweb, 0);
               continue;
            }
            else {
               connected = 1;
               break;
            }
         }
         if (connected)
            break;
      }

      if (pweb->error_no) {
         char message[256];
         netx_get_error_message(pweb->error_no, message, 250, 0);
         sprintf(pweb->error, "Connection Error: Cannot Connect to DB Server (%s:%d): Error Code: %d (%s)", (char *) pcon->psrv->ip_address, pcon->psrv->port, pweb->error_no, message);
         n = -5;
      }

      if (res) {
         NETX_FREEADDRINFO(res);
         res = NULL;
      }
   }
#endif

   if (ipv6) {
      if (connected) {
         pcon->net_connection = 1;
         pcon->closed = 0;
         if (pweb->psrv->ptls) {
            n = mgtls_open_session(pweb);
            if (n != CACHE_SUCCESS) {
               netx_tcp_disconnect(pweb, 0);
               return -5;
            }
         }
         return CACHE_SUCCESS;
      }
      else {
         if (getaddrinfo_ok) {
            netx_tcp_disconnect(pweb, 0);
            return -5;
         }
         else {
            char message[256];

            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "Connection Error: Cannot identify DB Server (%s:%d): Error Code: %d (%s)", (char *) pcon->psrv->ip_address, pcon->psrv->port, errorno, message);
            netx_tcp_disconnect(pweb, 0);
            return -5;
         }
      }
   }

   ipv6 = 0;
   inetaddr = NETX_INET_ADDR(pcon->psrv->ip_address);

   physical_ip = 0;
   if (isdigit(pcon->psrv->ip_address[0])) {
      char *p;

      p = strstr(pcon->psrv->ip_address, ".");
      if (p) {
         if (isdigit(*(++ p))) {
            p = strstr(p, ".");
            if (p) {
               if (isdigit(*(++ p))) {
                  p = strstr(p, ".");
                  if (p) {
                     if (isdigit(*(++ p))) {
                        physical_ip = 1;
                     }
                  }
               }
            }
         }
      }
   }

   if (inetaddr == INADDR_NONE || !physical_ip) {

      hp = NETX_GETHOSTBYNAME((const char *) pcon->psrv->ip_address);

      if (hp == NULL) {
         n = -2;
         strcpy(pweb->error, "Connection Error: Invalid Host");
         return n;
      }

      pptr = (struct in_addr **) hp->h_addr_list;
      connected = 0;

      spin_count = 0;

      for (; *pptr != NULL; pptr ++) {

         spin_count ++;

         pcon->cli_socket = NETX_SOCKET(AF_INET, SOCK_STREAM, 0);

         if (INVALID_SOCK(pcon->cli_socket)) {
            char message[256];

            n = -2;
            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "Connection Error: Invalid Socket: Context=1: Error Code: %d (%s)", errorno, message);
            break;
         }

#if !defined(_WIN32)
         BZERO((char *) &cli_addr, sizeof(cli_addr));
         BZERO((char *) &srv_addr, sizeof(srv_addr));
#endif

         cli_addr.sin_family = AF_INET;
         srv_addr.sin_port = NETX_HTONS((unsigned short) pcon->psrv->port);

         cli_addr.sin_addr.s_addr = NETX_HTONL(INADDR_ANY);
         cli_addr.sin_port = NETX_HTONS(0);

         n = NETX_BIND(pcon->cli_socket, (xLPSOCKADDR) &cli_addr, sizeof(cli_addr));

         if (SOCK_ERROR(n)) {
            char message[256];

            n = -3;
            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "Connection Error: Cannot bind to Socket: Error Code: %d (%s)", errorno, message);

            break;
         }

         if (netx_so.nagle_algorithm == 0) {

            int flag = 1;
            int result;

            result = NETX_SETSOCKOPT(pcon->cli_socket, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));
            if (result < 0) {
               strcpy(pweb->error, "Connection Error: Unable to disable the Nagle Algorithm");
            }
         }

         srv_addr.sin_family = AF_INET;
         srv_addr.sin_port = NETX_HTONS((unsigned short) pcon->psrv->port);

         NETX_MEMCPY(&srv_addr.sin_addr, *pptr, sizeof(struct in_addr));

         n = netx_tcp_connect_ex(pweb, (xLPSOCKADDR) &srv_addr, sizeof(srv_addr), pcon->timeout);

         if (n == -2) {
            pweb->error_no = n;
            n = -737;

            continue;
         }

         if (SOCK_ERROR(n)) {
            char message[256];

            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);

            pweb->error_no = errorno;
            sprintf(pweb->error, "Connection Error: Cannot Connect to DB Server (%s:%d): Error Code: %d (%s)", (char *) pcon->psrv->ip_address, pcon->psrv->port, errorno, message);
            n = -5;
            netx_tcp_disconnect(pweb, 0);
            continue;
         }
         else {
            connected = 1;
            break;
         }
      }
      if (!connected) {

         netx_tcp_disconnect(pweb, 0);

         strcpy(pweb->error, "Connection Error: Failed to find the DB Server via a DNS Lookup");

         return n;
      }
   }
   else {

      pcon->cli_socket = NETX_SOCKET(AF_INET, SOCK_STREAM, 0);

      if (INVALID_SOCK(pcon->cli_socket)) {
         char message[256];

         n = -2;
         errorno = (int) netx_get_last_error(0);
         netx_get_error_message(errorno, message, 250, 0);
         sprintf(pweb->error, "Connection Error: Invalid Socket: Context=2: Error Code: %d (%s)", errorno, message);

         return n;
      }

#if !defined(_WIN32)
      BZERO((char *) &cli_addr, sizeof(cli_addr));
      BZERO((char *) &srv_addr, sizeof(srv_addr));
#endif

      cli_addr.sin_family = AF_INET;
      cli_addr.sin_addr.s_addr = NETX_HTONL(INADDR_ANY);
      cli_addr.sin_port = NETX_HTONS(0);

      n = NETX_BIND(pcon->cli_socket, (xLPSOCKADDR) &cli_addr, sizeof(cli_addr));

      if (SOCK_ERROR(n)) {
         char message[256];

         n = -3;

         errorno = (int) netx_get_last_error(0);
         netx_get_error_message(errorno, message, 250, 0);

         sprintf(pweb->error, "Connection Error: Cannot bind to Socket: Error Code: %d (%s)", errorno, message);

         netx_tcp_disconnect(pweb, 0);

         return n;
      }

      if (netx_so.nagle_algorithm == 0) {

         int flag = 1;
         int result;

         result = NETX_SETSOCKOPT(pcon->cli_socket, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));

         if (result < 0) {
            strcpy(pweb->error, "Connection Error: Unable to disable the Nagle Algorithm");

         }
      }

      srv_addr.sin_port = NETX_HTONS((unsigned short) pcon->psrv->port);
      srv_addr.sin_family = AF_INET;
      srv_addr.sin_addr.s_addr = NETX_INET_ADDR(pcon->psrv->ip_address);

      n = netx_tcp_connect_ex(pweb, (xLPSOCKADDR) &srv_addr, sizeof(srv_addr), pcon->timeout);
      if (n == -2) {
         pweb->error_no = n;
         n = -737;

         netx_tcp_disconnect(pweb, 0);

         return n;
      }

      if (SOCK_ERROR(n)) {
         char message[256];

         errorno = (int) netx_get_last_error(0);
         netx_get_error_message(errorno, message, 250, 0);
         pweb->error_no = errorno;
         sprintf(pweb->error, "Connection Error: Cannot Connect to DB Server (%s:%d): Error Code: %d (%s)", (char *) pcon->psrv->ip_address, pcon->psrv->port, errorno, message);
         n = -5;
         netx_tcp_disconnect(pweb, 0);
         return n;
      }
   }

   pcon->net_connection = 1;
   pcon->closed = 0;

   return CACHE_SUCCESS;
}


int netx_tcp_handshake(MGWEB *pweb, int context)
{
   int len, len1;
   char buffer[2560];
   DBXCON *pcon;

   pcon = pweb->pcon;

   sprintf(buffer, "dbx1~%s~%d\n", pcon->psrv->uci ? pcon->psrv->uci : "", pcon->psrv->idle_timeout);
   len = (int) strlen(buffer);

   netx_tcp_write(pweb, (unsigned char *) buffer, len);
/* cmtjs
   len = netx_tcp_read(pweb, (unsigned char *) buffer, 500, 10, 0);

   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_handshake: len=%d", len);
      mg_log_event(pweb->plog, pweb, bufferx, "netx_tcp_handshake", -1);
      mg_log_event(pweb->plog, pweb, buffer, "netx_tcp_handshake - data", -1);
   }

*/

   len = netx_tcp_read(pweb, (unsigned char *) buffer, 5, 10, 1); /* v2.1.13 */
   if (len != 5) {
      return CACHE_NOCON;
   }

   len = mg_get_size((unsigned char *) buffer);
/*
   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_handshake: len=%d", len);
      mg_log_event(pweb->plog, pweb, bufferx, "netx_tcp_handshake", -1);
   }
*/
   len1 = netx_tcp_read(pweb, (unsigned char *) buffer, len, 10, 1); /* v2.1.13 */
   if (len1 != len) {
      return CACHE_NOCON;
   }


   if (pcon->psrv->dbtype == DBX_DBTYPE_IRIS || pcon->psrv->dbtype == DBX_DBTYPE_CACHE) {
      isc_parse_zv(buffer, pcon->p_zv);
   }
   else if (pcon->psrv->dbtype == DBX_DBTYPE_YOTTADB || pcon->psrv->dbtype == DBX_DBTYPE_GTM) {
      ydb_parse_zv(buffer, pcon->p_zv);
   }
   else { /* v2.5.31 */
      char *p;

      pcon->p_zv->mg_version = 0;
      pcon->p_zv->majorversion = 0;
      pcon->p_zv->minorversion = 0;
      pcon->p_zv->mg_build = 0;
      pcon->p_zv->vnumber = 0;
      p = strstr(buffer, "v");
      if (p) {
         pcon->p_zv->majorversion = (int) strtol(++ p, NULL, 10);
         pcon->p_zv->mg_version = strtod(p, NULL); 
         p = strstr(p, ".");
         if (p) {
            pcon->p_zv->minorversion = (int) strtol(++ p, NULL, 10);
            p = strstr(p, ".");
            if (p) {
               pcon->p_zv->mg_build = (int) strtol(++ p, NULL, 10);
            }
         }
      }
/*
      sprintf(buffer, "JavaScript engine: version=%f; major=%d; minor=%d; build=%d", pcon->p_zv->mg_version, pcon->p_zv->majorversion, pcon->p_zv->minorversion, pcon->p_zv->mg_build);
      mg_log_event(pweb->plog, pweb, buffer, "DB Server Version", -1);
*/
   }

   return 0;
}


int netx_tcp_ping(MGWEB *pweb, int context)
{
   int rc;
   unsigned char netbuf[32];
   unsigned long netbuf_used;
   DBXCON *pcon;

   pcon = pweb->pcon;

   netbuf_used = 5;
   mg_add_block_size((unsigned char *) netbuf, 0, 0, 0, DBX_CMND_PING);
/*
   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_ping SEND netbuf_used=%d;", (int) netbuf_used);
      mg_log_buffer(&(mg_system.log), pweb, (char *) netbuf, 5, bufferx, 0);
   }
*/
   rc = netx_tcp_write(pweb, (unsigned char *) netbuf, netbuf_used);
   if (rc < 0) {
      return -1;
   }
   rc = netx_tcp_read(pweb, (unsigned char *) netbuf, 20, pcon->timeout, 1);
/*
   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_ping RECV rc=%d; pcon->closed=%d;", rc, pcon->closed);
      mg_log_buffer(&(mg_system.log), pweb, (char *) netbuf, rc, bufferx, 0);
   }
*/
   if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
      return rc;
   }
   if (rc < 0) {
      return rc;
   }

   return 0;
}


/*

 i buf="xDBC" g main^%mgsqln
 i buf?1u.e1"HTTP/"1n1"."1n1c s buf=buf_$c(10) g main^%mgsqlw
 i $e(buf,1,4)="dbx1" g dbx^%zmgsis

dbx ; new wire protocol for access to M
 s res=$zv
 s res=$$esize256($l(res))_"0"_res
 w res,*-3
dbx1 ; test
 r head#4
 s len=$$esize256(head)
 s ^%cm($i(^%cm))=head
 r data#len
 s ^%cm($i(^%cm))=data
 s res="1"
 s res=$$esize256($l(res))_"0"_res
 w res,*-3
 q
 ;

*/


int netx_tcp_command(MGWEB *pweb, int command, int context)
{
   DBX_TRACE_INIT(0)
   int get, rc, offset, reconnect;
   unsigned int netbuf_used;
   unsigned char *netbuf;
   DBXCON *pcon;

#ifdef _WIN32
__try {
#endif

   pcon = pweb->pcon;

   rc = CACHE_SUCCESS;
   pweb->error[0] = '\0';
   offset = 10;

   netbuf = (unsigned char *) (pweb->input_buf.buf_addr - DBX_IBUFFER_OFFSET);
   netbuf_used = (pweb->input_buf.len_used + DBX_IBUFFER_OFFSET);

/*
   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_command SEND cmnd=%d; size=%d; netbuf_used=%d; requestno_in=%d; requestno_out=%d; pcon->closed=%d;", command, pweb->input_buf.len_used, netbuf_used, pweb->requestno_in, pweb->requestno_out, pcon->closed);
      mg_log_buffer(&(mg_system.log), pweb, netbuf, netbuf_used, bufferx, 0);
   }
*/

   reconnect = 0;

netx_tcp_command_reconnect:

   rc = netx_tcp_write(pweb, (unsigned char *) netbuf, netbuf_used);

   if (rc < 0) {
      if (reconnect) {
         return rc;
      }
      else {
         rc = mg_connect(pweb, 1);
         if (rc < 0) {
            return rc;
         }
         reconnect = 1;
         goto netx_tcp_command_reconnect;
      }
   }

   if (pweb->request_long) { /* v2.2.18 */

      rc = mg_execute_request_long(pweb, mg_write_chunk_tcp);

      if (rc < 0) {
         return rc;
      }
      mg_add_block_size((unsigned char *) netbuf, 0, 0, DBX_DSORT_EOD, DBX_DTYPE_STR8);
      rc = netx_tcp_write(pweb, (unsigned char *) netbuf, 5);
      if (rc < 0) {
         return rc;
      }
   }

/* read to timeout */
/*
{
   int len, nreads;
   char buffer[256];

   offset = 0;
   len = 0;
   nreads = 0;
   for (;;) {
      rc = netx_tcp_read(pcon, (unsigned char *) pweb->output_val.svalue.buf_addr + offset, 2048, 10, 1);
      if (rc == NETX_READ_TIMEOUT) {
         mg_log_event(&(mg_system.log), pweb, "Timeout", "*** TCP READ BUFFER ***", 0);
         break;
      }
      if (rc > 0) {
         offset += rc;
         len += rc;
         nreads ++;
      }
   }
   pweb->output_val.svalue.len_used = len;
   sprintf(buffer, "*** TCP READ BUFFER *** len_used=%d; nreads=%d;", pweb->output_val.svalue.len_used, nreads);
   mg_log_buffer(&(mg_system.log), pweb, pweb->output_val.svalue.buf_addr, pweb->output_val.svalue.len_used, buffer, 0);
   return CACHE_SUCCESS;
}
*/

   pweb->failover_possible = 0; /* can't failover after this point */
   pweb->output_val.svalue.len_used = 0;

   rc = netx_tcp_read(pweb, (unsigned char *) pweb->output_val.svalue.buf_addr, offset, pcon->timeout, 1);
/*
   {
      char bufferx[256];
      sprintf(bufferx, "netx_tcp_command RECV rc=%d; pcon->closed=%d;", rc, pcon->closed);
      mg_log_buffer(&(mg_system.log), pweb, netbuf, netbuf_used, bufferx, 0);
   }
*/
   if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
      return rc;
   }
   if (rc < 0) {
      if (reconnect) {
         return rc;
      }
      else {
         rc = mg_connect(pweb, 1);
         if (rc < 0) {
            return rc;
         }
         reconnect = 1;
         goto netx_tcp_command_reconnect;
      }
   }

   pweb->output_val.svalue.buf_addr[offset] = '\0';

   pweb->response_size = mg_get_block_size((unsigned char *) pweb->output_val.svalue.buf_addr, 0, &(pweb->output_val.sort), &(pweb->output_val.type));
   pweb->requestno_out = mg_get_size((unsigned char *) (pweb->output_val.svalue.buf_addr + 5));
   pweb->output_val.svalue.len_used += offset;

   if (pweb->response_size == 0 && pweb->output_val.svalue.buf_addr[9] == '\x01') {
      pweb->response_streamed = 1;
      pweb->response_size = 5;
      return netx_tcp_read_stream(pweb);
   }
   else if (pweb->response_size == 0 && pweb->output_val.svalue.buf_addr[9] == '\x02') {
      pweb->response_streamed = 2;
      pweb->response_size = 5;
      pcon->stream_tail_len = 0;
      return netx_tcp_read_stream(pweb);
   }

   MG_LOG_RESPONSE_FRAME(pweb, pweb->output_val.svalue.buf_addr, pweb->response_size);

/*
   {
      char buffer[256];
      sprintf(buffer, "rc=%d; sort=%d; type=%d; response_size=%d; buffer_size=%d; get=%d;", rc, pweb->output_val.sort, pweb->output_val.type, pweb->response_size, pweb->output_val.svalue.len_alloc, (pweb->output_val.svalue.len_alloc - DBX_HEADER_SIZE));
      mg_log_event(pweb->plog, pweb, buffer, "netx_tcp_read", 0);
   }
*/
   get = 0;
   if (pweb->response_size > 0) {
      get = pweb->response_size - 5; /* first 5 Bytes already read */
      if (pweb->response_size > (unsigned int) (pweb->output_val.svalue.len_alloc - DBX_HEADER_SIZE)) {
         get = (pweb->output_val.svalue.len_alloc - DBX_HEADER_SIZE);
         pweb->response_remaining = (pweb->response_size - get);
      }
      netx_tcp_read(pweb, (unsigned char *) pweb->output_val.svalue.buf_addr + offset, get, pcon->timeout, 1);
   }

   if (pweb->output_val.type == DBX_DTYPE_OREF) {
      pweb->output_val.svalue.buf_addr[offset + get] = '\0';
      pweb->output_val.num.oref = (unsigned int) strtol(pweb->output_val.svalue.buf_addr + offset, NULL, 10);
      pweb->output_val.num.int32 = pweb->output_val.num.oref;
   }

/*
   {
      char buffer[256];
      sprintf(buffer, "netx_tcp_command RECV cmnd=%d; len=%d; sort=%d; type=%d; oref=%d; rc=%d; error=%s;", command, len, pcon->output_val.sort, pcon->output_val.type, pcon->output_val.num.oref, rc, pweb->error);
      mg_buffer_dump(pcon, pcon->output_val.svalue.buf_addr, len, buffer, 8, 0);
   }
*/
   pweb->output_val.svalue.len_used += get;
   if (rc > 0) {
      rc = CACHE_SUCCESS;
   }
   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:netx_tcp_command: %x:%d", code, DBX_TRACE_VAR);
      strcpy(mg_system.config_error, bufferx);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int netx_tcp_read_stream(MGWEB *pweb)
{
   DBX_TRACE_INIT(0)
   int rc, get, eos;
   DBXVAL *pval;
   DBXCON *pcon;

#ifdef _WIN32
__try {
#endif

   pcon = pweb->pcon;

   rc = CACHE_SUCCESS;
   pval = &(pweb->output_val);

   if (pweb->response_streamed == 2) { /* ASCII stream mode */

      pweb->response_remaining = 0;
      memset((void *) pweb->db_chunk_head, 0, sizeof(pweb->db_chunk_head));

      for (;;) {
         if (pcon->stream_tail_len > 0) {
            memcpy((void *) (pval->svalue.buf_addr + pval->svalue.len_used), (void *) pcon->stream_tail, pcon->stream_tail_len);
            pval->svalue.len_used += pcon->stream_tail_len;
            pcon->stream_tail_len = 0;
         }

         eos = 0;
         get = (pval->svalue.len_alloc - (pval->svalue.len_used + DBX_HEADER_SIZE));
         while (get) {
            rc = netx_tcp_read(pweb, (unsigned char *) pval->svalue.buf_addr + pval->svalue.len_used, get, pcon->timeout, 0);
/*
            {
               char bufferx[256];
               sprintf(bufferx, "ASCII stream response from DB Server: rc=%d; len_used=%d; get=%d;", rc, pval->svalue.len_used, get);
               mg_log_buffer(pweb->plog, pweb, pval->svalue.buf_addr, pval->svalue.len_used + (rc > 0 ? rc : 0), bufferx, 0);
            }
*/
            if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
               break;
            }
            else if (rc == 0) {
               rc = NETX_READ_EOF;
               break;
            }

            get -= rc;
            pval->svalue.len_used += rc;
            pweb->response_size += rc;
/*
            {
               char bufferx[256];
               sprintf(bufferx, "ASCII stream response from DB Server (last 4 Bytes): len_used=%d; %x %x %x %x;", pval->svalue.len_used, (unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 1], (unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 2], (unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 3], (unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 4]);
               mg_log_buffer(pweb->plog, pweb, pval->svalue.buf_addr, pval->svalue.len_used, bufferx, 0);
            }
*/
            if (pval->svalue.len_used > 4) {
               if (((unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 1]) == 0xff && ((unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 2]) == 0xff && ((unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 3]) == 0xff && ((unsigned char) pval->svalue.buf_addr[pval->svalue.len_used - 4]) == 0xff) {
                  eos = 1;
                  break;
               }
            }
         }

         MG_LOG_RESPONSE_BUFFER(pweb, pweb->db_chunk_head, pval->svalue.buf_addr, pval->svalue.len_used);

         if (pval->svalue.len_used > 4) {
            if (eos) {
               pweb->response_remaining = 0;
               pval->svalue.len_used -= 4;
               pweb->response_size -= 4;
            }
            else {
               pcon->stream_tail_len = 4;
               memcpy((void *) pcon->stream_tail, (void *) (pval->svalue.buf_addr + (pval->svalue.len_used - 4)), pcon->stream_tail_len);
               pval->svalue.len_used -= 4;
               pweb->response_remaining = 4;
            }
            rc = CACHE_SUCCESS;
         }

         if (eos) {
            rc = CACHE_SUCCESS;
            break;
         }
         else {
            if (mg_system.chunking && ((pweb->response_size > mg_system.chunking) || pweb->response_chunked)) {
               /* Can't read the whole response, so start (or continue) chunking assuming chunking is allowed */
               pweb->response_chunked = 1;
               rc = CACHE_SUCCESS;
               break;
            }
            else {
               if (pweb->response_size > DBX_MAXCLEN) { /* v2.8.38 */
                  /* Can't read the whole response to generate a 'Content-Length', so revert to streaming with a 'Connection: close' */
                  pweb->response_maxclen = 1;
                  rc = CACHE_SUCCESS;
                  break;
               }
               else {
                  /* 2.0.8 */
                  pval = mg_extend_response_memory(pweb);
                  if (!pval) {
                     rc = NETX_READ_ERROR;
                     break;
                  }
               }
            }
         }
      }

      return rc;
   }

   if (pweb->response_remaining == 0) {
      rc = netx_tcp_read(pweb, (unsigned char *) pweb->db_chunk_head, 4, pcon->timeout, 1);
      if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
         return rc;
      }
      if (pweb->db_chunk_head[0] == 0xff && pweb->db_chunk_head[1] == 0xff && pweb->db_chunk_head[2] == 0xff && pweb->db_chunk_head[3] == 0xff) {
         pweb->response_remaining = 0;
         MG_LOG_RESPONSE_FRAME(pweb, pweb->db_chunk_head, pweb->response_remaining);
         return CACHE_SUCCESS;
      }
      pweb->response_remaining = mg_get_size((unsigned char *) pweb->db_chunk_head);
      MG_LOG_RESPONSE_FRAME(pweb, pweb->db_chunk_head, pweb->response_remaining);
   }

   if (pweb->response_remaining == 0) {
      return NETX_READ_EOF;
   }

   for (;;) {
      rc = netx_tcp_read(pweb, (unsigned char *) pval->svalue.buf_addr + pval->svalue.len_used, pweb->response_remaining, pcon->timeout, 1);
/*
      {
         char bufferx[256];
         sprintf(bufferx, "Chunked response from DB Server: rc=%d; pweb->response_remaining=%d;", rc, pweb->response_remaining);
         mg_log_event(pweb->plog, pweb, bufferx, "mg_web: Read response", 0);
      }
*/

      if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
         break;
      }

      MG_LOG_RESPONSE_BUFFER(pweb, pweb->db_chunk_head, (pval->svalue.buf_addr + pval->svalue.len_used), pweb->response_remaining);

      pval->svalue.len_used += pweb->response_remaining;
      pweb->response_size += pweb->response_remaining;

      rc = netx_tcp_read(pweb, (unsigned char *) pweb->db_chunk_head, 4, pcon->timeout, 1);
      if (rc == NETX_READ_TIMEOUT || rc == NETX_READ_EOF) {
         break;
      }

      if (pweb->db_chunk_head[0] == 0xff && pweb->db_chunk_head[1] == 0xff && pweb->db_chunk_head[2] == 0xff && pweb->db_chunk_head[3] == 0xff) {
         pweb->response_remaining = 0;
         MG_LOG_RESPONSE_FRAME(pweb, pweb->db_chunk_head, pweb->response_remaining);
         rc = CACHE_SUCCESS;
         break;
      }
      pweb->response_remaining = mg_get_size((unsigned char *) pweb->db_chunk_head);
      MG_LOG_RESPONSE_FRAME(pweb, pweb->db_chunk_head, pweb->response_remaining);
      /* if ((pval->svalue.len_used + pweb->response_size) > (unsigned int) (pval->svalue.len_alloc - DBX_HEADER_SIZE)) { */
      if ((pval->svalue.len_used + pweb->response_remaining) > (unsigned int) (pval->svalue.len_alloc - DBX_HEADER_SIZE)) { /* v2.1.14 */
/*
         {
            char bufferx[256];
            sprintf(bufferx, "Possibly Chunked response from DB Server: pweb->response_size=%lu; pweb->response_remaining=%lu; pval->svalue.len_used=%lu; pval->svalue.len_alloc=%lu; mg_system.chunking=%lu;", pweb->response_size, pweb->response_remaining, pval->svalue.len_used, pval->svalue.len_alloc, mg_system.chunking);
            mg_log_event(pweb->plog, pweb, bufferx, "test", 0);
         }
*/
         if (mg_system.chunking && ((pweb->response_size > mg_system.chunking) || pweb->response_chunked)) {
            /* Can't read the whole response, so start (or continue) chunking assuming chunking is allowed */
            pweb->response_chunked = 1;
            rc = CACHE_SUCCESS;
            break;
         }
         else {
            if (pweb->response_size > DBX_MAXCLEN) { /* v2.8.38 */
               /* Can't read the whole response to generate a 'Content-Length', so revert to streaming with a 'Connection: close' */
               pweb->response_maxclen = 1;
               rc = CACHE_SUCCESS;
               break;
            }
            else {
               /* 2.0.8 */
               pval = mg_extend_response_memory(pweb);
               if (!pval) {
                  rc = NETX_READ_ERROR;
                  break;
               }
            }
         }
      }
   }
   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:netx_tcp_read_stream: %x:%d", code, DBX_TRACE_VAR);
      strcpy(mg_system.config_error, bufferx);
      mg_log_event(&(mg_system.log), NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

   return CACHE_FAILURE;
}
#endif
}


int netx_tcp_connect_ex(MGWEB *pweb, xLPSOCKADDR p_srv_addr, socklen_netx srv_addr_len, int timeout)
{
#if defined(_WIN32)
   int n;
#else
   int flags, n, error;
   socklen_netx len;
   fd_set rset, wset;
   struct timeval tval;
#endif
   DBXCON *pcon;

   pcon = pweb->pcon;

#if defined(SOLARIS) && BIT64PLAT
   timeout = 0;
#endif

   /* It seems that BIT64PLAT is set to 0 for 64-bit Solaris:  So, to be safe .... */

#if defined(SOLARIS)
   timeout = 0;
#endif

   if (timeout != 0) {

#if defined(_WIN32)

      n = NETX_CONNECT(pcon->cli_socket, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);
      return n;

#else
      flags = fcntl(pcon->cli_socket, F_GETFL, 0);
      n = fcntl(pcon->cli_socket, F_SETFL, flags | O_NONBLOCK);

      error = 0;

      n = NETX_CONNECT(pcon->cli_socket, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);

      if (n < 0) {

         if (errno != EINPROGRESS) {
#if defined(SOLARIS)
            if (errno != 2 && errno != 146) {
               sprintf((char *) pweb->error, "Diagnostic: Solaris: Initial Connection Error errno=%d; EINPROGRESS=%d", errno, EINPROGRESS);
               return -1;
            }
#else
            return -1;
#endif
         }
      }

      if (n != 0) {

         FD_ZERO(&rset);
         FD_SET(pcon->cli_socket, &rset);

         wset = rset;
         tval.tv_sec = timeout;
         tval.tv_usec = timeout;

         n = NETX_SELECT((int) (pcon->cli_socket + 1), &rset, &wset, NULL, &tval);

         if (n == 0) {
            close(pcon->cli_socket);
            errno = ETIMEDOUT;

            return (-2);
         }
         if (NETX_FD_ISSET(pcon->cli_socket, &rset) || NETX_FD_ISSET(pcon->cli_socket, &wset)) {

            len = sizeof(error);
            if (NETX_GETSOCKOPT(pcon->cli_socket, SOL_SOCKET, SO_ERROR, (void *) &error, &len) < 0) {

               sprintf((char *) pweb->error, "Diagnostic: Pending Error %d", errno);

               return (-1);   /* Solaris pending error */
            }
         }
         else {
            ;
         }
      }

      fcntl(pcon->cli_socket, F_SETFL, flags);      /* Restore file status flags */

      if (error) {

         close(pcon->cli_socket);
         errno = error;
         return (-1);
      }

      return 1;

#endif

   }
   else {

      n = NETX_CONNECT(pcon->cli_socket, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);

      return n;
   }

}


int netx_tcp_disconnect(MGWEB *pweb, int context)
{
   DBXCON *pcon;

   pcon = pweb->pcon;

   if (!pcon) {
      return 0;
   }

   if (pcon->cli_socket != (SOCKET) 0) {

      if (pweb->psrv->ptls && pcon->ptlscon) {
         mgtls_close_session(pweb);
      }

#if defined(_WIN32)
      NETX_CLOSESOCKET(pcon->cli_socket);
/*
      NETX_WSACLEANUP();
*/

#else
      close(pcon->cli_socket);
#endif

   }

   pcon->closed = 1;

   return 0;

}


int netx_tcp_write(MGWEB *pweb, unsigned char *data, int size)
{
   int n, rc, errorno, total;
   DBXCON *pcon;

   pcon = pweb->pcon;

   if (pcon->closed == 1) {
      strcpy(pweb->error, "TCP Write Error: Socket is Closed");
      return -1;
   }

   rc = 0;
   total = 0;
   for (;;) {
      if (pcon->ptlscon) {
         n = mgtls_send(pweb, (unsigned char *) (data + total), size - total);
      }
      else {
         n = NETX_SEND(pcon->cli_socket, (xLPSENDBUF) (data + total), size - total, 0);
      }

      if (SOCK_ERROR(n)) {
         errorno = (int) netx_get_last_error(0);
         if (NOT_BLOCKING(errorno) && errorno != 0) {
            char message[256];

            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "TCP Write Error: Cannot Write Data: Error Code: %d (%s)", errorno, message);
            rc = -1;
            break;
         }
      }
      else {
         total += n;
         if (total == size) {
            break;
         }
      }
   }

   if (rc < 0)
      return rc;
   else
      return size;
}


int netx_tcp_read(MGWEB *pweb, unsigned char *data, int size, int timeout, int context)
{
   int result, n, max_fd;
   int len;
   fd_set rset, eset;
   struct timeval tval;
   unsigned long spin_count;
   DBXCON *pcon;

   pcon = pweb->pcon;

   if (!pcon) {
      return NETX_READ_ERROR;
   }

   result = 0;

   tval.tv_sec = timeout;
   tval.tv_usec = 0;

   spin_count = 0;
   len = 0;
   for (;;) {
      spin_count ++;

      if (pcon->ptlscon) {
         if (timeout != pcon->current_timeout) {
            NETX_SETSOCKOPT(pcon->cli_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tval, sizeof(tval));
            pcon->current_timeout = timeout;
         }

         n = mgtls_recv(pweb, (unsigned char *) (data + len), size - len);
      }
      else {
         FD_ZERO(&rset);
         FD_ZERO(&eset);
         FD_SET(pcon->cli_socket, &rset);
         FD_SET(pcon->cli_socket, &eset);

#if defined(_WIN32)
         max_fd = (int) (pcon->cli_socket + 1);
         n = NETX_SELECT((int) max_fd, &rset, NULL, &eset, &tval);
#else
         if (pcon->int_pipe[0] > 0) {
            FD_SET(pcon->int_pipe[0], &rset);
         }
         max_fd = ((int) pcon->cli_socket) > pcon->int_pipe[0] ? ((int) pcon->cli_socket) : pcon->int_pipe[0];
         n = NETX_SELECT(max_fd + 1, &rset, NULL, &eset, &tval);
         if (n > 0 && pcon->int_pipe[0] > 0 && NETX_FD_ISSET(pcon->int_pipe[0], &rset)) {
            n = read(pcon->int_pipe[0], data, 4);
            if (!strncmp((char *) data, (char *) "exit", 4)) {
               data[0] = '\0';
               result = NETX_READ_ERROR;
               break;
            }
         }
#endif

         if (n == 0) {
            sprintf(pweb->error, "TCP Read Error: DB Server did not respond within the timeout period (%d seconds)", timeout);
            result = NETX_READ_TIMEOUT;
            break;
         }

         if (n < 0 || !NETX_FD_ISSET(pcon->cli_socket, &rset)) {
            int errorno;
            char message[256];

            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "TCP Read Error (on select): DB Server closed the connection unexpectedly: Error Code: %d (%s)", errorno, message);

            result = NETX_READ_ERROR;
            break;
         }

         n = NETX_RECV(pcon->cli_socket, (char *) data + len, size - len, 0);
      }

      if (n < 1) {
         if (n == 0) {
            strcpy(pweb->error, "TCP Read Error (on recv): DB Server closed the connection unexpectedly");
            result = NETX_READ_EOF;
            pcon->closed = 1;
            pcon->eof = 1;
         }
         else {
            int errorno;
            char message[256];

            errorno = (int) netx_get_last_error(0);
            netx_get_error_message(errorno, message, 250, 0);
            sprintf(pweb->error, "TCP Read Error (on recv): DB Server closed the connection unexpectedly: Error Code: %d (%s)", errorno, message);

            result = NETX_READ_ERROR;
            len = 0;
            pcon->closed = 1;
         }
         break;
      }

      len += n;
      if (context) { /* Must read length requested */
         if (len == size) {
            break;
         }
      }
      else {
         break;
      }
   }

   if (len) {
      result = len;
   }
   return result;
}


int netx_get_last_error(int context)
{
   int error_code;

#if defined(_WIN32)
   if (context)
      error_code = (int) GetLastError();
   else
      error_code = (int) NETX_WSAGETLASTERROR();
#else
   error_code = (int) errno;
#endif

   return error_code;
}


int netx_get_error_message(int error_code, char *message, int size, int context)
{
   *message = '\0';

#if defined(_WIN32)

   if (context == 0) {
      short ok;
      int len;
      char *p;
      LPVOID lpMsgBuf;

      ok = 0;
      lpMsgBuf = NULL;
      len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,
                           error_code,
                           /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), */
                           MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                           (LPTSTR) &lpMsgBuf,
                           0,
                           NULL 
                           );
      if (len && lpMsgBuf) {
         strncpy(message, (const char *) lpMsgBuf, size);
         p = strstr(message, "\r\n");
         if (p)
            *p = '\0';
         ok = 1;
      }
      if (lpMsgBuf)
         LocalFree(lpMsgBuf);

      if (!ok) {
         switch (error_code) {
            case EXCEPTION_ACCESS_VIOLATION:
               strncpy(message, "The thread attempted to read from or write to a virtual address for which it does not have the appropriate access.", size);
               break;
            case EXCEPTION_BREAKPOINT:
               strncpy(message, "A breakpoint was encountered.", size); 
               break;
            case EXCEPTION_DATATYPE_MISALIGNMENT:
               strncpy(message, "The thread attempted to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries, 32-bit values on 4-byte boundaries, and so on.", size);
               break;
            case EXCEPTION_SINGLE_STEP:
               strncpy(message, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.", size);
               break;
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
               strncpy(message, "The thread attempted to access an array element that is out of bounds, and the underlying hardware supports bounds checking.", size);
               break;
            case EXCEPTION_FLT_DENORMAL_OPERAND:
               strncpy(message, "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.", size);
               break;
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
               strncpy(message, "The thread attempted to divide a floating-point value by a floating-point divisor of zero.", size);
               break;
            case EXCEPTION_FLT_INEXACT_RESULT:
               strncpy(message, "The result of a floating-point operation cannot be represented exactly as a decimal fraction.", size);
               break;
            case EXCEPTION_FLT_INVALID_OPERATION:
               strncpy(message, "This exception represents any floating-point exception not included in this list.", size);
               break;
            case EXCEPTION_FLT_OVERFLOW:
               strncpy(message, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.", size);
               break;
            case EXCEPTION_FLT_STACK_CHECK:
               strncpy(message, "The stack overflowed or underflowed as the result of a floating-point operation.", size);
               break;
            case EXCEPTION_FLT_UNDERFLOW:
               strncpy(message, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.", size);
               break;
            case EXCEPTION_INT_DIVIDE_BY_ZERO:
               strncpy(message, "The thread attempted to divide an integer value by an integer divisor of zero.", size);
               break;
            case EXCEPTION_INT_OVERFLOW:
               strncpy(message, "The result of an integer operation caused a carry out of the most significant bit of the result.", size);
               break;
            case EXCEPTION_PRIV_INSTRUCTION:
               strncpy(message, "The thread attempted to execute an instruction whose operation is not allowed in the current machine mode.", size);
               break;
            case EXCEPTION_NONCONTINUABLE_EXCEPTION:
               strncpy(message, "The thread attempted to continue execution after a noncontinuable exception occurred.", size);
               break;
            default:
               strncpy(message, "Unrecognised system or hardware error.", size);
            break;
         }
      }
   }

#else

   if (context == 0) {
      strcpy(message, "");
#if 0

#if defined(LINUX) || defined(AIX) || defined(OSF1) || defined(MACOSX)
#if defined(_GNU_SOURCE)
      strerror_r(error_code, message, (size_t) size);
#else
      strerror_r(error_code, message, (size_t) size);
#endif
      size = (int) strlen(message);
#else
      netx_get_std_error_message(error_code, message, size, context);
      size = (int) strlen(message);
#endif

#else

      netx_get_std_error_message(error_code, message, size, context);
      size = (int) strlen(message);

#endif
   }

#endif

   message[size - 1] = '\0';

   return (int) strlen(message);
}


int netx_get_std_error_message(int error_code, char *message, int size, int context)
{

   strcpy(message, "");

#if !defined(_WIN32)
   switch (error_code) {
      case E2BIG:
         strncpy(message, "Argument list too long.", size);
         break;
      case EACCES:
         strncpy(message, "Permission denied.", size);
         break;
      case EADDRINUSE:
         strncpy(message, "Address in use.", size);
         break;
      case EADDRNOTAVAIL:
         strncpy(message, "Address not available.", size);
         break;
      case EAFNOSUPPORT:
         strncpy(message, "Address family not supported.", size);
         break;
      case EAGAIN:
         strncpy(message, "Resource unavailable, try again.", size);
         break;
      case EALREADY:
         strncpy(message, "Connection already in progress.", size);
         break;
      case EBADF:
         strncpy(message, "Bad file descriptor.", size);
         break;
#if !defined(MACOSX) && !defined(FREEBSD)
      case EBADMSG:
         strncpy(message, "Bad message.", size);
         break;
#endif
      case EBUSY:
         strncpy(message, "Device or resource busy.", size);
         break;
      case ECANCELED:
         strncpy(message, "Operation canceled.", size);
         break;
      case ECHILD:
         strncpy(message, "No child processes.", size);
         break;
      case ECONNABORTED:
         strncpy(message, "Connection aborted.", size);
         break;
      case ECONNREFUSED:
         strncpy(message, "Connection refused.", size);
         break;
      case ECONNRESET:
         strncpy(message, "Connection reset.", size);
         break;
      case EDEADLK:
         strncpy(message, "Resource deadlock would occur.", size);
         break;
      case EDESTADDRREQ:
         strncpy(message, "Destination address required.", size);
         break;
      case EDOM:
         strncpy(message, "Mathematics argument out of domain of function.", size);
         break;
      case EDQUOT:
         strncpy(message, "Reserved.", size);
         break;
      case EEXIST:
         strncpy(message, "File exists.", size);
         break;
      case EFAULT:
         strncpy(message, "Bad address.", size);
         break;
      case EFBIG:
         strncpy(message, "File too large.", size);
         break;
      case EHOSTUNREACH:
         strncpy(message, "Host is unreachable.", size);
         break;
      case EIDRM:
         strncpy(message, "Identifier removed.", size);
         break;
      case EILSEQ:
         strncpy(message, "Illegal byte sequence.", size);
         break;
      case EINPROGRESS:
         strncpy(message, "Operation in progress.", size);
         break;
      case EINTR:
         strncpy(message, "Interrupted function.", size);
         break;
      case EINVAL:
         strncpy(message, "Invalid argument.", size);
         break;
      case EIO:
         strncpy(message, "I/O error.", size);
         break;
      case EISCONN:
         strncpy(message, "Socket is connected.", size);
         break;
      case EISDIR:
         strncpy(message, "Is a directory.", size);
         break;
      case ELOOP:
         strncpy(message, "Too many levels of symbolic links.", size);
         break;
      case EMFILE:
         strncpy(message, "Too many open files.", size);
         break;
      case EMLINK:
         strncpy(message, "Too many links.", size);
         break;
      case EMSGSIZE:
         strncpy(message, "Message too large.", size);
         break;
#if !defined(MACOSX) && !defined(OSF1) && !defined(FREEBSD)
      case EMULTIHOP:
         strncpy(message, "Reserved.", size);
         break;
#endif
      case ENAMETOOLONG:
         strncpy(message, "Filename too long.", size);
         break;
      case ENETDOWN:
         strncpy(message, "Network is down.", size);
         break;
      case ENETRESET:
         strncpy(message, "Connection aborted by network.", size);
         break;
      case ENETUNREACH:
         strncpy(message, "Network unreachable.", size);
         break;
      case ENFILE:
         strncpy(message, "Too many files open in system.", size);
         break;
      case ENOBUFS:
         strncpy(message, "No buffer space available.", size);
         break;
#if !defined(MACOSX) && !defined(FREEBSD)
      case ENODATA:
         strncpy(message, "[XSR] [Option Start] No message is available on the STREAM head read queue. [Option End]", size);
         break;
#endif
      case ENODEV:
         strncpy(message, "No such device.", size);
         break;
      case ENOENT:
         strncpy(message, "No such file or directory.", size);
         break;
      case ENOEXEC:
         strncpy(message, "Executable file format error.", size);
         break;
      case ENOLCK:
         strncpy(message, "No locks available.", size);
         break;
#if !defined(MACOSX) && !defined(OSF1) && !defined(FREEBSD)
      case ENOLINK:
         strncpy(message, "Reserved.", size);
         break;
#endif
      case ENOMEM:
         strncpy(message, "Not enough space.", size);
         break;
      case ENOMSG:
         strncpy(message, "No message of the desired type.", size);
         break;
      case ENOPROTOOPT:
         strncpy(message, "Protocol not available.", size);
         break;
      case ENOSPC:
         strncpy(message, "No space left on device.", size);
         break;
#if !defined(MACOSX) && !defined(FREEBSD)
      case ENOSR:
         strncpy(message, "[XSR] [Option Start] No STREAM resources. [Option End]", size);
         break;
#endif
#if !defined(MACOSX) && !defined(FREEBSD)
      case ENOSTR:
         strncpy(message, "[XSR] [Option Start] Not a STREAM. [Option End]", size);
         break;
#endif
      case ENOSYS:
         strncpy(message, "Function not supported.", size);
         break;
      case ENOTCONN:
         strncpy(message, "The socket is not connected.", size);
         break;
      case ENOTDIR:
         strncpy(message, "Not a directory.", size);
         break;
#if !defined(AIX) && !defined(AIX5)
      case ENOTEMPTY:
         strncpy(message, "Directory not empty.", size);
         break;
#endif
      case ENOTSOCK:
         strncpy(message, "Not a socket.", size);
         break;
      case ENOTSUP:
         strncpy(message, "Not supported.", size);
         break;
      case ENOTTY:
         strncpy(message, "Inappropriate I/O control operation.", size);
         break;
      case ENXIO:
         strncpy(message, "No such device or address.", size);
         break;
#if !defined(LINUX) && !defined(MACOSX) && !defined(FREEBSD)
      case EOPNOTSUPP:
         strncpy(message, "Operation not supported on socket.", size);
         break;
#endif
#if !defined(OSF1)
      case EOVERFLOW:
         strncpy(message, "Value too large to be stored in data type.", size);
         break;
#endif
      case EPERM:
         strncpy(message, "Operation not permitted.", size);
         break;
      case EPIPE:
         strncpy(message, "Broken pipe.", size);
         break;
#if !defined(MACOSX) && !defined(FREEBSD)
      case EPROTO:
         strncpy(message, "Protocol error.", size);
         break;
#endif
      case EPROTONOSUPPORT:
         strncpy(message, "Protocol not supported.", size);
         break;
      case EPROTOTYPE:
         strncpy(message, "Protocol wrong type for socket.", size);
         break;
      case ERANGE:
         strncpy(message, "Result too large.", size);
         break;
      case EROFS:
         strncpy(message, "Read-only file system.", size);
         break;
      case ESPIPE:
         strncpy(message, "Invalid seek.", size);
         break;
      case ESRCH:
         strncpy(message, "No such process.", size);
         break;
      case ESTALE:
         strncpy(message, "Reserved.", size);
         break;
#if !defined(MACOSX) && !defined(FREEBSD)
      case ETIME:
         strncpy(message, "[XSR] [Option Start] Stream ioctl() timeout. [Option End]", size);
         break;
#endif
      case ETIMEDOUT:
         strncpy(message, "Connection timed out.", size);
         break;
      case ETXTBSY:
         strncpy(message, "Text file busy.", size);
         break;
#if !defined(LINUX) && !defined(AIX) && !defined(AIX5) && !defined(MACOSX) && !defined(OSF1) && !defined(SOLARIS) && !defined(FREEBSD)
      case EWOULDBLOCK:
         strncpy(message, "Operation would block.", size);
         break;
#endif
      case EXDEV:
         strncpy(message, "Cross-device link.", size);
         break;
      default:
         strcpy(message, "");
      break;
   }
#endif

   return (int) strlen(message);
}

