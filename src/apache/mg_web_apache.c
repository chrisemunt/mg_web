/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: Apache HTTP Gateway for InterSystems Cache/IRIS and YottaDB |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2023 MGateway Ltd                                     |
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

#include <stdio.h>
#include <time.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "util_script.h"
#include "http_request.h"
#include "http_connection.h"
#include "apr_strings.h"

#include "apr_thread_cond.h"
#include "apr_queue.h"

#include "apr_base64.h"
#include "apr_sha1.h"
#include "util_ebcdic.h"

#include "mg_web.h"
#include "mg_websocket.h"

#define MG_MAGIC_TYPE1        "application/x-mgweb"
#define MG_MAGIC_TYPE2        "text/mgweb"
#define MG_FILE_TYPES         ".mgw.mgweb."
#define MG_DEFAULT_TIMEOUT    300000
#define MG_RBUFFER_SIZE       1024

#define MG_WS_QUEUE_CAPACITY  16 /* capacity of queue used for communication between main thread and other threads */

#if !defined(APR_ARRAY_IDX)
#define APR_ARRAY_IDX(ary,i,type) (((type *)(ary)->elts)[i])
#endif
#if !defined(APR_ARRAY_PUSH)
#define APR_ARRAY_PUSH(ary,type) (*((type *)apr_array_push(ary)))
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct mg_conf {
   int     cmode;                /* Environment to which record applies (directory,  */
                                 /* server, or combination).                         */
#define CONF_MODE_SERVER    1
#define CONF_MODE_DIRECTORY 2
#define CONF_MODE_COMBO     3    /* Shouldn't ever happen.                           */

   int     local;                /* Boolean: was "Example" directive declared here?  */
   int     congenital;           /* Boolean: did we inherit an "Example"?            */
   short   mg_enabled;
   char    mg_config_file[256];
   char    mg_log_file[256];
   char    mg_file_types[128];
} mg_conf;


typedef struct tagMGWEBAPACHE {
   request_rec *        r;
   mg_conf *            sconf;
   mg_conf *            dconf;
   short                read_status;
   int                  read_eos;
   apr_bucket_brigade * read_bucket_brigade;
   apr_bucket *         read_bucket;
   apr_status_t         read_rv;
   const char *         read_data;
   apr_size_t           read_data_size;
   apr_size_t           read_data_offset;
   unsigned long        read_total;
   apr_bucket_brigade * write_bucket_brigade;

   /* Websocket support */
   apr_bucket_brigade * obb;
   apr_bucket_brigade * ibb;
   ap_filter_t *        of;
   apr_thread_mutex_t * wsmutex;
   apr_array_header_t * protocols;
   apr_os_thread_t      main_thread;
   apr_thread_cond_t *  write_cond;
   apr_queue_t *        queue;
   apr_pollset_t *      pollset;
   apr_pollfd_t         pollfd;
   apr_int32_t          pollcnt;
   apr_pollfd_t *       signaled;
   apr_status_t         rv;

} MGWEBAPACHE, *LPWEBAPACHE;


typedef struct tagMGWEBTABLE {
   MGWEB *pweb;
   char *name;
   char *value;
} MGWEBTABLE, *LPMGWEBTABLE;

#ifdef __cplusplus
}
#endif

/*
   Declare ourselves so the configuration routines can find and know us.
   We'll fill it in at the end of the module.
 */

module AP_MODULE_DECLARE_DATA mg_web_module;


static int           mg_handler                    (request_rec *r);
static const char *  mg_cmd                        (cmd_parms *cmd, void *dconf, const char *args);
static int           mg_init                       (apr_pool_t *pPool, apr_pool_t *pLog, apr_pool_t *pTemp, server_rec *s);
static void          mg_child_init                 (apr_pool_t *p, server_rec *s);
static apr_status_t  mg_child_exit                 (void *data);
static void *        mg_create_dir_conf            (apr_pool_t *p, char *dirspec);
static void *        mg_merge_dir_conf             (apr_pool_t *p, void *parent_conf, void *newloc_conf);
static void *        mg_create_server_conf         (apr_pool_t *p, server_rec *s);
static void *        mg_merge_server_conf          (apr_pool_t *p, void *server1_conf, void *server2_conf);
static void          mg_register_hooks             (apr_pool_t *p);
int                  mg_check_file_type            (MGWEBAPACHE *pwebapache, char *type);
int                  mg_parse_table                (void *rec, const char *key, const char *value);

static void          mg_websocket_handshake        (MGWEBAPACHE *pwebapache, const char *key);
static void          mg_websocket_parse_protocol   (MGWEBAPACHE *pwebapache, const char *sec_websocket_protocol);
static size_t        mg_websocket_protocol_count   (MGWEBAPACHE *pwebapache);
static const char *  mg_websocket_protocol_index   (MGWEBAPACHE *pwebapache, const size_t index);
static void          mg_websocket_protocol_set     (MGWEBAPACHE *pwebapache, const char *protocol);


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


/* Main request handler for MGWEB requests */

