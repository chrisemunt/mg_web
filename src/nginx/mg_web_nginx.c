/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: Nginx HTTP Gateway for InterSystems Cache/IRIS and YottaDB  |
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
   | http://www.nginx.org/licenses/LICENSE-2.0                                |
   |                                                                          |
   | Unless required by applicable law or agreed to in writing, software      |
   | distributed under the License is distributed on an "AS IS" BASIS,        |
   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
   | See the License for the specific language governing permissions and      |
   | limitations under the License.                                           |      
   |                                                                          |
   ----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <time.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#if !defined(_WIN32)
#include <pthread.h>
#endif

#include "mg_web.c"

#define MG_MAGIC_TYPE1        "application/x-mgweb"
#define MG_MAGIC_TYPE2        "text/mgweb"
#define MG_FILE_TYPES         ".mgw.mgweb."
#define MG_DEFAULT_TIMEOUT    300000
#define MG_RBUFFER_SIZE       1024
/*
#define MG_API_TRACE          1
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   int   n;
   char  mg_config_file[256];
   char  mg_log_file[256];
   char  mg_thread_pool[64];
} mg_conf_main;


typedef struct {
   int   n;
   char  mg_config_file[256];
   char  mg_log_file[256];
   char  mg_thread_pool[64];
} mg_conf_server;


typedef struct {
   short mg_enabled;
   char  mg_file_types[128];
   char  mg_config_file[256];
   char  mg_log_file[256];
   char  mg_thread_pool[64];
} mg_conf_location;


typedef struct tagMGWEBNGINX {
   ngx_http_request_t   *r;
   mg_conf_main         *mconf;
   mg_conf_server       *sconf;
   mg_conf_location     *dconf;
   ngx_buf_t            *output_buf_last;
   ngx_chain_t          *output_head;
   ngx_chain_t          *output_last;
   unsigned int         more_request_content;
   ngx_str_t            thread_pool;
   char                 mg_thread_pool[64];
   int                  async;
   MGWEB                *pweb;
} MGWEBNGINX, *LPWEBNGINX;


typedef struct {
   MGWEBNGINX  *pwebnginx;
} mg_thread_ctx;


#ifdef __cplusplus
}
#endif

/*
 * Declare ourselves so the configuration routines can find and know us.
 * We'll fill it in at the end of the module.
 */

