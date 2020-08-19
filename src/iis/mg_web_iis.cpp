/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: IIS HTTP Gateway for InterSystems Cache/IRIS and YottaDB    |
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

#define _WINSOCKAPI_
#include <windows.h>
#include <sal.h>
#include <httpserv.h>
#include <stdio.h>
#include <time.h>

#include "mg_web.h"

#define MGWEB_RQ_NOTIFICATION_CONTINUE       1
#define MGWEB_RQ_NOTIFICATION_PENDING        2
#define MGWEB_RQ_NOTIFICATION_FINISH_REQUEST 3
#define RBUFFER_SIZE                         2048


#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagMGWEBIIS {
   void *         phttp_context;
   void *         pprovider;
   void *         phttp_response;
   void *         phttp_request;
   HTTP_REQUEST * phttp_rawrequest;
   PCSTR          rbuffer;
   int            exit_code;
} MGWEBIIS, *LPWEBIIS;

#ifdef __cplusplus
}
#endif


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
      unsigned long request_clen;
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
      pwebiis->phttp_rawrequest = pHttpRequest->GetRawHttpRequest();

      pwebiis->rbuffer = (PCSTR) pHttpContext->AllocateRequestMemory(RBUFFER_SIZE);

      size = 32;
      hr = ((IHttpContext *) pwebiis->phttp_context)->GetServerVariable("CONTENT_LENGTH", (PCSTR *) &(pwebiis->rbuffer), &size);
      request_clen = (unsigned long) strtol((char *) pwebiis->rbuffer, NULL, 10);

      pweb = mg_obtain_request_memory((void *) pwebiis, request_clen);

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
   short phase;
   int result, total;
   HRESULT hr;
	DWORD bytes_received, byte_count;

   phase = 0;

#ifdef _WIN32
__try {
#endif

   result = 0;
   total = 0;
   for (;;) {
      bytes_received = (buffer_size - total);
	   /* Retrieve the byte count for the request. */
      byte_count = ((IHttpRequest *) ((MGWEBIIS *) pweb->pweb_server)->phttp_request)->GetRemainingEntityBytes();

      phase = 1;

	   /* Retrieve the request body. */
      hr = ((IHttpRequest *) ((MGWEBIIS *) pweb->pweb_server)->phttp_request)->ReadEntityBody((void *) (pbuffer + total), bytes_received, false, &bytes_received, NULL);
      /* Test for an error. */
/*
      {
         char buffer[256];
         sprintf_s(buffer, 255, "hr=%d; buffer_size=%d; bytes_received=%d; total=%d; FAILED(hr)=%d;", hr, buffer_size, bytes_received, total, FAILED(hr));
         mg_log_event(pweb->plog, pweb, buffer, "mg_client_read", 0);
      }
*/
      if (bytes_received > 0) {
         total += bytes_received;
      }
      if (total >= buffer_size) {
         break;
      }
      if (FAILED(hr)) {
         break;
      }

   }

   phase = 2;

   if (FAILED(hr)) {

      phase = 3;

      /* End of data is okay. */
      if (ERROR_HANDLE_EOF != (hr & 0x0000FFFF)) {
         phase = 4;
		   /* Set the error status. */
         ((IHttpEventProvider *) ((MGWEBIIS *) pweb->pweb_server)->pprovider)->SetErrorStatus(hr);
         /* End additional processing. */
         ((MGWEBIIS *) pweb->pweb_server)->exit_code = MGWEB_RQ_NOTIFICATION_FINISH_REQUEST;
         bytes_received = 0;
         result = -1;
		}
      else { /* End of data */
         result = 0;
      }
	}
	else if (bytes_received > 0) {
      phase = 5;
      pbuffer[total] = '\0';
      result = (int) total;
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
      sprintf_s(buffer, 255, "Exception caught in f:mg_read_client: %x:%d", code, phase);
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