static int mg_handler(request_rec *r)
{
   int retval;
   long n1;
   unsigned int n;
   unsigned long request_clen;
   char *pval;
   unsigned char buffer[MG_RBUFFER_SIZE];
   apr_status_t rv;
   MGWEB *pweb;
   MGWEBAPACHE mgwebapache, *pwebapache;
   apr_table_t *e = r->subprocess_env;
   conn_rec *c = r->connection;
   struct ap_filter_t *cur = NULL;

#ifdef _WIN32
__try {
#endif

   if (!r) {
      return DECLINED;
   }

   if (!r->uri) {
      return DECLINED;
   }

   pwebapache = &mgwebapache;

   pwebapache->r = r;
   pwebapache->read_status = 0;
   pwebapache->read_eos = 0;
   pwebapache->read_bucket_brigade = NULL;
   pwebapache->read_bucket = NULL;
   pwebapache->read_rv = 0;
   pwebapache->read_data = 0;
   pwebapache->read_data_size = 0;
   pwebapache->read_data_offset = 0;
   pwebapache->read_total = 0;

   pwebapache->write_bucket_brigade = NULL;

   pwebapache->dconf = (mg_conf *) ap_get_module_config(r->per_dir_config, &mg_web_module);
   pwebapache->sconf = (mg_conf *) ap_get_module_config(r->server->module_config, &mg_web_module);
/*
   ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, pwebapache->r, "mg_web: sconf->mg_enabled=%d; dconf->mg_enabled=%d; conf=%s; log=%s; uri=%s; timeout=%d;", pwebapache->sconf->mg_enabled, pwebapache->dconf->mg_enabled, pwebapache->sconf->mg_config_file, pwebapache->sconf->mg_log_file, r->uri, (int) pwebapache->r->server->timeout/1000);
*/
   n = (unsigned int) strlen(r->uri);
   if (!n) {
      return DECLINED;
   }
   n1 = 0;

   for (n --; n > 0; n --) {
      if (r->uri[n] == '.') {
         for (n ++; r->uri[n]; n ++) {
            if (r->uri[n] == '/' || r->uri[n] == '\\')
               break;
            buffer[n1 ++] = r->uri[n];
            if (n1 > (MG_RBUFFER_SIZE - 3))
               break;
         }
         break;
      }
   }
   buffer[n1] = '\0';

   if (pwebapache->dconf->mg_enabled == 0) {

      if (!n1) {
         return DECLINED;
      }
      mg_lcase((char *) buffer);

      if (strcmp(r->handler, MG_MAGIC_TYPE1) && strcmp(r->handler, MG_MAGIC_TYPE2) && strcmp(r->handler, "mg-web-handler")) {
         if (!mg_check_file_type(pwebapache, (char *) buffer)) {
            short ok;
            char *p;

            ok = 0;
            p = strstr(r->uri, ".");
            if (p) {
               strncpy((char *) buffer, p + 1, 3);
               buffer[3] = '\0'; 
               mg_lcase((char *) buffer);
               if (mg_check_file_type(pwebapache, (char *) buffer)) {
                  ok = 1;
               }
            }
            if (!ok) {
               /* v2.4.28 */
               /* ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "mg_web: Bad request: %s", r->uri); */
               return DECLINED;
            }
         }
      }
   }

   if (!r->method) {
      ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "mg_web: Bad method for request: %s", r->uri);
      return DECLINED;
   }

   if (strlen(r->method) > 16) {
      ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "mg_web: Oversize method (> 16 Bytes) for request: %s", r->uri);
      return DECLINED;
   }


/*
   if ((retval = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)))
      return retval;
*/

   if ((retval = ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK)))
      return retval;

/*
   if ((retval = ap_setup_client_block(r, REQUEST_CHUNKED_PASS)))
      return retval;
*/

   ap_add_common_vars(r);
   ap_add_cgi_vars(r);

   request_clen = 0;
   pval = (char *) apr_table_get(r->subprocess_env, "CONTENT_LENGTH");
   if (pval) {
      request_clen = (unsigned long) strtol(pval, NULL, 10);
   }
   pweb = mg_obtain_request_memory((void *) pwebapache, (unsigned long) request_clen);
   if (!pweb) {
      return HTTP_INTERNAL_SERVER_ERROR;
   }

   pweb->http_version_major = r->proto_num / 1000;
   pweb->http_version_minor = r->proto_num % 1000;

/*
   {
      char bufferx[256];
      sprintf(bufferx, "HTTP raw version: proto_num=%d; protocol=%s; version=%d.%d;", r->proto_num, r->protocol, pweb->http_version_major, pweb->http_version_minor);
      mg_log_event(&(mg_system.log), NULL, bufferx, "mg_web: information", 0);
   }
*/

   pweb->pweb_server = (void *) pwebapache;
   pweb->evented = 0;
   pweb->wserver_chunks_response = 0;
   if (pweb->http_version_major == 2) {
      pweb->wserver_chunks_response = 1;
   }

   retval = mg_web((MGWEB *) pweb);

   if (pwebapache->write_bucket_brigade) {
      APR_BRIGADE_INSERT_TAIL(pwebapache->write_bucket_brigade, apr_bucket_eos_create(c->bucket_alloc));
      rv = ap_pass_brigade(r->output_filters, pwebapache->write_bucket_brigade);
   }

   mg_release_request_memory(pweb);

   return OK;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER ) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf(buffer, "Exception caught in f:mg_handler: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return HTTP_INTERNAL_SERVER_ERROR;
}
#endif
}


/* Process configuration directives (in httpd.conf) related to this module */

static const char *mg_cmd(cmd_parms *cmd, void *dconf, const char *args)
{
   int n;
   short context;
   char *pname;
   char buffer1[64], buffer2[256];
   mg_conf *conf;

    /*
     * Determine from our context into which record to put the entry.
     * cmd->path == NULL means we're in server-wide context; otherwise,
     * we're dealing with a per-directory setting.
     */

   if (cmd->path == NULL) {
      context = 0;
      conf = (mg_conf *) ap_get_module_config(cmd->server->module_config, &mg_web_module);
   }
   else {
      context = 1;
      conf = (mg_conf *) dconf;
   }

   pname = (char *) cmd->cmd->name;
   if (!pname || !args)
      return NULL;

   strncpy(buffer1, pname, 32);
   buffer1[32] = '\0';
   mg_lcase(buffer1);

   if (!strcmp(buffer1, "mgwebconfigfile")) {
      strncpy(conf->mg_config_file, args, 250);
      conf->mg_config_file[250] = '\0';
      strcpy(mg_system.config_file, conf->mg_config_file);
   }
   else if (!strcmp(buffer1, "mgweblogfile")) {
      strncpy(conf->mg_log_file, args, 250);
      conf->mg_log_file[250] = '\0';
      strcpy(mg_system.log.log_file, conf->mg_log_file);
   }
   else if (!strcmp(buffer1, "mgwebfiletypes")) {
      strcpy(buffer2, ".");
      strncpy(buffer2 + 1, args, 126);
      buffer2[126] = '\0';
      strcat(buffer2, ".");
      for (n = 0; buffer2[n]; n ++) {
         if (buffer2[n] == ' ')
            buffer2[n] = '.';
      }
      mg_lcase(buffer2);
      strcpy(conf->mg_file_types, buffer2);
   }
   else if (!strcmp(buffer1, "mgweb")) {
      strncpy(buffer2, args, 7);
      buffer2[7] = '\0';
      mg_lcase(buffer2);
      if (!strcmp(buffer2, "on"))
         conf->mg_enabled = 1;
   }

   return NULL;
}


