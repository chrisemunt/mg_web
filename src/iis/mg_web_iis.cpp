/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: IIS HTTP Gateway for InterSystems Cache/IRIS and YottaDB    |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2024 MGateway Ltd                                     |
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

#define _WINSOCKAPI_
#include <windows.h>
#include <sal.h>
#include <httpserv.h>
#include <stdio.h>
#include <time.h>
#include <Iiswebsocket.h>

#include "mg_web.h"
#include "mg_websocket.h"

#define MGWEB_RQ_NOTIFICATION_CONTINUE       1
#define MGWEB_RQ_NOTIFICATION_PENDING        2
#define MGWEB_RQ_NOTIFICATION_FINISH_REQUEST 3
#define RBUFFER_SIZE                         2048


#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagMGWEBIIS {
   IHttpContext *       phttp_context;
   IHttpContext3 *      phttp_context3;
   IHttpEventProvider * pprovider;
   IHttpResponse *      phttp_response;
   IHttpRequest *       phttp_request;
   IHttpServer *        phttp_server;
   HTTP_REQUEST *       phttp_rawrequest;
   IWebSocketContext *  pwebsocket_context;
   DBXMUTEX             wsmutex;
   PCSTR                rbuffer;
   int                  exit_code;
} MGWEBIIS, *LPWEBIIS;

#ifdef __cplusplus
}
#endif

IHttpServer * phttp_server = NULL;


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
   switch (fdwReason)
   { 
      case DLL_PROCESS_ATTACH:
         GetModuleFileName((HMODULE) hinstDLL, mg_system.module_file, 250);
         mg_worker_init();
         break;
      case DLL_THREAD_ATTACH:
         break;
      case DLL_THREAD_DETACH:
         break;
      case DLL_PROCESS_DETACH:
         mg_worker_exit();
         break;
   }
   return TRUE;
}