static ngx_int_t     mg_handler                 (ngx_http_request_t *r);
static void          mg_payload_handler         (ngx_http_request_t *r);
static int           mg_execute                 (ngx_http_request_t *r, MGWEBNGINX *pwebnginx);
static int           mg_execute_launch_thread   (MGWEBNGINX *pwebnginx);
#if defined(_WIN32)
DWORD WINAPI         mg_execute_detached_thread (LPVOID pargs);
#else
void *               mg_execute_detached_thread (void *pargs);
#endif
#if defined(NGX_THREADS)
static void          mg_execute_pool            (void *data, ngx_log_t *log);
static void          mg_execute_pool_completion (ngx_event_t *ev);
#endif
static char *        mg_param_mgweb             (ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *        mg_param_mgwebfiletypes    (ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *        mg_param_mgwebconfigfile   (ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *        mg_param_mgweblogfile      (ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *        mg_param_mgwebthreadpool   (ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t     mg_pre_conf                (ngx_conf_t *cf);
static ngx_int_t     mg_post_conf               (ngx_conf_t *cf);
static void *        mg_create_main_conf        (ngx_conf_t *cf);
char *               mg_init_main_conf          (ngx_conf_t *cf, void *conf);
static void *        mg_create_server_conf      (ngx_conf_t *cf);
static char *        mg_merge_server_conf       (ngx_conf_t *cf, void *parent, void *child);
static void *        mg_create_location_conf    (ngx_conf_t *cf);
static char *        mg_merge_location_conf     (ngx_conf_t *cf, void *parent, void *child);
ngx_int_t            mg_master_init             (ngx_log_t *log);
void                 mg_master_exit             (ngx_cycle_t *cycle);
ngx_int_t            mg_module_init             (ngx_cycle_t *cycle);
ngx_int_t            mg_process_init            (ngx_cycle_t *cycle);
void                 mg_process_exit            (ngx_cycle_t *cycle);
ngx_int_t            mg_thread_init             (ngx_cycle_t *cycle);
void                 mg_thread_exit             (ngx_cycle_t *cycle);
int                  mg_check_file_type         (char *mg_file_types_main, char *mg_file_types_server, char *mg_file_types_loc, char *type);
void *               mg_malloc_nginx            (void *pweb_server, unsigned long size);
void *               mg_remalloc_nginx          (void *pweb_server, void *p, unsigned long size);
int                  mg_free_nginx              (void *pweb_server, void *p);


#if defined(MG_API_TRACE)
#if defined(_WIN32)
static DBXLOG debug_log = {"c:/temp/mgweb.log", "", "", 0, 0, 0, 0, 0, "", ""};
#else
static DBXLOG debug_log = {"/tmp/mgweb.log", "", "", 0, 0, 0, 0, 0, "", ""};
#endif
#endif

#if defined(_WIN32)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
   switch (fdwReason)
   { 
      case DLL_PROCESS_ATTACH:
         GetModuleFileName((HMODULE) hinstDLL, mg_system.module_file, 250);
         break;
      case DLL_THREAD_ATTACH:
         break;
      case DLL_THREAD_DETACH:
         break;
      case DLL_PROCESS_DETACH:
         break;
   }
   return TRUE;
}
#endif


static ngx_http_module_t  ngx_http_mg_web_ctx = {
   mg_pre_conf,               /* preconfiguration */
   mg_post_conf,              /* postconfiguration */

   mg_create_main_conf,       /* create main configuration */
   mg_init_main_conf,         /* init main configuration */

   mg_create_server_conf,     /* create server configuration */
   mg_merge_server_conf,      /* merge server configuration */

   mg_create_location_conf,   /* create location configuration */
   mg_merge_location_conf     /* merge location configuration */
};

ngx_module_t  ngx_http_mg_web;

static ngx_command_t  ngx_http_mg_web_commands[] = {
   {
      ngx_string("MGWEB"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      mg_param_mgweb,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL
   },

   {
      ngx_string("MGWEBFileTypes"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_ANY,
      mg_param_mgwebfiletypes,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL
   },

   {
      ngx_string("MGWEBConfigFile"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_ANY,
      mg_param_mgwebconfigfile,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL
   },

   {
      ngx_string("MGWEBLogFile"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_ANY,
      mg_param_mgweblogfile,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL
   },
   {
      ngx_string("MGWEBThreadPool"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      mg_param_mgwebthreadpool,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL
   },
};


ngx_module_t  ngx_http_mg_web = {
   NGX_MODULE_V1,
   &ngx_http_mg_web_ctx,      /* module context */
   ngx_http_mg_web_commands,  /* module directives */
   NGX_HTTP_MODULE,           /* module type */
   mg_master_init,            /* init master */
   mg_module_init,            /* init module */
   mg_process_init,           /* init process */
   mg_thread_init,            /* init thread */
   mg_thread_exit,            /* exit thread */
   mg_process_exit,           /* exit process */
   mg_master_exit,            /* exit master */
   NGX_MODULE_V1_PADDING
};


/* Main content handler for MGWEB requests */

static ngx_int_t mg_handler(ngx_http_request_t *r)
{
   int n, clen, ok;
   int rc;
   char ext[16];
   mg_conf_location *dconf;
   MGWEB *pweb;
   MGWEBNGINX *pwebnginx;

#if defined(MG_API_TRACE)
   ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "mg_web: mg_handler()");
#endif

   dconf = ngx_http_get_module_loc_conf(r, ngx_http_mg_web);
   ext[0] = '\0';
   if (r->uri.len > 3) {
      for (n = r->uri.len - 1; n > 1; n --) {
         if (r->uri.data[n] == '.') {
            n ++;
            clen = r->uri.len - n;
            if (clen < 16) {
               strncpy(ext, (char *) (r->uri.data + n), clen);
               ext[clen] = '\0';
               break;
            }
         }
      }
   }

   ok = mg_check_file_type("", "", dconf->mg_file_types, ext);

   if (!ok && !dconf->mg_enabled) {
      return NGX_DECLINED;
   }
     
   pwebnginx = ngx_pcalloc(r->pool, sizeof(MGWEBNGINX));
   if (pwebnginx == NULL) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
   }

   ngx_http_set_ctx(r, pwebnginx, ngx_http_mg_web);
   pwebnginx->r = r;

   pweb = (MGWEB *) ngx_pcalloc(r->pool, sizeof(MGWEB) + 128100);
   if (pweb == NULL) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
   }
   pwebnginx->pweb = pweb;

   pwebnginx->async = 1;
   strcpy(pwebnginx->mg_thread_pool, dconf->mg_thread_pool);
   pwebnginx->thread_pool.data = (u_char *) pwebnginx->mg_thread_pool;
   pwebnginx->thread_pool.len = (size_t) strlen(pwebnginx->mg_thread_pool);

   pweb->pweb_server = (void *) pwebnginx;
   pweb->input_buf.buf_addr = ((char *) pweb) + sizeof(MGWEB);
   pweb->input_buf.buf_addr += 15;
   pweb->input_buf.len_alloc = 128000;
   pweb->input_buf.len_used = 0;

   pweb->output_val.svalue.buf_addr = pweb->input_buf.buf_addr;
   pweb->output_val.svalue.len_alloc = pweb->input_buf.len_alloc;
   pweb->output_val.svalue.len_used = pweb->input_buf.len_used;

   pweb = mg_obtain_request_memory((void *) pwebnginx, (unsigned long) (r->headers_in.content_length_n > 0 ? r->headers_in.content_length_n : 0));
   if (pweb == NULL) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
   }

   pwebnginx->pweb = pweb;
   pweb->pweb_server = (void *) pwebnginx;
   pweb->evented = 1;

   rc = mg_web((MGWEB *) pweb);

   mg_add_block_size((unsigned char *) pweb->input_buf.buf_addr + pweb->input_buf.len_used, (unsigned long) 0, (unsigned long) pweb->request_clen, DBX_DSORT_WEBCONTENT, DBX_DTYPE_STR);
   pweb->input_buf.len_used += 5;

   rc = ngx_http_read_client_request_body(r, mg_payload_handler);

   if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
      return rc;
   }

   if (rc == NGX_AGAIN) {
      pwebnginx->more_request_content = 1;
      return NGX_DONE;
   }

   return NGX_DONE;
}


static void mg_payload_handler(ngx_http_request_t *r)
{
   int rc;
   size_t len;
   ngx_buf_t *buf;
   ngx_chain_t *cl;
   size_t total;
   MGWEBNGINX *pwebnginx;
   MGWEB *pweb;

   pwebnginx = ngx_http_get_module_ctx(r, ngx_http_mg_web);
   pweb = pwebnginx->pweb;

   /* More request content */
   if (pwebnginx->more_request_content) {
      pwebnginx->more_request_content = 0;
      ngx_http_core_run_phases(r);
      return;
   }

   total = 0;
   cl = r->request_body->bufs;
 
   if (r->request_body->temp_file) {
/*
      {
         char bufferx[256];
         sprintf(bufferx, "request_clen=%d;", pweb->request_clen);
         mg_log_event(&(mg_system.log), NULL, bufferx, "mg_payload_handler: temp_file", 0);
      }
*/
      while ((len = ngx_read_file(&r->request_body->temp_file->file, (u_char *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used), 4096, total)) > 0) {
         total += len;
         pweb->input_buf.len_used += len;
      }
   }
   else {
/*
      {
         char bufferx[256];
         sprintf(bufferx, "request_clen=%d;", pweb->request_clen);
         mg_log_event(&(mg_system.log), NULL, bufferx, "mg_payload_handler: memory_chain", 0);
      }
*/
      while (cl) {
         buf = cl->buf;

         if (buf->pos) {
            len = (int) (buf->last - buf->pos);
            if (len == 0) {
               len = pweb->request_clen;
               memcpy((void *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used), (char *) buf->start, len);
               total += len;
               pweb->input_buf.len_used += len;
            }
            else {
               memcpy((void *) (pweb->input_buf.buf_addr + pweb->input_buf.len_used), (char *) buf->start, len);
               total += len;
               pweb->input_buf.len_used += len;
            }
         }
         cl = cl->next;
      }
   }

#if defined(NGX_THREADS)
   if (pwebnginx->thread_pool.len > 0) {
      ngx_thread_pool_t *tp = NULL;
      ngx_thread_task_t *task = NULL;
      mg_thread_ctx *task_ctx = NULL;

      tp = ngx_thread_pool_get((ngx_cycle_t *) ngx_cycle, &(pwebnginx->thread_pool));
      if (tp == NULL) {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "mg_web: thread pool \"%V\" not found", &(pwebnginx->thread_pool));
         return;
      }

      task = ngx_thread_task_alloc(r->pool, sizeof(mg_thread_ctx));
      if (task == NULL) {
         return;
      }

      task_ctx = task->ctx;
      task_ctx->pwebnginx = pwebnginx;

      task->handler = mg_execute_pool;
      task->event.handler = mg_execute_pool_completion;
      task->event.data = (void *) r;

/*
      {
         char bufferx[256];
         sprintf(bufferx, "Thread Pool: task=%p; pwebnginx=%p r=%p;", task, pwebnginx, r);
         mg_log_event(&(mg_system.log), NULL, bufferx, "Thread Pool", 0);
      }
*/

      if (ngx_thread_task_post(tp, task) != NGX_OK) {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "mg_web: failed to post new task");
         return;
      }
/*
      r->main->blocked ++;
      r->aio = 1;
*/
      return;
   }
#endif

   if (pwebnginx->async) {
      rc = mg_execute_launch_thread(pwebnginx);
      if (rc != CACHE_SUCCESS) {
         return;
      }
   }
   else {
      rc = mg_execute(r, pwebnginx);
      if (rc != CACHE_SUCCESS) {
         return;
      }
   }

   return;
}


static int mg_execute(ngx_http_request_t *r, MGWEBNGINX *pwebnginx)
{
   int rc;
   MGWEB *pweb;

   pweb = pwebnginx->pweb;
   pwebnginx->output_last = NULL;
   pwebnginx->output_buf_last = NULL;

   rc = mg_web_process(pweb);

   if (pwebnginx->output_head) {
      pwebnginx->output_buf_last->last_buf = 1;
      rc = ngx_http_output_filter(r, pwebnginx->output_head);
      ngx_http_finalize_request(r, rc);
   }
   else {
      ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
   }

   return 1;
}


static int mg_execute_launch_thread(MGWEBNGINX *pwebnginx)
{
#if defined(_WIN32)
   HANDLE thread_handle;
   SIZE_T stack_size;
   DWORD thread_id, flags;
   LPSECURITY_ATTRIBUTES pattr;

   stack_size = 0;
   flags = 0;
   pattr = NULL;

   thread_handle = CreateThread(pattr, stack_size, (LPTHREAD_START_ROUTINE) mg_execute_detached_thread, (LPVOID) pwebnginx, flags, &thread_id);
   if (!thread_handle) {
      ngx_log_error(NGX_LOG_ERR, pwebnginx->r->connection->log, 0, "Failed to create thread.");
      ngx_http_finalize_request(pwebnginx->r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      return -1;
   }
   CloseHandle(thread_handle);
#else
   int rc = 0;
   pthread_attr_t attr;
   pthread_t child_thread;

   size_t stacksize, newstacksize;

   pthread_attr_init(&attr);

   stacksize = 0;
   pthread_attr_getstacksize(&attr, &stacksize);

   newstacksize = 0x40000; /* 262144 */
   newstacksize = 0x70000; /* 458752 */
   pthread_attr_setstacksize(&attr, newstacksize);

   rc = pthread_create(&child_thread, &attr, mg_execute_detached_thread, (void *) pwebnginx);
   if (rc) {
      ngx_log_error(NGX_LOG_ERR, pwebnginx->r->connection->log, 0, "Failed to create thread.");
      ngx_http_finalize_request(pwebnginx->r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      return -1;
   }
#endif

   return 0;
}


#if defined(_WIN32)
DWORD WINAPI mg_execute_detached_thread(LPVOID pargs)
#else
void * mg_execute_detached_thread(void *pargs)
#endif
{
   MGWEBNGINX *pwebnginx;

   pwebnginx = (MGWEBNGINX *) pargs;

#if !defined(_WIN32)
   pthread_detach(pthread_self());
#endif

   mg_execute(pwebnginx->r, pwebnginx);

#if defined(_WIN32)
   return 0;
#else
   return NULL;
#endif
}


#if defined(NGX_THREADS)
static void mg_execute_pool(void *data, ngx_log_t *log)
{
   mg_thread_ctx *task_ctx = data;
   MGWEBNGINX *pwebnginx;

   pwebnginx = task_ctx->pwebnginx;
/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_execute_pool: pwebnginx=%p; r=%p;", pwebnginx, pwebnginx->r);
      mg_log_event(&(mg_system.log), NULL, "mg_execute_pool", bufferx, 0);
   }
*/

   mg_execute(pwebnginx->r, pwebnginx);

   return;
}


static void mg_execute_pool_completion(ngx_event_t *ev)
{
/*
   ngx_http_request_t *r = (ngx_http_request_t *) ev->data;
*/

/*
   {
      char bufferx[256];
      sprintf(bufferx, "mg_execute_pool_completion: r=%p;", r);
      mg_log_event(&(mg_system.log), NULL, "mg_execute_pool_completion", bufferx, 0);
   }
*/

/*
   r->main->blocked --;
   r->aio = 0;

   ngx_http_handler(r);
*/

   return;
}
#endif /* #if defined(NGX_THREADS) */


static char * mg_param_mgweb(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   char *p;
   char buffer[256];
   ngx_str_t *value;
   mg_conf_location *dconf = conf;
   ngx_http_core_loc_conf_t  *clcf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_param_mgweb()", "mg_web: trace", 0);
#endif

   value = cf->args->elts;

   p = (char *) value[1].data;

   dconf->mg_enabled = 0;
   if (p) {
      strcpy(buffer, p);
      mg_lcase(buffer);
      if (!strcmp(buffer, "on")) {
         dconf->mg_enabled = 1;
      }
   }

   clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
   clcf->handler = mg_handler;

   return NGX_CONF_OK;
}


static char * mg_param_mgwebfiletypes(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   int n, len;
   char *p;
   char buffer[256];
   ngx_str_t *value;
   mg_conf_location *dconf = conf;
   ngx_http_core_loc_conf_t  *clcf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_param_mgwebfiletypes()", "mg_web: trace", 0);
#endif

   value = cf->args->elts;

   dconf->mg_file_types[0] = '\0';
   for (n = 1; ; n ++) {
      p = (char *) value[n].data;
      if (!p) {
         break;
      }

      strcpy(buffer, p);
      mg_lcase(buffer);
      len = strlen(buffer);
      if (len) {
         if (buffer[0] != '.') {
            strcat(dconf->mg_file_types, ".");
         }
         strcat(dconf->mg_file_types, buffer);
      }
   }
   if (dconf->mg_file_types[0]) {
      strcat(dconf->mg_file_types, ".");
   }

   clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
   clcf->handler = mg_handler;

   return NGX_CONF_OK;
}


static char * mg_param_mgwebconfigfile(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   char *p;
   ngx_str_t *value;
   mg_conf_main *dconf = conf;
   ngx_http_core_loc_conf_t  *clcf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_param_mgwebconfigfile()", "mg_web: trace", 0);
#endif

   value = cf->args->elts;

   p = (char *) value[1].data;

   dconf->mg_config_file[0] = '\0';
   if (p) {
      strcpy(dconf->mg_config_file, p);
   }

   clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
   clcf->handler = mg_handler;

   return NGX_CONF_OK;
}


static char * mg_param_mgweblogfile(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   char *p;
   ngx_str_t *value;
   mg_conf_main *dconf = conf;
   ngx_http_core_loc_conf_t  *clcf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_param_mgweblogfile()", "mg_web: trace", 0);
#endif

   value = cf->args->elts;

   p = (char *) value[1].data;

   dconf->mg_log_file[0] = '\0';
   if (p) {
      strcpy(dconf->mg_log_file, p);
   }

   clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
   clcf->handler = mg_handler;

   return NGX_CONF_OK;
}


static char * mg_param_mgwebthreadpool(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
   char *p;
   ngx_str_t *value;
   mg_conf_location *dconf = conf;
   ngx_http_core_loc_conf_t  *clcf;


#if defined(NGX_THREADS)
   ngx_thread_pool_t *tp;
#endif

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_param_mgweblogfile()", "mg_web: trace", 0);
#endif

   value = cf->args->elts;

#if defined(NGX_THREADS)
   if (cf->args->nelts > 1) {
      tp = ngx_thread_pool_add(cf, &value[1]);
   }
   else {
      tp = ngx_thread_pool_add(cf, NULL);
   }
   if (tp == NULL) {
      return NGX_CONF_ERROR;
   }
#endif

   if (cf->args->nelts > 1) {
      p = (char *) value[1].data;
      dconf->mg_thread_pool[0] = '\0';
      if (p) {
         strcpy(dconf->mg_thread_pool, p);
      }
   }

   clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
   clcf->handler = mg_handler;

   return NGX_CONF_OK;
}


static ngx_int_t mg_pre_conf(ngx_conf_t *cf)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_pre_conf()", "mg_web: trace", 0);
#endif

   return NGX_OK;
}


static ngx_int_t mg_post_conf(ngx_conf_t *cf)
{
   ngx_http_handler_pt *h;
   ngx_http_core_main_conf_t *cmcf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_post_conf()", "mg_web: trace", 0);
#endif

   cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

   h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
   if (h == NULL) {
      return NGX_ERROR;
   }

   *h = mg_handler;

   return NGX_OK;
}


static void * mg_create_main_conf(ngx_conf_t *cf)
{
   mg_conf_main *conf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_create_main_conf()", "mg_web: trace", 0);
#endif

   conf = ngx_pcalloc(cf->pool, sizeof(mg_conf_main));
   if (conf == NULL) {
      return NGX_CONF_ERROR;
   }

   conf->mg_config_file[0] = '\0';
   conf->mg_log_file[0] = '\0';
   conf->mg_thread_pool[0] = '\0';

#if defined(MG_API_TRACE)
   {
      char bufferx[256];
      sprintf(bufferx, "mg_create_main_conf: conf=%p", conf);
      mg_log_event(&debug_log, NULL, conf->mg_thread_pool, bufferx, 0);
   }
#endif

   return conf;
}


char * mg_init_main_conf(ngx_conf_t *cf, void *conf)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_init_main_conf()", "mg_web: trace", 0);
#endif

   return NGX_CONF_OK;
}


static void * mg_create_server_conf(ngx_conf_t *cf)
{
   mg_conf_server *conf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_create_server_conf()", "mg_web: trace", 0);
#endif

   conf = ngx_pcalloc(cf->pool, sizeof(mg_conf_server));
   if (conf == NULL) {
      return NGX_CONF_ERROR;
   }

   conf->mg_config_file[0] = '\0';
   conf->mg_log_file[0] = '\0';
   conf->mg_thread_pool[0] = '\0';

#if defined(MG_API_TRACE)
   {
      char bufferx[256];
      sprintf(bufferx, "mg_create_server_conf: conf=%p", conf);
      mg_log_event(&debug_log, NULL, conf->mg_thread_pool, bufferx, 0);
   }
#endif

   return conf;
}


static char * mg_merge_server_conf(ngx_conf_t *cf, void *parent, void *child)
{
#if defined(MG_API_TRACE)
   ngx_log_error(NGX_LOG_INFO, cf->log, 0, "mg_web: mg_merge_server_conf()", 0);
#endif

   return NGX_CONF_OK;
}



static void * mg_create_location_conf(ngx_conf_t *cf)
{
   mg_conf_location *conf;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_create_location_conf()", "mg_web: trace", 0);
#endif

   conf = ngx_pcalloc(cf->pool, sizeof(mg_conf_location));
   if (conf == NULL) {
      return NGX_CONF_ERROR;
   }
   conf->mg_enabled = 0;
   conf->mg_file_types[0] = '\0';
   conf->mg_config_file[0] = '\0';
   conf->mg_log_file[0] = '\0';
   conf->mg_thread_pool[0] = '\0';

#if defined(MG_API_TRACE)
   {
      char bufferx[256];
      sprintf(bufferx, "mg_create_location_conf: conf=%p", conf);
      mg_log_event(&debug_log, NULL, conf->mg_thread_pool, bufferx, 0);
   }
#endif

   return conf;
}


static char * mg_merge_location_conf(ngx_conf_t *cf, void *parent, void *child)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_merge_location_conf()", "mg_web: trace", 0);
#endif

    return NGX_CONF_OK;
}