/* 
 * This function is called during server initialization.  Any information
 * that needs to be recorded must be in static cells, since there's no
 * configuration record.
 *
 */


static int mg_init(apr_pool_t *pPool, apr_pool_t *pLog, apr_pool_t *pTemp, server_rec *s)
{
   char *sname = s->server_hostname;
   mg_conf *sconf;

   sconf = (mg_conf *) ap_get_module_config(s->module_config, &mg_web_module);

   return OK;

}



/* 
 * This function is called during server initialization when a heavy-weight
 * process (such as a child) is being initialised.  As with the
 * module-initialization function, any information that needs to be recorded
 * must be in static cells, since there's no configuration record.
 *
 * There is no return value.
 */


static void mg_child_init(apr_pool_t *p, server_rec *s)
{
   char *sname = s->server_hostname;

   apr_pool_cleanup_register(p, s, mg_child_exit, mg_child_exit);

   mg_worker_init();

   return;
}



/* 
 * This function is called when an heavy-weight process (such as a child) is
 * being run down or destroyed.  As with the child-initialization function,
 * any information that needs to be recorded must be in static cells, since
 * there's no configuration record.
 *
 */


static apr_status_t mg_child_exit(void *data)
{
   server_rec *s = data;
   char *sname = s->server_hostname;

   sname = (sname != NULL) ? sname : "";

   mg_worker_exit();

   return APR_SUCCESS;
}



/*
 * This function gets called to create up a per-directory configuration
 * record.  This will be called for the "default" server environment, and for
 * each directory for which the parser finds any of our directives applicable.
 * If a directory doesn't have any of our directives involved (i.e., they
 * aren't in the .htaccess file, or a <Location>, <Directory>, or related
 * block), this routine will *not* be called - the configuration for the
 * closest ancestor is used.
 *
 * The return value is a pointer to the created module-specific
 * structure.
 */

static void * mg_create_dir_conf(apr_pool_t *p, char *dirspec)
{
   mg_conf *conf;
   char *dname = dirspec;

   /*
    * Allocate the space for our record from the pool supplied.
    */

   conf = (mg_conf *) apr_pcalloc(p, sizeof(mg_conf));

   /*
    * Now fill in the defaults.  If there are any `parent' configuration
    * records, they'll get merged as part of a separate callback.
    */

   conf->local = 0;
   conf->congenital = 0;
   conf->cmode = CONF_MODE_DIRECTORY;

   dname = (dname != NULL) ? dname : "";
   conf->mg_enabled = 0;

   conf->mg_file_types[0] = '\0';

   return (void *) conf;
}



/*
 * This function gets called to merge two per-directory configuration
 * records.  This is typically done to cope with things like .htaccess files
 * or <Location> directives for directories that are beneath one for which a
 * configuration record was already created.  The routine has the
 * responsibility of creating a new record and merging the contents of the
 * other two into it appropriately.  If the module doesn't declare a merge
 * routine, the record for the closest ancestor location (that has one) is
 * used exclusively.
 *
 * The routine MUST NOT modify any of its arguments!
 *
 * The return value is a pointer to the created module-specific structure
 * containing the merged values.
 */

static void * mg_merge_dir_conf(apr_pool_t *p, void *parent_conf, void *newloc_conf)
{
   mg_conf *merged_conf = (mg_conf *) apr_pcalloc(p, sizeof(mg_conf));
   mg_conf *pconf = (mg_conf *) parent_conf;
   mg_conf *nconf = (mg_conf *) newloc_conf;

   /*
    * Some things get copied directly from the more-specific record, rather
    * than getting merged.
    */

   merged_conf->local = nconf->local;

   merged_conf->mg_enabled = nconf->mg_enabled;

   strcpy(merged_conf->mg_file_types, nconf->mg_file_types);
   strcpy(merged_conf->mg_config_file, nconf->mg_config_file);
   strcpy(merged_conf->mg_log_file, nconf->mg_log_file);

   /*
    * Others, like the setting of the `congenital' flag, get ORed in.  The
    * setting of that particular flag, for instance, is TRUE if it was ever
    * true anywhere in the upstream configuration.
    */

   merged_conf->congenital = (pconf->congenital | pconf->local);

   /*
    * If we're merging records for two different types of environment (server
    * and directory), mark the new record appropriately.  Otherwise, inherit
    * the current value.
    */

   merged_conf->cmode = (pconf->cmode == nconf->cmode) ? pconf->cmode : CONF_MODE_COMBO;

   return (void *) merged_conf;
}



/*
 * This function gets called to create a per-server configuration
 * record.  It will always be called for the "default" server.
 *
 * The return value is a pointer to the created module-specific
 * structure.
 */


static void * mg_create_server_conf(apr_pool_t *p, server_rec *s)
{
   mg_conf *cfg;
   char    *sname = s->server_hostname;

   /*
    * As with the mg_create_dir_conf() routine, we allocate and fill in an
    * empty record.
    */

   cfg = (mg_conf *) apr_pcalloc(p, sizeof(mg_conf));
   cfg->local = 0;
   cfg->congenital = 0;
   cfg->cmode = CONF_MODE_SERVER;

   sname = (sname != NULL) ? sname : "";

   return (void *) cfg;
}



/*
 * This function gets called to merge two per-server configuration
 * records.  This is typically done to cope with things like virtual hosts and
 * the default server configuration  The routine has the responsibility of
 * creating a new record and merging the contents of the other two into it
 * appropriately.  If the module doesn't declare a merge routine, the more
 * specific existing record is used exclusively.
 *
 * The routine MUST NOT modify any of its arguments!
 *
 * The return value is a pointer to the created module-specific structure
 * containing the merged values.
 */