/* Create the module class. */
class mg_web_iis : public CHttpModule
{
public:
   REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext * pHttpContext, IN IHttpEventProvider * pProvider)
   {
      UNREFERENCED_PARAMETER( pProvider );
      int request_chunked;
      unsigned long request_clen;
      char buffer[256];
      DWORD size;
      MGWEB *pweb;
      MGWEBIIS mgwebiis, *pwebiis;
      /* Create an HRESULT to receive return values from methods. */
      HRESULT hr = 0;

      /* Retrieve a pointer to the response. */
      IHttpResponse * pHttpResponse = pHttpContext->GetResponse();
      IHttpRequest * pHttpRequest = pHttpContext->GetRequest();

      pwebiis = &mgwebiis;

      pwebiis->phttp_context = pHttpContext;
      pwebiis->pprovider = pProvider;
      pwebiis->phttp_response = pHttpResponse;
      pwebiis->phttp_request = pHttpRequest;

      pwebiis->phttp_server = phttp_server;
      pwebiis->phttp_context3 = NULL;
      pwebiis->pwebsocket_context = NULL;

      pwebiis->phttp_rawrequest = pHttpRequest->GetRawHttpRequest();

      pwebiis->rbuffer = (PCSTR) pHttpContext->AllocateRequestMemory(RBUFFER_SIZE);

      request_clen = 0;
      size = 32;
      hr = ((IHttpContext *) pwebiis->phttp_context)->GetServerVariable("CONTENT_LENGTH", (PCSTR *) &(pwebiis->rbuffer), &size);
      if (SUCCEEDED(hr)) {
         request_clen = (unsigned long) strtol((char *) pwebiis->rbuffer, NULL, 10);
      }
      /* v2.8.37 */
      request_chunked = 0;
      size = 128;
      buffer[0] = '\0';
      hr = ((IHttpContext *) pwebiis->phttp_context)->GetServerVariable("HTTP_TRANSFER_ENCODING", (PCSTR *) &(pwebiis->rbuffer), &size);
      if (SUCCEEDED(hr)) {
         if (size > 0 && size < 128) {
            strcpy_s(buffer, size + 1, (char *) pwebiis->rbuffer);
         }
         mg_lcase((char *) buffer);
         if (strstr(buffer, "chunked")) {
            request_chunked = 1;
            request_clen = 0;
         }
      }

      pweb = mg_obtain_request_memory((void *) pwebiis, request_clen, request_chunked, MG_WS_IIS); /* v2.7.33 */

      pweb->pweb_server = (void *) pwebiis;
      pweb->evented = 0;
      pweb->wserver_chunks_response = 0;
      pweb->http_version_major = pwebiis->phttp_rawrequest->Version.MajorVersion;
      pweb->http_version_minor = pwebiis->phttp_rawrequest->Version.MajorVersion;
      if ((pwebiis->phttp_rawrequest->Flags & HTTP_REQUEST_FLAG_HTTP2)) {
         pweb->http_version_major = 2;
         pweb->http_version_minor = 0;
         pweb->wserver_chunks_response = 1;
      }

/*
      {
         char bufferx[256];
         sprintf(bufferx, "HTTP raw version: %d.%d; flags=%lu ishttp2=%d", pweb->http_version_major, pweb->http_version_minor, pwebiis->phttp_rawrequest->Flags, (pwebiis->phttp_rawrequest->Flags & HTTP_REQUEST_FLAG_HTTP2));
         mg_log_event(&(mg_system.log), NULL, bufferx, "mg_web: information", 0);
      }
*/

      /* Test for an error. */
      if (pHttpResponse != NULL) {

         pwebiis->exit_code = MGWEB_RQ_NOTIFICATION_FINISH_REQUEST;

         /* Clear the existing response. */
         pHttpResponse->Clear();
         /* Disable buffering for this response. */
         pHttpResponse->DisableBuffering();
         /* Set the default MIME type to plain text/html. */
         pHttpResponse->SetHeader(HttpHeaderContentType, "text/html", (USHORT)strlen("text/html"), TRUE);

         mg_web((MGWEB *) pweb);
 
         mg_release_request_memory(pweb);

         if (pwebiis->exit_code == MGWEB_RQ_NOTIFICATION_CONTINUE)
            return RQ_NOTIFICATION_CONTINUE;
         else if (pwebiis->exit_code == MGWEB_RQ_NOTIFICATION_PENDING)
            return RQ_NOTIFICATION_PENDING;
         else if (pwebiis->exit_code == MGWEB_RQ_NOTIFICATION_FINISH_REQUEST)
            return RQ_NOTIFICATION_FINISH_REQUEST;
         else
            return RQ_NOTIFICATION_FINISH_REQUEST;
      }

      mg_release_request_memory(pweb);

      /* Return processing to the pipeline. */
      return RQ_NOTIFICATION_CONTINUE;
   }

   /* Process an RQ_MAP_REQUEST_HANDLER post-event. */
   REQUEST_NOTIFICATION_STATUS OnPostExecuteMapRequestHandler(IN IHttpContext * pHttpContext, IN IHttpEventProvider * pProvider)
   {
      UNREFERENCED_PARAMETER( pHttpContext );
      UNREFERENCED_PARAMETER( pProvider );

      return RQ_NOTIFICATION_CONTINUE;
   }

};


/* Create the module's class factory. */
class mg_web_iis_factory : public IHttpModuleFactory
{
public:
   HRESULT GetHttpModule(OUT CHttpModule ** ppModule, IN IModuleAllocator * pAllocator)
   {
      UNREFERENCED_PARAMETER( pAllocator );

      /* Create a new instance. */
      mg_web_iis * pmodule = new mg_web_iis;

      /* Test for an error. */
      if (!pmodule) {
         /* Return an error if the factory cannot create the instance. */
         return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
      }
      else {
         /* Return a pointer to the module. */
         *ppModule = pmodule;
         pmodule = NULL;
         /* Return a success status. */
         return S_OK;
      }            
   }

   void Terminate()
   {
      /* Remove the class from memory. */
      delete this;
   }
};


#ifdef __cplusplus
extern "C" {
#endif

/* Create the module's exported registration function. */
HRESULT __declspec(dllexport) RegisterModule(DWORD dwServerVersion, IHttpModuleRegistrationInfo *pModuleInfo, IHttpServer *pGlobalInfo)
{
   UNREFERENCED_PARAMETER(dwServerVersion);
   UNREFERENCED_PARAMETER(pGlobalInfo);

   phttp_server = pGlobalInfo;

   /* Set the request notifications and exit. */
   return pModuleInfo->SetRequestNotifications(new mg_web_iis_factory, RQ_EXECUTE_REQUEST_HANDLER, 0);
}

#ifdef __cplusplus
}
#endif