ngx_int_t mg_master_init(ngx_log_t *log)
{

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_master_init()", "mg_web: trace", 0);
#endif

   return NGX_OK;
}


void mg_master_exit(ngx_cycle_t *cycle)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_master_exit()", "mg_web: trace", 0);
#endif

   return;
}


ngx_int_t mg_module_init(ngx_cycle_t *cycle)
{
   mg_conf_main  *pmain;

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_module_init()", "mg_web: trace", 0);
#endif

   pmain = (mg_conf_main *) ngx_http_cycle_get_module_main_conf(cycle, ngx_http_mg_web);

   if (pmain) {
      strcpy(mg_system.config_file, pmain->mg_config_file);
      strcpy(mg_system.log.log_file, pmain->mg_log_file);
   }

   return NGX_OK;
}


ngx_int_t mg_process_init(ngx_cycle_t *cycle)
{

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_process_init()", "mg_web: trace", 0);
   mg_log_event(&debug_log, NULL, mg_system.config_file, "mg_web: trace: mg_module_init(): config file", 0);
   mg_log_event(&debug_log, NULL, mg_system.log.log_file, "mg_web: trace: mg_module_init(): log file", 0);
#endif

   mg_worker_init();
   mg_ext_malloc = mg_malloc_nginx;
   mg_ext_realloc = mg_remalloc_nginx;
   mg_ext_free = mg_free_nginx;

   return NGX_OK;
}