static void *mg_merge_server_conf(apr_pool_t *p, void *server1_conf, void *server2_conf)
{
   mg_conf *merged_conf = (mg_conf *) apr_pcalloc(p, sizeof(mg_conf));
   mg_conf *s1conf = (mg_conf *) server1_conf;
   mg_conf *s2conf = (mg_conf *) server2_conf;

   /*
    * Our inheritance rules are our own, and part of our module's semantics.
    * Basically, just note whence we came.
    */

   merged_conf->cmode = (s1conf->cmode == s2conf->cmode) ? s1conf->cmode : CONF_MODE_COMBO;
   merged_conf->local = s2conf->local;
   merged_conf->congenital = (s1conf->congenital | s1conf->local);

   return (void *) merged_conf;
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Which functions are responsible for which hooks in the server.           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* 
 * Each function our module provides to handle a particular hook is
 * specified here.  The functions are registered using 
 * ap_hook_foo(name, predecessors, successors, position)
 * where foo is the name of the hook.
 *
 * The args are as follows:
 * name         -> the name of the function to call.
 * predecessors -> a list of modules whose calls to this hook must be
 *                 invoked before this module.
 * successors   -> a list of modules whose calls to this hook must be
 *                 invoked after this module.
 * position     -> The relative position of this module.  One of
 *                 APR_HOOK_FIRST, APR_HOOK_MIDDLE, or APR_HOOK_LAST.
 *                 Most modules will use APR_HOOK_MIDDLE.  If multiple
 *                 modules use the same relative position, Apache will
 *                 determine which to call first.
 *                 If your module relies on another module to run first,
 *                 or another module running after yours, use the 
 *                 predecessors and/or successors.
 *
 * The number in brackets indicates the order in which the routine is called
 * during request processing.  Note that not all routines are necessarily
 * called (such as if a resource doesn't have access restrictions).
 * The actual delivery of content to the browser [9] is not handled by
 * a hook; see the handler declarations below.
 */

static void mg_register_hooks(apr_pool_t *p)
{

   ap_hook_post_config(mg_init, NULL, NULL, APR_HOOK_MIDDLE);
   ap_hook_child_init(mg_child_init, NULL, NULL, APR_HOOK_MIDDLE);
   ap_hook_handler(mg_handler, NULL, NULL, APR_HOOK_MIDDLE);

   return;
}



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* All of the routines have been declared now.  Here's the list of          */
/* directives specific to our module, and information about where they      */
/* may appear and how the command parser should pass them to us for         */
/* processing.  Note that care must be taken to ensure that there are NO    */
/* collisions of directive names between modules.                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* 
 * List of directives specific to our module.
 */


static const command_rec mg_web_commands [] = {

   AP_INIT_RAW_ARGS("MGWEBConfigFile", mg_cmd, NULL, OR_FILEINFO, "MGWEB Configuration File"),

   AP_INIT_RAW_ARGS("MGWEBLogFile", mg_cmd, NULL, OR_FILEINFO, "MGWEB Configuration File"),

   AP_INIT_RAW_ARGS("MGWEBFileTypes", mg_cmd, NULL, OR_FILEINFO, "List of file types (by extension) to be processed by MGWEB"),

   AP_INIT_RAW_ARGS("MGWEB", mg_cmd, NULL, OR_FILEINFO, "Set to 'On' to enable the entire Location to be processed by MGWEB"),

   {NULL}
};



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Finally, the list of callback routines and data structures that          */
/* provide the hooks into our module from the other parts of the server.    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* 
 * Module definition for configuration.  If a particular callback is not
 * needed, replace its routine name below with the word NULL.
 *
 * The number in brackets indicates the order in which the routine is called
 * during request processing.  Note that not all routines are necessarily
 * called (such as if a resource doesn't have access restrictions).
 */


module AP_MODULE_DECLARE_DATA mg_web_module = {

   STANDARD20_MODULE_STUFF,
   mg_create_dir_conf,        /* per-directory config creater */
   mg_merge_dir_conf,         /* dir config merger - default is to override */
   mg_create_server_conf,     /* server config creator */
   mg_merge_server_conf,      /* server config merger */
   mg_web_commands,           /* command table */
   mg_register_hooks,         /* set up other request processing hooks */
};



/* Functions responsible for communicating with the NSD module follow */

int mg_check_file_type(MGWEBAPACHE *pwebapache, char *type)
{
   int result;
   char buffer[32];

   strcpy(buffer, ".");
   strncpy(buffer + 1, type, 10);
   buffer[11] = '\0';
   strcat(buffer, ".");

   if (strstr(MG_FILE_TYPES, buffer))
      result = 1;
   else if (pwebapache->dconf && pwebapache->dconf->mg_file_types[0] && (strstr(pwebapache->dconf->mg_file_types, buffer) || strstr(pwebapache->dconf->mg_file_types, ".*.")))
      result = 2;
   else if (pwebapache->sconf && pwebapache->sconf->mg_file_types[0] && (strstr(pwebapache->sconf->mg_file_types, buffer) || strstr(pwebapache->dconf->mg_file_types, ".*.")))
      result = 3;
   else
      result = 0;
/*
   ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, pwebapache->r, "mg_web: mg_check_file_type: result=%d; pwebapache->dconf->mg_file_types=%s; pwebapache->dconf->mg_file_types=%s; buffer=%s", result, pwebapache->dconf->mg_file_types, pwebapache->sconf->mg_file_types, buffer);
*/
   return result;
}


int mg_parse_table(void *rec, const char *key, const char *value)
{
   int n, offset;
   char buffer[128];
   MGWEBTABLE * ptable;
   MGWEB *pweb;
   MGWEBAPACHE *pwebapache;

   if (!rec || !key || !value) {
      return 0;
   }

   ptable = (MGWEBTABLE *) rec;
   pweb = (MGWEB *) ptable->pweb;
   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   if (ptable->name) {
      strncpy(buffer, (char *) key, 120);
      buffer[120] = '\0';
      mg_lcase((char *) buffer);

      if (!strcmp(ptable->name, buffer)) {
         ptable->value = (char * ) value;
      }
   }
   else {
      strcpy(buffer, "HTTP_");
      strncpy(buffer + 5, (char *) key, 110);
      buffer[115] = '\0';
      for (n = 5; buffer[n]; n ++) {
         if (buffer[n] == '-')
            buffer[n] = '_';
         else if (((int) buffer[n]) >= 97)
            buffer[n] = (char) (buffer[n] - 32);
      }
      if (!strcmp(buffer + 5, "CONTENT_LENGTH") || !strcmp(buffer + 5, "CONTENT_TYPE"))
         offset = 5;
      else
         offset = 0;
      mg_add_cgi_variable(pweb, (char *) buffer + offset, (int) strlen(buffer + offset), (char *) value, (int) strlen(value));
   }

   return 1;
}


int mg_get_cgi_variable(MGWEB *pweb, char *name, char *pbuffer, int *pbuffer_size)
{
   int rc, len;
   char *pval, *pval1;
   const char *p;
   char buffer[256];
   apr_table_t *e_subprocess_env;
   MGWEBAPACHE *pwebapache;
   MGWEBTABLE mgwebtable;

#ifdef _WIN32
__try {
#endif

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   pval = NULL;
   pval1 = NULL;
   e_subprocess_env = pwebapache->r->subprocess_env;

   if (!strcasecmp(name, "HTTP*")) {
      mgwebtable.pweb = pweb;
      mgwebtable.name = NULL;
      mgwebtable.value = NULL;
      apr_table_do(mg_parse_table, (void *) &mgwebtable, pwebapache->r->headers_in, NULL);
      rc = MG_CGI_LIST;
      return rc;
   }
   else if (!strcasecmp(name, "AUTH_PASSWORD")) {
      if ((rc = ap_get_basic_auth_pw(pwebapache->r, &p)) == OK)
         pval = (char *) p;
   }
   else if (!strcasecmp(name, "HTTP_AUTHORIZATION")) {
      mgwebtable.pweb = pweb;
      mgwebtable.name = "authorization";
      mgwebtable.value = NULL;

      apr_table_do(mg_parse_table, (void *) &mgwebtable, pwebapache->r->headers_in, NULL);
      pval = mgwebtable.value;
   }
   else if (!strcasecmp(name, "SERVER_PORT_SECURE")) {

	   /* Apache doesn't support secure requests inherently, so
	    * we have no way of knowing. We'll be conservative, and say
	    * all requests are insecure.
	    */
      pval = "0";
   }
   else if (!strcasecmp(name, "URL") || !strcasecmp(name, "SCRIPT_NAME")) {
	   pval = pwebapache->r->uri;
   }
   else if (!strcasecmp(name, "PATH_TRANSLATED")) {

	   pval = (char *) apr_table_get(e_subprocess_env, "DOCUMENT_ROOT");
      if (pval) {
         if (pwebapache->r->uri) {
            pval1 = pwebapache->r->uri;
         }
      }
      else {
   	   pval = (char *) apr_table_get(e_subprocess_env, name);
      }
   }
   else if (!strcasecmp(name, "SERVER_SOFTWARE")) {
	   pval = (char *) apr_table_get(e_subprocess_env, name);
      if (pval && strlen(pval) < 200) {
         sprintf(buffer, "%s mg_web/%s", pval, DBX_VERSION);
         pval = buffer;
      }
   }
   else {
	   pval = (char *) apr_table_get(e_subprocess_env, name);
   }

   rc = MG_CGI_SUCCESS;
   if (pval) {
      len = (int) strlen(pval);
      if (len < (*pbuffer_size)) {
         strcpy(pbuffer, pval);
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
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_get_server_variable: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
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
   short done;
   int result, ptr, bytes_available;
   apr_size_t total;
   MGWEBAPACHE *pwebapache;

#ifdef _WIN32
__try {
#endif

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   total = 0;
   result = 0;

   if (pwebapache->read_status == 0) {
      pwebapache->read_bucket_brigade = apr_brigade_create(pwebapache->r->pool, pwebapache->r->connection->bucket_alloc);
      pwebapache->read_eos = 0;

      pwebapache->read_data = NULL;
      pwebapache->read_data_size = 0;
      pwebapache->read_data_offset = 0;

      pwebapache->read_total = 0;

      pwebapache->read_status = 1;
   }

   ptr = 0;

   if (pwebapache->read_data) {
      bytes_available = (int) (pwebapache->read_data_size - pwebapache->read_data_offset);
      if (bytes_available >= buffer_size) {
         memcpy((void *) pbuffer, (void *) (pwebapache->read_data + pwebapache->read_data_offset), sizeof(char) * buffer_size);

         result = buffer_size;
         pwebapache->read_data_offset += buffer_size;
         return result;
      }
      else {

         memcpy((void *) pbuffer, (void *) (pwebapache->read_data + pwebapache->read_data_offset), sizeof(char) * bytes_available);

         result = bytes_available;
         ptr += bytes_available;
         pwebapache->read_data = NULL;
         pwebapache->read_data_size = 0;
         pwebapache->read_data_offset = 0;
      }
   }

   for (; !(pwebapache->read_eos); ) {

      done = 0;

      if (pwebapache->read_status == 1) {
         pwebapache->read_rv = ap_get_brigade(pwebapache->r->input_filters, pwebapache->read_bucket_brigade, AP_MODE_READBYTES, APR_BLOCK_READ, HUGE_STRING_LEN);

         if (pwebapache->read_rv != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, pwebapache->read_rv, pwebapache->r, "mg_web: Error reading request entity data");
            break;
         }

         pwebapache->read_bucket = APR_BRIGADE_FIRST(pwebapache->read_bucket_brigade);

         pwebapache->read_status = 2;
      }

      for (;;) {

         apr_size_t total;

         if (pwebapache->read_bucket == APR_BRIGADE_SENTINEL(pwebapache->read_bucket_brigade)) {
            pwebapache->read_status = 1;
            break;
         }

         if (APR_BUCKET_IS_EOS(pwebapache->read_bucket)) {
            pwebapache->read_status = 1;
            pwebapache->read_eos = 1;
            break;
         }

         if (!APR_BUCKET_IS_FLUSH(pwebapache->read_bucket)) {

            apr_bucket_read(pwebapache->read_bucket, &(pwebapache->read_data), &(pwebapache->read_data_size), APR_BLOCK_READ);
            pwebapache->read_data_offset = 0;

            pwebapache->read_total += (unsigned long) pwebapache->read_data_size;
            /* Usually 8K read */

            total = buffer_size - result;

            if (pwebapache->read_data_size > 0) {

               if (pwebapache->read_data_size >= total) {
                  memcpy((void *) (pbuffer + ptr), (void *) pwebapache->read_data, sizeof(char) * total);
                  ptr += (int) total;
                  result += (int) total;
                  pwebapache->read_data_offset += total;
                  done = 1;
               }
               else {

                  memcpy((void *) (pbuffer + ptr), (void *) pwebapache->read_data, sizeof(char) * pwebapache->read_data_size);

                  ptr += (int) pwebapache->read_data_size;
                  result += (int) pwebapache->read_data_size;
                  pwebapache->read_data = NULL;
               }

            }

         }

         pwebapache->read_bucket = APR_BUCKET_NEXT(pwebapache->read_bucket);

         if (done)
            break;
      }

      if (done)
         break;

      apr_brigade_cleanup(pwebapache->read_bucket_brigade);

      if (pwebapache->read_eos)
         break;
   }


   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_read_client: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
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
   apr_bucket *b;
   MGWEBAPACHE *pwebapache;

#ifdef _WIN32
__try {
#endif

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
/*
   mg_log_buffer(pweb->plog, pweb, pbuffer, buffer_size, "mgweb: response", 0);
*/
   if (!pwebapache->write_bucket_brigade) {
      pwebapache->write_bucket_brigade = apr_brigade_create(pwebapache->r->pool, pwebapache->r->connection->bucket_alloc);
      apr_brigade_cleanup(pwebapache->write_bucket_brigade);
   }

   b = apr_bucket_heap_create((char *) pbuffer, buffer_size, NULL, pwebapache->write_bucket_brigade->bucket_alloc);

   APR_BRIGADE_INSERT_TAIL(pwebapache->write_bucket_brigade, b);

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_write_client: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
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
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
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
   int n, status, ctsize, cookie_no;
   char *pa, *pz, *p1, *p2, *ct, *reason;
   MGWEBAPACHE *pwebapache;

#ifdef _WIN32
__try {
#endif

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   cookie_no = 0;
/*
   mg_log_buffer(pweb->plog, pweb, pweb->response_headers, (int) strlen(pweb->response_headers), "mgweb: headers", 0);
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
         pwebapache->r->status = status;
      }
      else {
         p1 = strstr(pa, ":");
         if (p1) {
            *p1 = '\0';
            p1 ++;
            while (*p1 == ' ')
               p1 ++;

            mg_ccase(pa);

            if (!strcmp(p1, "Content-Type")) {
               ctsize = (int) strlen(p1) + 1;
               ct = (char *) apr_pcalloc(pwebapache->r->pool, ctsize);
               strcpy(ct, p1);
               pwebapache->r->content_type = ct;
            }
            else {
               if (!strcmp(pa, "Set-Cookie")) {
                  if (!cookie_no) {
                     apr_table_set(pwebapache->r->headers_out, pa, p1);
                  }
                  else {
                     apr_table_add(pwebapache->r->headers_out, pa, p1);
                  }
                  cookie_no ++;
               }
               else {
                  apr_table_set(pwebapache->r->headers_out, pa, p1);
               }
            }
         }
      }
      pa = (pz + 2);
   }

   return 0;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf_s(buffer, 255, "Exception caught in f:mg_submit_headers: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif

}


static void mg_websocket_handshake(MGWEBAPACHE *pwebapache, const char *key)
{
   apr_byte_t response[32];
   apr_byte_t digest[APR_SHA1_DIGESTSIZE];
   apr_sha1_ctx_t context;
   int len;

   apr_sha1_init(&context);
   apr_sha1_update(&context, key, (int) strlen(key));
   apr_sha1_update(&context, MG_WS_WEBSOCKET_GUID, MG_WS_WEBSOCKET_GUID_LEN);
   apr_sha1_final(digest, &context);

   len = apr_base64_encode_binary((char *)response, digest, sizeof(digest));
   response[len] = '\0';

   apr_table_setn(pwebapache->r->headers_out, "Sec-WebSocket-Accept", apr_pstrdup(pwebapache->r->pool, (const char *)response));
   return;
}


static void mg_websocket_parse_protocol(MGWEBAPACHE *pwebapache, const char *sec_websocket_protocol)
{
   apr_array_header_t *protocols = apr_array_make(pwebapache->r->pool, 1, sizeof(char *));
   char *protocol_state = NULL;
   char *protocol = apr_strtok(apr_pstrdup(pwebapache->r->pool, sec_websocket_protocol), ", \t", &protocol_state);

   while (protocol != NULL) {
      APR_ARRAY_PUSH(protocols, char *) = protocol;
      protocol = apr_strtok(NULL, ", \t", &protocol_state);
   }
   if (!apr_is_empty_array(protocols)) {
      pwebapache->protocols = protocols;
   }
   return;
}


static size_t mg_websocket_protocol_count(MGWEBAPACHE *pwebapache)
{
   size_t count = 0;

   if ((pwebapache != NULL) && (pwebapache->protocols != NULL) && !apr_is_empty_array(pwebapache->protocols)) {
      count = (size_t) pwebapache->protocols->nelts;
   }
   return count;
}


static const char * mg_websocket_protocol_index(MGWEBAPACHE *pwebapache, const size_t index)
{
   if (index < mg_websocket_protocol_count(pwebapache)) {
      return APR_ARRAY_IDX(pwebapache->protocols, index, char *);
   }
   return NULL;
}


static void mg_websocket_protocol_set(MGWEBAPACHE *pwebapache, const char *protocol)
{
   if ((pwebapache != NULL) && (protocol != NULL)) {

      if (pwebapache->r != NULL) {
         apr_table_setn(pwebapache->r->headers_out, "Sec-WebSocket-Protocol", apr_pstrdup(pwebapache->r->pool, protocol));
      }
   }
   return;
}


int mg_websocket_init(MGWEB *pweb)
{
   ap_filter_t *input_filter;
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   pwebapache->obb = NULL;
   pwebapache->wsmutex = NULL;
   pwebapache->protocols = NULL;
   pwebapache->main_thread = apr_os_thread_current();
   pwebapache->write_cond = NULL;
   pwebapache->pollset = NULL;
   pwebapache->queue = NULL;

   /* Remove the HTTP input filter. */
   for (input_filter = pwebapache->r->input_filters; input_filter != NULL; input_filter = input_filter->next) {
      if ((input_filter->frec != NULL) && (input_filter->frec->name != NULL) && !strcasecmp(input_filter->frec->name, "http_in")) {
         ap_remove_input_filter(input_filter);
         break;
      }
   }

   /* Remove the reqtimeout input filter. */
   for (input_filter = pwebapache->r->input_filters; input_filter != NULL; input_filter = input_filter->next) {
      if ((input_filter->frec != NULL) && (input_filter->frec->name != NULL) && !strcasecmp(input_filter->frec->name, "reqtimeout")) {
         ap_remove_input_filter(input_filter);
         break;
      }
   }

   pwebapache->r->input_filters = pwebapache->r->connection->input_filters;
   pwebapache->r->proto_input_filters = pwebapache->r->connection->input_filters;

   apr_table_clear(pwebapache->r->headers_out);
   apr_table_setn(pwebapache->r->headers_out, "Upgrade", "websocket");
   apr_table_setn(pwebapache->r->headers_out, "Connection", "Upgrade");

   /* Set the expected acceptance response */
   mg_websocket_handshake(pwebapache, pweb->pwsock->sec_websocket_key);

   /* Handle the WebSocket protocol */
   if (pweb->pwsock->sec_websocket_protocol[0] != '\0') {
      /* Parse the WebSocket protocol entry */
      mg_websocket_parse_protocol(pwebapache, pweb->pwsock->sec_websocket_protocol);

      if (mg_websocket_protocol_count(pwebapache) > 0) {
         /*
            Default to using the first protocol in the list
         */
         mg_websocket_protocol_set(pwebapache, mg_websocket_protocol_index(pwebapache, 0));
      }
   }

   mg_websocket_create_lock(pweb);
   apr_thread_cond_create(&(pwebapache->write_cond), pwebapache->r->pool);
   mg_websocket_lock(pweb);

   /* Now that the connection has been established, disable the socket timeout */
   apr_socket_timeout_set(ap_get_conn_socket(pwebapache->r->connection), -1);

   /* Set response status code and status line */
   pwebapache->r->status = HTTP_SWITCHING_PROTOCOLS;
   pwebapache->r->status_line = ap_get_status_line(pwebapache->r->status);

   /* Send the headers */
   ap_send_interim_response(pwebapache->r, 1);

   pweb->pwsock->status = MG_WEBSOCKET_HEADERS_SENT;

   return 0;
}


int mg_websocket_create_lock(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   apr_thread_mutex_create(&(pwebapache->wsmutex), APR_THREAD_MUTEX_DEFAULT, pwebapache->r->pool);

   return 0;
}


int mg_websocket_destroy_lock(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   apr_thread_mutex_destroy(pwebapache->wsmutex);

   return 0;
}


int mg_websocket_lock(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   apr_thread_mutex_lock(pwebapache->wsmutex);

   return 0;
}


int mg_websocket_unlock(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   apr_thread_mutex_unlock(pwebapache->wsmutex);

   return 0;
}



int mg_websocket_frame_init(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   if ((pwebapache->obb = apr_brigade_create(pwebapache->r->pool, pwebapache->r->connection->bucket_alloc)) != NULL &&
      (pwebapache->ibb = apr_brigade_create(pwebapache->r->pool, pwebapache->r->connection->bucket_alloc)) != NULL &&
      apr_pollset_create(&(pwebapache->pollset), 1, pwebapache->r->pool, APR_POLLSET_WAKEABLE) == APR_SUCCESS &&
      apr_queue_create(&(pwebapache->queue), MG_WS_QUEUE_CAPACITY, pwebapache->r->pool) == APR_SUCCESS) {

      memset(&(pwebapache->pollfd), 0, sizeof(pwebapache->pollfd));
      pwebapache->pollfd.p = pwebapache->r->pool;
      pwebapache->pollfd.desc_type = APR_POLL_SOCKET;
      pwebapache->pollfd.reqevents = APR_POLLIN;
      pwebapache->pollfd.desc.s = ap_get_conn_socket(pwebapache->r->connection);
      apr_pollset_add(pwebapache->pollset, &(pwebapache->pollfd));

      mg_websocket_unlock(pweb);

      return CACHE_SUCCESS;
   }
   else {
      return CACHE_FAILURE;
   }
}


int mg_websocket_frame_read(MGWEB *pweb, MGWSRSTATE *pread_state)
{
   int timeout, work_done;
   apr_status_t rv;
   MGWSMESS *msg;
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   work_done = 0;

   /* check for incoming data from client */
   pweb->pwsock->block_size = mg_websocket_read_block(pweb, (char *) pweb->pwsock->block, sizeof(pweb->pwsock->block));

   if (pwebapache->rv == APR_SUCCESS) {
      mg_websocket_incoming_frame(pweb, pread_state, (char *) pweb->pwsock->block, pweb->pwsock->block_size);
      work_done = 1;
   }
   else if (!APR_STATUS_IS_EAGAIN(pwebapache->rv)) {
      pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
      pread_state->status_code = MG_WS_STATUS_CODE_PROTOCOL_ERROR;
      return -1;
   }

   /* check for outgoing data to client */
   rv = apr_queue_trypop(pwebapache->queue, (void **) &msg);
   if (rv == APR_SUCCESS) {
      mg_websocket_lock(pweb);

      msg->written = mg_websocket_write_block(pweb, (int) msg->type, (unsigned char *) msg->buffer, (size_t) msg->buffer_size);
      msg->done = 1;
      apr_thread_cond_signal(pwebapache->write_cond);
      mg_websocket_unlock(pweb);
      work_done = 1;
   }
   else if (!APR_STATUS_IS_EAGAIN(rv)) {
      pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
      pread_state->status_code = MG_WS_STATUS_CODE_INTERNAL_ERROR;
      return -2;
   }

   timeout = work_done ? 0 : -1;
   rv = apr_pollset_poll(pwebapache->pollset, timeout, &(pwebapache->pollcnt), (const apr_pollfd_t **) &(pwebapache->signaled));
   if (rv != APR_SUCCESS && !APR_STATUS_IS_EINTR(rv) && !APR_STATUS_IS_TIMEUP(rv)) {
      pread_state->framing_state = MG_WS_DATA_FRAMING_CLOSE;
      pread_state->status_code = MG_WS_STATUS_CODE_INTERNAL_ERROR;
      return -3;
   }

   return CACHE_SUCCESS;
}


int mg_websocket_frame_exit(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   /* We are done with the bucket brigades, pollset, and queue */
   mg_websocket_lock(pweb);

   apr_brigade_destroy(pwebapache->obb);
   apr_brigade_destroy(pwebapache->ibb);
   pwebapache->obb = NULL;
   pwebapache->ibb = NULL;

   apr_pollset_destroy(pwebapache->pollset);
   pwebapache->pollset = NULL;

   apr_queue_term(pwebapache->queue);
   pwebapache->queue = NULL;

   return 0;
}


size_t mg_websocket_read_block(MGWEB *pweb, char *buffer, size_t bufsiz)
{
   apr_size_t readbufsiz;
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   readbufsiz = 0;
   if ((pwebapache->rv = ap_get_brigade(pwebapache->r->input_filters, pwebapache->ibb, AP_MODE_READBYTES, APR_NONBLOCK_READ, (apr_size_t) bufsiz)) == APR_SUCCESS) {
      if ((pwebapache->rv = apr_brigade_flatten(pwebapache->ibb, buffer, (apr_size_t *) &bufsiz)) == APR_SUCCESS) {
         readbufsiz = (apr_size_t) bufsiz;
         if (readbufsiz == 0) {
            /*
               Some input filters return APR_SUCCESS on a non-blocking read when no data are available.
               This is not ideal behavior, but we must account for it by changing the status to APR_EAGAIN.
            */
            pwebapache->rv = APR_EAGAIN;
         }
      }
      apr_brigade_cleanup(pwebapache->ibb);
   }
/*
   if (readbufsiz > 0) {
      char bufferx[256];
      sprintf(bufferx, "WEBSOCKET READ len=%d;", readbufsiz);
      mg_log_buffer(pweb->plog, pweb, buffer, readbufsiz, bufferx, 0);
   }
*/
   return (size_t) readbufsiz;
}


size_t mg_websocket_read(MGWEB *pweb, char *buffer, size_t bufsiz)
{
   return 0;
}


size_t mg_websocket_queue_block(MGWEB *pweb, int type, unsigned char *buffer, size_t buffer_size, short locked)
{
   apr_status_t rv;
   size_t written;
   MGWSMESS msg = { type, buffer, buffer_size, 0, 0 };
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;
   written = 0;

   if (!pweb->pwsock) {
      return 0;
   }

   if (!locked) {
      mg_websocket_lock(pweb);
   }
   if (type == MG_WS_MESSAGE_TYPE_CLOSE) {
      if (pweb->pwsock->status == MG_WEBSOCKET_CLOSED) {
         goto mg_websocket_queue_block_exit;
      }
      else {
         pweb->pwsock->status = MG_WEBSOCKET_CLOSED;
      }
   }

   if (apr_os_thread_equal(apr_os_thread_current(), pwebapache->main_thread)) {
      /* This is the main thread. It's safe to write messages directly. */
      written = mg_websocket_write_block(pweb, type, buffer, buffer_size);
   }
   else if (pwebapache->pollset && pwebapache->queue && !pweb->pwsock->closing) {
      /* Dispatch this message to the main thread. */
      msg.type = type;
      msg.buffer = buffer;
      msg.buffer_size = buffer_size;
      msg.written = 0;
      msg.done = 0;
      rv = apr_queue_push(pwebapache->queue, &msg);
      if (rv != APR_SUCCESS) {
         goto mg_websocket_queue_block_exit;
      }
      apr_pollset_wakeup(pwebapache->pollset);

      while (pwebapache->pollset && pwebapache->queue && !msg.done && !pweb->pwsock->closing) {
         apr_thread_cond_wait(pwebapache->write_cond, pwebapache->wsmutex);
      }

      if (msg.done) {
         written = msg.written;
      }
   }

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
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   pos = 0;
   written = 0;
   payload_length = (mg_uint64_t) ((buffer != NULL) ? buffer_size : 0);
/*
{
   char bufferx[256];
   sprintf(bufferx, "websocket mg_websocket_write_block payload_length=%llu; pos=%d; obb=%p; pweb->pwsock->closing=%d", payload_length, pos, pwebapache->obb, pweb->pwsock->closing);
   mg_log_buffer(pweb->plog, pweb, header, pos, bufferx, 0);
}
*/

   if ((pwebapache->r != NULL) && (pwebapache->obb != NULL) && !pweb->pwsock->closing) {
      pwebapache->of = pwebapache->r->connection->output_filters;

      pos = mg_websocket_create_header(pweb, type, header, payload_length);
/*
{
   char bufferx[256];
   sprintf(bufferx, "websocket mg_websocket_write_block payload_length=%llu; pos=%d", payload_length, pos);
   mg_log_buffer(pweb->plog, pweb, header, pos, bufferx, 0);
}
*/
      mg_websocket_write(pweb, (char *) header, (int) pos); /* Header */

      if (payload_length > 0) {
         if (mg_websocket_write(pweb, (char *) buffer, (int) buffer_size) > 0) { /* Payload Data */
            written = buffer_size;
         }
      }

      if (ap_fflush(pwebapache->of, pwebapache->obb) != APR_SUCCESS) {
         written = 0;
      }
   }

   return written;
}


int mg_websocket_write(MGWEB *pweb, char *buffer, int len)
{
   int rc;
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

/*
   {
      char bufferx[256];
      sprintf(bufferx, "WEBSOCKET WRITE len=%d;", len);
      mg_log_buffer(pweb->plog, pweb, buffer, len, bufferx, 0);
   }
*/

   if (ap_fwrite(pwebapache->of, pwebapache->obb, (const char *) buffer, (apr_size_t) len) == APR_SUCCESS) { /* Payload Data */
      rc = len;
   }

   return rc;
}


int mg_websocket_exit(MGWEB *pweb)
{
   MGWEBAPACHE *pwebapache;

   pwebapache = (MGWEBAPACHE *) pweb->pweb_server;

   mg_websocket_unlock(pweb);

   pwebapache->r->connection->keepalive = AP_CONN_CLOSE;

   /* Close the connection */
   ap_lingering_close(pwebapache->r->connection);

   apr_thread_cond_destroy(pwebapache->write_cond);
   mg_websocket_destroy_lock(pweb);

   return 0;
}