int mg_get_cgi_variable(MGWEB *pweb, char *name, char *pbuffer, int *pbuffer_size)
{
   short phase;
   int rc, offset;
   char *pa, *pz, *pv;
   char buffer[256];
   HRESULT hr;
   DWORD osize, rsize;
   PCSTR buff = NULL;
   IHttpContext * phttp_context;
   IHttpEventProvider * pprovider;

   phase = 0;

#ifdef _WIN32
__try {
#endif

	phttp_context = (IHttpContext *) ((MGWEBIIS *) pweb->pweb_server)->phttp_context;
	pprovider = (IHttpEventProvider *) ((MGWEBIIS *) pweb->pweb_server)->pprovider;
   rc = MG_CGI_UNDEFINED;
   phase = 0;
   osize = *pbuffer_size;

   if (!strcmp(name, "QUERY_STRING")) {

      /*
         Workaround for a problem in IIS that could corrupt query strings containing un-escaped Shift JIS characters
         Ref: http://forums.iis.net/p/1149967/1872171.aspx#1872171
      */

      const HTTP_REQUEST *phttp_request = ((MGWEBIIS *) pweb->pweb_server)->phttp_rawrequest;
      if (phttp_request->CookedUrl.QueryStringLength != 0) {

         DWORD query_len = phttp_request->CookedUrl.QueryStringLength / sizeof(WCHAR) - 1;
         PSTR pquery = (PSTR) phttp_context->AllocateRequestMemory(query_len + 1);

         if (pquery == NULL) {
            return MG_CGI_TOOLONG;
         }

         for (DWORD i = 0; i <= query_len; i ++) {
            pquery[i] = (CHAR) phttp_request->CookedUrl.pQueryString[i + 1];
         }

         if ((osize + 1) < query_len) {
            *pbuffer_size = query_len;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return MG_CGI_TOOLONG;
         }
         *pbuffer_size = query_len;

         if (osize > (DWORD) (*pbuffer_size + 1)) {
		      strncpy_s((char *) pbuffer, osize, (char *) pquery, *pbuffer_size + 1);
            pbuffer[*pbuffer_size + 1] = '\0';
         }

         return MG_CGI_SUCCESS;
      }
   }
   else if (!strcmp(name, "HTTP*")) {
      buff = (PCSTR) ((MGWEBIIS *) pweb->pweb_server)->rbuffer;
      hr = phttp_context->GetServerVariable("ALL_HTTP", (PCSTR *) &buff, (DWORD *) pbuffer_size);
      if (SUCCEEDED(hr)) {
         pa = (char *) buff;
         pz = strstr(pa, "\n");
         while (pz && *pa) {
            *pz = '\0';
            pv = strstr(pa, ":");
            if (pv) {
               *pv = '\0';
               pv ++;
               offset = 0;
               if (!strcmp(pa, "HTTP_CONTENT_LENGTH") || !strcmp(pa, "HTTP_CONTENT_TYPE")) {
                  offset = 5;
               }
               mg_add_cgi_variable(pweb, pa + offset, (int) strlen(pa + offset), (char *) pv, (int) strlen(pv));
            }
            pa = (pz + 1);
            pz = strstr(pa, "\n");
         }
      }
	   rc = MG_CGI_LIST;
   }
   else {
      buff = (PCSTR) ((MGWEBIIS *) pweb->pweb_server)->rbuffer;
      hr = phttp_context->GetServerVariable(name, (PCSTR *) &buff, (DWORD *) pbuffer_size);

      if (buff) {
         rsize = (DWORD) strlen(buff);
      }

     if (SUCCEEDED(hr)) {
         if (rsize > 0 && osize < (DWORD) ((*pbuffer_size) + 1)) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            phase = 111;
            *pbuffer_size = 0;
            rc = MG_CGI_TOOLONG;
         }
         else {
            phase = 3;
            if (!strcmp(name, "SERVER_SOFTWARE")) {
               sprintf_s(buffer, sizeof(buffer), "%s mg_web/%s", (char *) buff, DBX_VERSION);
               *pbuffer_size = (int) strlen(buffer);
               buff = (PCSTR) buffer;
            }
            if (!strcmp(name, "SERVER_PROTOCOL")) {
               strcpy_s(buffer, (*pbuffer_size) + 1, (char *) buff);
               if (pweb->http_version_major == 2 && *pbuffer_size > 5 && buffer[5] == '1') {
                  buffer[5] = (char) (pweb->http_version_major + 48);
                  buffer[7] = (char) (pweb->http_version_minor + 48);
               }
               buff = (PCSTR) buffer;
            }
		      strcpy_s(pbuffer, (*pbuffer_size) + 1, (char *) buff);
            phase = 4;
		      rc = MG_CGI_SUCCESS;
	      }
      }
      else {
         if (hr == ERROR_INSUFFICIENT_BUFFER) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            phase = 1;
            *pbuffer_size = 0;
            rc = MG_CGI_TOOLONG;
         }
         else {
            phase = 2;
            *pbuffer_size = 0;
            rc = MG_CGI_UNDEFINED;
         }
      }
   }

   phase = 20;

	return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_get_server_variable: %x:%d", code, phase);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return MG_CGI_UNDEFINED;
}
#endif

}