void mg_process_exit(ngx_cycle_t *cycle)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_process_exit()", "mg_web: trace", 0);
#endif

   mg_worker_exit();

   return;
}


ngx_int_t mg_thread_init(ngx_cycle_t *cycle)
{

#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_process_init()", "mg_web: trace", 0);
#endif

   return NGX_OK;
}


void mg_thread_exit(ngx_cycle_t *cycle)
{
#if defined(MG_API_TRACE)
   mg_log_event(&debug_log, NULL, "mg_web: mg_thread_exit()", "mg_web: trace", 0);
#endif

   return;
}


int mg_check_file_type(char *mg_file_types_main, char *mg_file_types_server, char *mg_file_types_loc, char *type)
{
   int result;
   char buffer[32];

   strcpy(buffer, ".");
   strncpy(buffer + 1, type, 10);
   buffer[11] = '\0';
   strcat(buffer, ".");

   if (mg_file_types_loc[0] && (strstr(mg_file_types_loc, buffer) || strstr(mg_file_types_loc, ".*.")))
      result = 2;
   else if (mg_file_types_server[0] && (strstr(mg_file_types_server, buffer) || strstr(mg_file_types_server, ".*.")))
      result = 3;
   else
      result = 0;

   return result;
}


void * mg_malloc_nginx(void *pweb_server, unsigned long size)
{
   return ngx_pcalloc(((MGWEBNGINX *) pweb_server)->r->pool, size);
}