int mg_client_read(MGWEB *pweb, unsigned char *pbuffer, int buffer_size)
{
   int result, total;
   HRESULT hr;
	DWORD bytes_received, byte_count;

#ifdef _WIN32
__try {
#endif

   result = 0;
   total = 0;
   for (;;) {

      bytes_received = (buffer_size - total);
	   /* Retrieve the byte count for the request. */
      byte_count = ((IHttpRequest *) ((MGWEBIIS *) pweb->pweb_server)->phttp_request)->GetRemainingEntityBytes();

	   /* Retrieve the request body. */
      hr = ((IHttpRequest *) ((MGWEBIIS *) pweb->pweb_server)->phttp_request)->ReadEntityBody((void *) (pbuffer + total), bytes_received, false, &bytes_received, NULL);
      /* Test for an error. */
/*
      {
         char buffer[256];
         sprintf_s(buffer, 255, "hr=%d; buffer_size=%d; byte_count=%d; bytes_received=%d; total=%d; FAILED(hr)=%d;", hr, buffer_size, byte_count, bytes_received, total, FAILED(hr));
         mg_log_event(pweb->plog, pweb, buffer, "mg_client_read", 0);
      }
*/
      if (FAILED(hr)) {
         break;
      }

      if (bytes_received > 0) {
         total += bytes_received;
      }
      if (total >= buffer_size) {
         break;
      }
   }

   if (FAILED(hr)) {
      /* End of data is okay. */
      if (ERROR_HANDLE_EOF != (hr & 0x0000FFFF)) {
		   /* Set the error status. */
         ((IHttpEventProvider *) ((MGWEBIIS *) pweb->pweb_server)->pprovider)->SetErrorStatus(hr);
         /* End additional processing. */
         ((MGWEBIIS *) pweb->pweb_server)->exit_code = MGWEB_RQ_NOTIFICATION_FINISH_REQUEST;
         pweb->request_read_status = -1;
		}
      else { /* End of data */
         pweb->request_read_status = 1;
      }
	}
	if (total > 0) {
      pbuffer[total] = '\0';
      result = (int) total;
	}

/*
   {
      char buffer[256];
      sprintf_s(buffer, 255, "buffer_size=%d; total=%d; pweb->request_read_status=%d;", buffer_size, result, pweb->request_read_status);
      mg_log_event(pweb->plog, pweb, buffer, "mg_client_read", 0);
      if (total > 0) {
         mg_log_buffer(pweb->plog, pweb, (char *) pbuffer, total, "mg_client_read: buffer", 0);
      }
   }
*/

   return result;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_read_client: %x", code);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif
}


int mg_client_write(MGWEB *pweb, unsigned char *pbuffer, int buffer_size)
{
   short phase;
   int result, total, max;
   HRESULT hr;
   /* Buffer to store if asyncronous completion is pending. */
   BOOL completion_expected = false;
	/* Create a string with the response. */
	PCSTR pszBuffer = (PCSTR) pbuffer;
   /* Create a data chunk. */
   HTTP_DATA_CHUNK data_chunk;
   /* Set the chunk to a chunk in memory. */
   data_chunk.DataChunkType = HttpDataChunkFromMemory;
   /* Buffer for bytes written of data chunk. */
   DWORD sent;

   phase = 0;

#ifdef _WIN32
__try {
#endif

   result = 0;
   total = 0;

   for (;;) {
      max = (buffer_size - total);
      if (max > 60000) {
         max = 60000;
      }
      /* Set the chunk to the buffer. */
      data_chunk.FromMemory.pBuffer = (PVOID) (pbuffer + total);
      /* Set the chunk size to the buffer size. */
      data_chunk.FromMemory.BufferLength = (USHORT) max;

      phase = 1;

      /* Insert the data chunk into the response. */
      hr = ((IHttpResponse *) ((MGWEBIIS *) pweb->pweb_server)->phttp_response)->WriteEntityChunks(&data_chunk, 1, FALSE, TRUE, &sent);
      if (FAILED(hr)) {
         break;
      }
/*
      {
         char buffer[256];
         sprintf_s(buffer, 255, "hr=%d; buffer_size=%d; max=%d; cbSent=%d; FAILED(hr)=%d; SUCCEEDED(hr)=%d", hr, buffer_size, max, sent, FAILED(hr), SUCCEEDED(hr));
         mg_log_event(pweb->plog, pweb, buffer, "mg_client_write", 0);
      }
*/
      total += max;
      if (total >= buffer_size) {
         break;
      }
   }

   phase = 2;

   /* Flush the response to the client. */

   if (SUCCEEDED(hr)) {
	   result = sent;
   }
   else {
		/* Set the error status. */
      ((IHttpEventProvider *) ((MGWEBIIS *) pweb->pweb_server)->pprovider)->SetErrorStatus(hr);
      /* End additional processing. */
      ((MGWEBIIS *) pweb->pweb_server)->exit_code = MGWEB_RQ_NOTIFICATION_FINISH_REQUEST;
      result = 0;
   }

   phase = 9;

   return result;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_write_client: %x:%d", code, phase);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif

}


/* v2.7.33 */
int mg_client_write_now(MGWEB *pweb, unsigned char *pbuffer, int buffer_size)
{
   return mg_client_write(pweb, pbuffer, buffer_size);
}


int mg_suppress_headers(MGWEB *pweb)
{
   short phase;

   phase = 0;

#ifdef _WIN32
__try {
#endif

   ((IHttpResponse *) ((MGWEBIIS *) pweb->pweb_server)->phttp_response)->SuppressHeaders();
   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_suppress_headers: %x:%d", code, phase);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif
}


int mg_submit_headers(MGWEB *pweb)
{
   short phase;
   int n, status;
   char *pa, *pz, *p1, *p2;
   HRESULT hr;
   PCSTR reason;

   phase = 0;

#ifdef _WIN32
__try {
#endif

   ((IHttpResponse *) ((MGWEBIIS *) pweb->pweb_server)->phttp_response)->ClearHeaders();

   phase = 1;
   pa = pweb->response_headers;

   for (n = 0; ;n ++) {
      phase = 2;
      pz = strstr(pa, "\r\n");
      if (!pz)
         break;

      *pz = '\0';
      if (!strlen(pa))
         break;

      if (n == 0) {

         phase = 3;
         status = 200;
         reason = "OK";

         p1 = strstr(pa, " ");
         if (p1) {
            *p1 = '\0';
            p1 ++;
            while (*p1 == ' ')
               p1 ++;
            status = (int) strtol(p1, NULL, 10);
            p2 = strstr(p1, " ");
            if (p2) {
               *p2 = '\0';
               p2 ++;
               while (*p2 == ' ')
                  p2 ++;
               reason = p2;
            }
         }
         phase = 4;
         ((IHttpResponse *) ((MGWEBIIS *) pweb->pweb_server)->phttp_response)->SetStatus(status, reason, 0, S_OK);
         phase = 5;
      }
      else {
         p1 = strstr(pa, ":");
         if (p1) {
            *p1 = '\0';
            p1 ++;
            while (*p1 == ' ')
               p1 ++;
            phase = 6;
            hr = ((IHttpResponse *) ((MGWEBIIS *) pweb->pweb_server)->phttp_response)->SetHeader((PCSTR) pa, (PCSTR) p1, (USHORT) strlen(p1), FALSE);
            phase = 7;
         }
      }
      pa = (pz + 2);
   }

   phase = 9;

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_submit_headers: %x:%d", code, phase);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif

}


int mg_websocket_init(MGWEB *pweb)
{
   int rc;
   HRESULT hr;
   USHORT status_code;
   USHORT sub_status;
   DWORD cbsent;
   BOOL completion_expected;
   char buffer[256];
   IHttpServer * phttp_server;
   IHttpContext * phttp_context;
   IHttpContext3 * phttp_context3;
   IHttpResponse * phttp_response;
   MGWEBIIS * pwebiis;

#ifdef _WIN32
__try {
#endif

   pwebiis = (MGWEBIIS *) pweb->pweb_server;

   rc = 0;
   completion_expected = false;
   phttp_server = (IHttpServer *) pwebiis->phttp_server;
   phttp_context = (IHttpContext *) pwebiis->phttp_context;
   phttp_response = (IHttpResponse *) pwebiis->phttp_response;

   hr = HttpGetExtendedInterface(phttp_server, phttp_context, &phttp_context3);

   if (FAILED(hr)) {
      mg_log_event(pweb->plog, pweb, "HttpGetExtendedInterface failed", "mg_web: websocket error", 0);
      return CACHE_FAILURE;
   }

   status_code = 101;
   sub_status = 0;
   strcpy_s(buffer, 255, "Switching Protocols");
   hr = phttp_response->SetStatus(status_code, (PCSTR) buffer, sub_status, S_OK);

   cbsent = 0;
   hr = phttp_response->Flush(false, true, &cbsent, &completion_expected);

   pwebiis->pwebsocket_context = (IWebSocketContext *) phttp_context3->GetNamedContextContainer()->GetNamedContext((LPCWSTR) L"websockets");

   if (pwebiis->pwebsocket_context == NULL) {
      mg_log_event(pweb->plog, pweb, "WebSocket context failed", "mg_web: websocket error", 0);
      return CACHE_FAILURE;
   }
   else {
      phttp_context3->EnableFullDuplex();
      rc = CACHE_SUCCESS;
   }
   pweb->pwsock->status = MG_WEBSOCKET_HEADERS_SENT;

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER ) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_websocket_init: %x", code);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif
}


int mg_websocket_create_lock(MGWEB *pweb)
{
   MGWEBIIS *pwebiis;

   pwebiis = (MGWEBIIS *) pweb->pweb_server;
   mg_mutex_create(&(pwebiis->wsmutex));

   return 0;
}


int mg_websocket_destroy_lock(MGWEB *pweb)
{
   MGWEBIIS *pwebiis;

   pwebiis = (MGWEBIIS *) pweb->pweb_server;
   mg_mutex_destroy(&(pwebiis->wsmutex));

   return 0;
}


int mg_websocket_lock(MGWEB *pweb)
{
   int rc;
   MGWEBIIS *pwebiis;

   pwebiis = (MGWEBIIS *) pweb->pweb_server;
   rc = mg_mutex_lock(&(pwebiis->wsmutex), 0);

   return rc;
}


int mg_websocket_unlock(MGWEB *pweb)
{
   int rc;
   MGWEBIIS *pwebiis;

   pwebiis = (MGWEBIIS *) pweb->pweb_server;
   rc = mg_mutex_unlock(&(pwebiis->wsmutex));

   return rc;
}


int mg_websocket_frame_init(MGWEB *pweb)
{
   if (pweb->pwsock) {
      return CACHE_SUCCESS;
   }
   else {
      return CACHE_FAILURE;
   }
}


int mg_websocket_frame_read(MGWEB *pweb, MGWSRSTATE *pread_state)
{
   pweb->pwsock->block_size = mg_websocket_read_block(pweb, (char *) pweb->pwsock->block, sizeof(pweb->pwsock->block));

   if (pweb->pwsock->block_size < 1) {
      pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
      pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
      return -1;
   }

   mg_websocket_incoming_frame(pweb, pread_state, (char *) pweb->pwsock->block, pweb->pwsock->block_size);

   return 0;
}


int mg_websocket_frame_exit(MGWEB *pweb)
{
   return 0;
}


size_t mg_websocket_read_block(MGWEB *pweb, char *buffer, size_t bufsiz)
{
   int n, offs;
   size_t get;
   long long payload_length;

   offs = 0;
   payload_length = 0;

   if (pweb->pwsock->remaining_length == 0) {
      n = (int) mg_websocket_read(pweb, (char *) buffer, 6);
      offs = 6;

      if (n == 0) {
         return n;
      }
      payload_length = (long long) MG_WS_FRAME_GET_PAYLOAD_LEN((unsigned char) buffer[1]);
      if (payload_length == 126) {
         n = (int) mg_websocket_read(pweb, (char *) buffer + offs, 2);
         offs += 2;
         if (n == 0) {
            return n;
         }
         payload_length = (((unsigned char) buffer[2]) * 0x100) + ((unsigned char) buffer[3]);

      }
      else if (payload_length == 127) {
         n = (int) mg_websocket_read(pweb, (char *) buffer + offs, 8);
         offs += 8;
         if (n == 0) {
            return n;
         }
         payload_length = 0;
#if defined(_WIN64)
         payload_length += (((unsigned char) buffer[2]) * 0x100000000000000);
         payload_length += (((unsigned char) buffer[3]) * 0x1000000000000);
         payload_length += (((unsigned char) buffer[4]) * 0x10000000000);
         payload_length += (((unsigned char) buffer[5]) * 0x100000000);
#endif
         payload_length += (((unsigned char) buffer[6]) * 0x1000000);
         payload_length += (((unsigned char) buffer[7]) * 0x10000);
         payload_length += (((unsigned char) buffer[8]) * 0x100);
         payload_length += (((unsigned char) buffer[9]));
      }

      pweb->pwsock->remaining_length = (size_t) payload_length;
   }

   if ((pweb->pwsock->remaining_length + (offs + 1)) < bufsiz)
      get = pweb->pwsock->remaining_length;
   else
      get = (bufsiz - (offs + 1));

   if (get > 0) {
      n = (int) mg_websocket_read(pweb, (char *) buffer + offs, (int) get);
      pweb->pwsock->remaining_length -= n;
   }
   else {
      n = 0;
      pweb->pwsock->remaining_length = 0;
   }

   return (n + offs);
}


size_t mg_websocket_read(MGWEB *pweb, char *buffer, size_t bufsiz)
{
   size_t bytes_read;

   bytes_read = (size_t) mg_client_read(pweb, (unsigned char *) buffer, (int) bufsiz);

   return bytes_read;
}


size_t mg_websocket_queue_block(MGWEB *pweb, int type, unsigned char *buffer, size_t buffer_size, short locked)
{
   size_t written;

   written = 0;

   if (!pweb->pwsock) {
      return 0;
   }

   if (type == MG_WS_MESSAGE_TYPE_CLOSE) {
      if (pweb->pwsock->status == MG_WEBSOCKET_CLOSED) {
         goto mg_websocket_queue_block_exit;
      }
      else {
         pweb->pwsock->status = MG_WEBSOCKET_CLOSED;
      }
   }

   written = mg_websocket_write_block(pweb, type, buffer, buffer_size);

mg_websocket_queue_block_exit:

   if (!locked) {
      mg_websocket_unlock(pweb);
   }

   return written;
}


size_t mg_websocket_write_block(MGWEB *pweb, int type, unsigned char *buffer, size_t buffer_size)
{
   unsigned char header[32];
   size_t pos, written;
   mg_uint64_t payload_length;

   pos = 0;
   written = 0;
   payload_length = (mg_uint64_t) ((buffer != NULL) ? buffer_size : 0);

   if (!pweb->pwsock->closing) {

      pos = mg_websocket_create_header(pweb, type, header, payload_length);

      mg_websocket_write(pweb, (char *) header, (int) pos); /* Header */

      if (payload_length > 0) {
         if (mg_websocket_write(pweb, (char *) buffer, (int) buffer_size) > 0) { /* Payload Data */
            written = buffer_size;
         }
      }
   }

   return written;
}



int mg_websocket_write(MGWEB *pweb, char *buffer, int len)
{
   int rc;

   rc = mg_client_write(pweb, (unsigned char *) buffer, (int) len);
   return rc;
}


int mg_websocket_exit(MGWEB *pweb)
{
   MGWEBIIS *pwebiis;

   pwebiis = (MGWEBIIS *) pweb->pweb_server;

   if (pwebiis->pwebsocket_context) {
      pwebiis->pwebsocket_context->CloseTcpConnection();
   }

   return CACHE_SUCCESS;
}