void * mg_remalloc_nginx(void *pweb_server, void *p, unsigned long size)
{
   return ngx_pcalloc(((MGWEBNGINX *) pweb_server)->r->pool, size);
}


int mg_free_nginx(void *pweb_server, void *p)
{
   return 0;
}


int mg_get_cgi_variable(MGWEB *pweb, char *name, char *pbuffer, int *pbuffer_size)
{
   int rc, len, len1, n, offset;
   unsigned int tn;
   char *pval;
   char buffer[256], vname[64];
   MGWEBNGINX *pwebnginx;
   ngx_list_part_t  *part;
   ngx_table_elt_t *hd;

#ifdef _WIN32
__try {
#endif

   pwebnginx = (MGWEBNGINX *) pweb->pweb_server;
   pval = NULL;
   len = 0;
   len1 = 0;

   if (!strcmp(name, "REQUEST_METHOD")) {
      pval = (char *) pwebnginx->r->method_name.data;
      len = (int) pwebnginx->r->method_name.len;
   }
   else if (!strcmp(name, "SCRIPT_NAME")) {
      pval = (char *) pwebnginx->r->uri.data;
      len = (int) pwebnginx->r->uri.len;
   }
   else if (!strcmp(name, "QUERY_STRING")) {
      pval = (char *) pwebnginx->r->args.data;
      len = (int) pwebnginx->r->args.len;
   }
   else if (!strcmp(name, "HTTP*")) {
      part = &(pwebnginx->r->headers_in.headers.part);

      if (part) {
         hd = part->elts;
         for (tn = 0 ;; tn++) {
            if (tn >= part->nelts) {
               if (part->next == NULL) {
                  break;
               }
               part = part->next;
               hd = part->elts;
               tn = 0;
            }

            if (hd[tn].key.data) {
               if (hd[tn].key.len > 60) {
                  continue;
               }
               strcpy(vname, "HTTP_");
               strncpy(vname + 5, (char *) hd[tn].key.data, (int) hd[tn].key.len);
               vname[hd[tn].key.len + 5] = '\0';
               mg_ucase((char *) vname);
               for (n = 0; vname[n]; n ++) {
                  if (vname[n] == '-')
                     vname[n] = '_';
               }
               offset = 0;
               if (!strcmp(vname + 5, "CONTENT_LENGTH") || !strcmp(vname + 5, "CONTENT_TYPE")) {
                  offset = 5;
               }
               mg_add_cgi_variable(pweb, vname + offset, (int) strlen(vname + offset), (char *) hd[tn].value.data, (int) hd[tn].value.len);
            }
         }
      }
      rc = MG_CGI_LIST;
	   return rc;
   }
   else if (!strcmp(name, "SERVER_PROTOCOL")) {
      pval = (char *) pwebnginx->r->http_protocol.data;
      len = (int) pwebnginx->r->http_protocol.len;
   }
   else if (!strcmp(name, "SERVER_SOFTWARE")) {
      sprintf(buffer, "%s mg_web/%s", (char *) NGINX_VER, (char *) DBX_VERSION);
      pval = (char *) buffer;
      len = (int) strlen(pval);
   }
   else if (!strcmp(name, "REMOTE_ADDR")) {
      if (pwebnginx->r->connection->addr_text.data && pwebnginx->r->connection->addr_text.len >= 1) {
         pval = (char *) pwebnginx->r->connection->addr_text.data;
         len = (int) pwebnginx->r->connection->addr_text.len;
      }
   }
   else if (!strcmp(name, "REMOTE_PORT")) {
      int port;
      struct sockaddr_in   *sin;
#if (NGX_HAVE_INET6)
      struct sockaddr_in6  *sin6;
#endif

      port = 0;
      switch (pwebnginx->r->connection->sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
         case AF_INET6:
            sin6 = (struct sockaddr_in6 *) pwebnginx->r->connection->sockaddr;
            port = ntohs(sin6->sin6_port);
            break;
#endif
         case AF_UNIX:
            port = 0;
            break;
         default: /* AF_INET */
            sin = (struct sockaddr_in *) pwebnginx->r->connection->sockaddr;
            port = ntohs(sin->sin_port);
            break;
      }
      if (port > 0 &&  port < 65536) {
         sprintf((char *) buffer, "%d", port);
         pval = (char *) buffer;
         len = (int) strlen(pval);
      }
   }
   else if (!strcmp(name, "REMOTE_HOST")) {
      if (pwebnginx->r->connection->addr_text.data && pwebnginx->r->connection->addr_text.len >= 1) {
         pval = (char *) pwebnginx->r->connection->addr_text.data;
         len = (int) pwebnginx->r->connection->addr_text.len;
      }
   }
   else if (!strcmp(name, "PATH_TRANSLATED")) {
      pval = (char *) pwebnginx->r->uri.data;
      len = (int) pwebnginx->r->uri.len;
   }
   else {
      part = &(pwebnginx->r->headers_in.headers.part);

      if (part) {
         hd = part->elts;
         for (tn = 0 ;; tn++) {
            if (tn >= part->nelts) {
               if (part->next == NULL) {
                  break;
               }
               part = part->next;
               hd = part->elts;
               tn = 0;
            }

            if (hd[tn].key.data) {
/*
               {
                  char bufferx[1024];
                  sprintf(bufferx, "name=%s (%d); value=%s (%d)", hd[tn].key.data, (int) hd[tn].key.len, hd[tn].value.data, (int) hd[tn].value.len);
                  mg_log_event(mg_system.plog, NULL, bufferx, "cgi list", 0);
               }
*/
               if (hd[tn].key.len > 60) {
                  continue;
               }
               strncpy(vname, (char *) hd[tn].key.data, (int) hd[tn].key.len);
               vname[hd[tn].key.len] = '\0';
               mg_ucase((char *) vname);
               for (n = 0; vname[n]; n ++) {
                  if (vname[n] == '-')
                     vname[n] = '_';
               }
               if (!strcmp((char *) vname, name)) {
                  pval = (char *) hd[tn].value.data;
                  len = (int) hd[tn].value.len;
                  break;
               }
               else if (!strcmp((char *) vname, name + 5)) {
                  pval = (char *) hd[tn].value.data;
                  len = (int) hd[tn].value.len;
                  break;
               }
               else if (!strcmp((char *) vname, "HOST") && !strncmp(name, "SERVER_", 7)) {
                  for (n = 0; n < (int) hd[tn].value.len; n ++) {
                     len = (int) hd[tn].value.len;
                     len1 = 0;
                     if (hd[tn].value.data[n] == ':') {
                        len1 = len - (n + 1);
                        len = n;
                        break;
                     }
                  }
                  if (!strcmp(name + 7, "NAME")) {
                     pval = (char *) hd[tn].value.data;
                     break;
                  }
                  else if (!strcmp(name + 7, "PORT")) {
                     if (len1) {
                        pval = (char *) hd[tn].value.data + (len + 1);
                        len = len1;
                     }
                     else {
                        pval = "80";
                        len = 2;
                     }
                     break;
                  }
               }
            }
         }
      }
   }

   rc = MG_CGI_SUCCESS;
   if (pval) {
      if (len < (*pbuffer_size)) {
         strncpy(pbuffer, pval, len);
         *pbuffer_size = len;
      }
      else {
         rc = MG_CGI_TOOLONG;
      }
   }
   else {
      pval = "";
      *pbuffer_size = 0;
      rc = MG_CGI_UNDEFINED;
   }

	return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char bufferx[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(bufferx, 255, "Exception caught in f:mg_get_server_variable: %x", code);
      mg_log_event(pweb->plog, NULL, bufferx, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif

}


int mg_client_read(MGWEB *pweb, unsigned char *pbuffer, int buffer_size)
{
   int result;

#ifdef _WIN32
__try {
#endif

   result = 0;

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
   ngx_chain_t *c;
   ngx_buf_t *b;
   MGWEBNGINX *pwebnginx;

#ifdef _WIN32
__try {
#endif
/*
   mg_log_buffer(pweb->plog, pweb, (char *) pbuffer, buffer_size, "Add buffer to chain", 0);
*/
   pwebnginx = (MGWEBNGINX *) pweb->pweb_server;

   c = ngx_pcalloc(pwebnginx->r->pool, sizeof(ngx_chain_t));
   if (c == NULL) {
      ngx_log_error(NGX_LOG_ERR, pwebnginx->r->connection->log, 0, "Failed to allocate response buffer.");
      ngx_http_finalize_request(pwebnginx->r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      return -1;
   }

   b = ngx_pcalloc(pwebnginx->r->pool, sizeof(ngx_buf_t));
   if (b == NULL) {
      ngx_log_error(NGX_LOG_ERR, pwebnginx->r->connection->log, 0, "Failed to allocate response buffer.");
      ngx_http_finalize_request(pwebnginx->r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      return -1;
   }

   b->pos = (u_char *) pbuffer; /* first position in memory of the data */
   b->last = (u_char *) (pbuffer + buffer_size); /* last position */

   b->memory = 1; /* content is in read-only memory */
   /* (i.e., filters should copy it rather than rewrite in place) */

   b->last_buf = 0; /* there might be more buffers in the request */

   c->buf = b;
   c->next = NULL;
   pwebnginx->output_buf_last = b;
   if (pwebnginx->output_head == NULL) {
      pwebnginx->output_head = c;
   }
   else {
      pwebnginx->output_last->next = c;
   }
   pwebnginx->output_last = c;

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_write_client: %x", code);
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
#ifdef _WIN32
__try {
#endif

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_suppress_headers: %x", code);
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
   int n, rc, status, size;
   char *pa, *pz, *p1, *p2;
   MGWEBNGINX *pwebnginx;
   ngx_table_elt_t *h;

#ifdef _WIN32
__try {
#endif

   pwebnginx = (MGWEBNGINX *) pweb->pweb_server;
   rc = NGX_OK;

/*
   {
      unsigned int tn;
      char vname[64];
      ngx_list_part_t  *part;
      ngx_table_elt_t *hd;

      part = &(pwebnginx->r->headers_out.headers.part);

      if (part) {
         hd = part->elts;
         for (tn = 0 ;; tn++) {
            if (tn >= part->nelts) {
               if (part->next == NULL) {
                  break;
               }
               part = part->next;
               hd = part->elts;
               tn = 0;
            }

            if (hd[tn].key.data) {
               if (hd[tn].key.len > 60) {
                  continue;
               }
               strncpy(vname, (char *) hd[tn].key.data, (int) hd[tn].key.len);
               vname[hd[tn].key.len] = '\0';

               {
                  char buffer[256];
                  sprintf(buffer, "header %s=%s;", (char *) vname, (char *) hd[tn].value.data);
                  mg_log_event(pweb->plog, NULL, buffer, "mgweb: headers: current list", 0);
               }

            }
         }
      }
   }
*/

   pa = pweb->response_headers;

   for (n = 0; ;n ++) {
      pz = strstr(pa, "\r\n");
      if (!pz)
         break;

      *pz = '\0';
      if (!strlen(pa))
         break;

      if (n == 0) {
         status = 200;
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
            }
         }
         pwebnginx->r->headers_out.status = status;
      }
      else {
         p1 = strstr(pa, ":");
         if (p1) {
            *p1 = '\0';
            p1 ++;
            while (*p1 == ' ')
               p1 ++;

            mg_ccase(pa);
            if (!strcmp(pa, "Content-Type")) {
               size = (int) strlen(p1);
               pwebnginx->r->headers_out.content_type.len = size;
               pwebnginx->r->headers_out.content_type.data = (u_char *) p1;
            }
            else if (!strcmp(pa, "Content-Length")) {
               pwebnginx->r->headers_out.content_length_n = pweb->response_clen;
            }
            else if (!strcmp(pa, "Connection")) {
               ;
            }
            else {
               h = (ngx_table_elt_t *) ngx_list_push(&(pwebnginx->r->headers_out.headers));
               if (h == NULL) {
                  return NGX_HTTP_INTERNAL_SERVER_ERROR;
               }
               h->hash = 1;
               ngx_str_set(&h->key, pa);
               h->key.len = (int) strlen(pa);
               ngx_str_set(&h->value, p1);
               h->value.len = (int) strlen(p1);
            }
         }
      }
      pa = (pz + 2);
   }

   rc = ngx_http_send_header(pwebnginx->r);

/*
   {
      char buffer[256];
      sprintf(buffer, "rc=%d; clen=%d; status=%d;", n, pweb->response_clen, (int) pwebnginx->r->headers_out.status);
      mg_log_event(pweb->plog, NULL, buffer, "mgweb: headers result", 0);
   }
*/

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_submit_headers: %x", code);
      mg_log_event(pweb->plog, NULL, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return NGX_ERROR;
}
#endif

}

