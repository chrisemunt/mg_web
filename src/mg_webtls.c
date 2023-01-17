/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2023 M/Gateway Developments Ltd,                      |
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
#include "mg_webtls.h"

#if DBX_WITH_TLS >= 1

static DBXCRYPTSO *  mg_crypt_so       = NULL;
static DBXTLSSO *    mg_tls_so         = NULL;


int mgtls_open_session(MGWEB *pweb)
{
   int rc;
   char *subject, *issuer;
   char message_buffer[4096];
   SSL_CTX *ctx;
   SSL *ssl;
   X509 *server_cert;
   SSL_METHOD *method;
   DBXTLSCON *ptlscon;

#ifdef _WIN32
__try {
#endif

   rc = CACHE_SUCCESS;

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "cipher_list=%s; cert_file=%s; key_file=%s; CA_file=%s; CApath=%s; Password=%s; VerifyPeer=%d; KeyType=%d;", pweb->psrv->ptls->cipher_list, pweb->psrv->ptls->cert_file, pweb->psrv->ptls->key_file, pweb->psrv->ptls->ca_file, pweb->psrv->ptls->ca_path, pweb->psrv->ptls->password, pweb->psrv->ptls->verify_peer, pweb->psrv->ptls->key_type);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: configuration", 0);
   }

   /* With TLS v1.1 and later can use OPENSSL_init_ssl() for explicit initialization */

   if (mg_tls_so->p_SSL_library_init) {
      rc = mg_tls_so->p_SSL_library_init();

      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_library_init();", rc);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_library_init", 0);
      }
      if (rc != 1) {
         sprintf(message_buffer, "%d = SSL_library_init();", rc);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_library_init: Error", 0);
         rc = CACHE_NOCON;
         goto mgtls_open_session_exit;
      }
   }

   method = NULL;
   if (mg_tls_so->p_TLSv1_2_client_method) {
      method = mg_tls_so->p_TLSv1_2_client_method();
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%p = TLSv1_2_client_method();", method);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: TLSv1_2_client_method", 0);
      }
      if (method == NULL) {
         sprintf(message_buffer, "%p = TLSv1_2_client_method();", method);
      }
   }
   else if (mg_tls_so->p_TLSv1_1_client_method) {
      method = mg_tls_so->p_TLSv1_1_client_method();
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%p = TLSv1_1_client_method();", method);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: TLSv1_2_client_method", 0);
      }
      if (method == NULL) {
         sprintf(message_buffer, "%p = TLSv1_1_client_method();", method);
      }
   }
   else if (mg_tls_so->p_TLSv1_client_method) {
      method = mg_tls_so->p_TLSv1_client_method();
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%p = TLSv1_client_method();", method);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: TLSv1_2_client_method", 0);
      }
      if (method == NULL) {
         sprintf(message_buffer, "%p = TLSv1_client_method();", method);
      }
   }
   else if (mg_tls_so->p_SSLv3_client_method) {
      method = mg_tls_so->p_SSLv3_client_method();
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%p = SSLv3_client_method();", method);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSLv3_client_method", 0);
      }
      if (method == NULL) {
         sprintf(message_buffer, "%p = SSLv3_client_method();", method);
      }
   }
   else if (mg_tls_so->p_SSLv23_client_method) {
      method = mg_tls_so->p_SSLv23_client_method();
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%p = SSLv23_client_method();", method);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSLv23_client_method", 0);
      }
      if (method == NULL) {
         sprintf(message_buffer, "%p = SSLv23_client_method();", method);
      }
   }
   if (method == NULL) {
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: TLSv*_client_method OR SSLv*_client_method: Error", 0);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }

   if (mg_tls_so->p_SSL_load_error_strings) {
      mg_tls_so->p_SSL_load_error_strings();

      if (pweb->plog->log_tls > 0) {
         strcpy(message_buffer, "SSL_load_error_strings();");
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_load_error_strings", 0);
      }
   }

   ctx = mg_tls_so->p_SSL_CTX_new(method);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "%p = SSL_CTX_new(%p);", ctx, method);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_new", 0);
   }
   if (ctx == NULL) {
      sprintf(message_buffer, "%p = SSL_CTX_new(%p);", ctx, method);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_new: Error", 0);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }

#if defined(SSL_CTRL_OPTIONS)
   if (mg_tls_so->p_SSL_CTX_ctrl) {
      rc = mg_tls_so->p_SSL_CTX_ctrl(ctx, SSL_CTRL_OPTIONS, SSL_OP_NO_SSLv2, NULL);
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_CTX_ctrl(%p, %d, %ld, NULL);", rc, ctx, SSL_CTRL_OPTIONS, SSL_OP_NO_SSLv2);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_ctrl", 0);
      }
   }
#else
   if (mg_tls_so->p_SSL_CTX_set_options) {
      rc = mg_tls_so->p_SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_CTX_set_options(%p, %d);", rc, ctx, SSL_OP_NO_SSLv2);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_set_options", 0);
      }
   }
#endif

   rc = mg_tls_so->p_SSL_CTX_set_cipher_list(ctx, pweb->psrv->ptls->cipher_list);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "%d = SSL_CTX_set_cipher_list(%p, %s);", rc, ctx, pweb->psrv->ptls->cipher_list);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_set_cipher_list", 0);
   }
   if (rc == 0) {
      sprintf(message_buffer, "%d = SSL_CTX_set_cipher_list(%p, %s);", rc, ctx, pweb->psrv->ptls->cipher_list);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_set_cipher_list: Error", 0);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }

   if (pweb->psrv->ptls->cert_file && pweb->psrv->ptls->cert_file[0]) {

      rc = mg_tls_so->p_SSL_CTX_use_certificate_file(ctx, pweb->psrv->ptls->cert_file, SSL_FILETYPE_PEM);

      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_CTX_use_certificate_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->cert_file, SSL_FILETYPE_PEM);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_certificate_file", 0);
      }
      if (rc != 1) {
         sprintf(message_buffer, "%d = SSL_CTX_use_certificate_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->cert_file, SSL_FILETYPE_PEM);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_certificate_file: Error", 0);
         mgtls_log_error(pweb);
         rc = CACHE_NOCON;
         goto mgtls_open_session_exit;
      }

      if (pweb->psrv->ptls->password && pweb->psrv->ptls->password[0]) {
         mg_tls_so->p_SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) pweb->psrv->ptls->password);

         if (pweb->plog->log_tls > 0) {
            sprintf(message_buffer, "SSL_CTX_set_default_passwd_cb_userdata(%p, %s);", ctx, pweb->psrv->ptls->password);
            mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_set_default_passwd_cb_userdata", 0);
         }
      }

      if (pweb->psrv->ptls->key_type == 0) {
         rc = mg_tls_so->p_SSL_CTX_use_RSAPrivateKey_file(ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);

         if (pweb->plog->log_tls > 0) {
            sprintf(message_buffer, "%d = SSL_CTX_use_RSAPrivateKey_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);
            mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_RSAPrivateKey_file", 0);
         }
         if (rc != 1) {
            sprintf(message_buffer, "%d = SSL_CTX_use_RSAPrivateKey_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);
            mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_RSAPrivateKey_file: Error", 0);
            mgtls_log_error(pweb);
            rc = CACHE_NOCON;
            goto mgtls_open_session_exit;
         }

      }
      else {
         if (pweb->psrv->ptls->key_file && pweb->psrv->ptls->key_file[0]) {
            rc = mg_tls_so->p_SSL_CTX_use_PrivateKey_file(ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);

            if (pweb->plog->log_tls > 0) {
               sprintf(message_buffer, "%d = SSL_CTX_use_PrivateKey_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);
               mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_PrivateKey_file", 0);
            }
            if (rc != 1) {
               sprintf(message_buffer, "%d = SSL_CTX_use_PrivateKey_file(%p, %s, %d);", rc, ctx, pweb->psrv->ptls->key_file, SSL_FILETYPE_PEM);
               mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_use_PrivateKey_file: Error", 0);
               mgtls_log_error(pweb);
               rc = CACHE_NOCON;
               goto mgtls_open_session_exit;
            }
         }
         else {
            mg_log_event(pweb->plog, pweb, "Certificate key file not specified in the configuration", "TLS: SSL_CTX_use_PrivateKey_file: Error", 0);
            rc = CACHE_NOCON;
            goto mgtls_open_session_exit;
         }
      }

      rc = mg_tls_so->p_SSL_CTX_check_private_key(ctx);

      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_CTX_check_private_key(%p);", rc, ctx);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_check_private_key", 0);
      }
      if (rc != 1) {
         sprintf(message_buffer, "%d = SSL_CTX_check_private_key(%p);", rc, ctx);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_check_private_key: Error", 0);
         mgtls_log_error(pweb);
         rc = CACHE_NOCON;
         goto mgtls_open_session_exit;
      }

      if (pweb->psrv->ptls->verify_peer == 1) {
         mg_tls_so->p_SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

         if (pweb->plog->log_tls > 0) {
            sprintf(message_buffer, "%d = SSL_CTX_set_verify(%p, %d, NULL);", rc, ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
            mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_set_verify", 0);
         }
      }
   }

   if (pweb->psrv->ptls->ca_file && pweb->psrv->ptls->ca_file[0]) {
      rc = mg_tls_so->p_SSL_CTX_load_verify_locations(ctx, pweb->psrv->ptls->ca_file, NULL);

      if (pweb->plog->log_tls > 0) {
         sprintf(message_buffer, "%d = SSL_CTX_load_verify_locations(%p, %s, NULL);", rc, ctx, pweb->psrv->ptls->ca_file);
         mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_CTX_load_verify_locations", 0);
      }
      if (!rc) {
         mg_log_event(pweb->plog, pweb, "Unable to verify locations for client authentication.  Details to follow ...", "TLS: SSL_CTX_load_verify_locations: Error", 0);
         mgtls_log_error(pweb);
         rc = CACHE_NOCON;
         goto mgtls_open_session_exit;
      }
   }

   ssl = mg_tls_so->p_SSL_new(ctx);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "%p = SSL_new(%p);", ssl, ctx);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_new", 0);
   }
   if (ssl == NULL) {
      sprintf(message_buffer, "%p = SSL_new(%p);", ssl, ctx);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_new: Error", 0);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }

   mg_tls_so->p_SSL_set_fd(ssl, (int) pweb->pcon->cli_socket);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "SSL_set_fd(%p, %d);", ssl, (int) pweb->pcon->cli_socket);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS SSL_set_fd", 0);
   }

   rc = mg_tls_so->p_SSL_connect(ssl);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "%d = SSL_connect(%p);", rc, ssl);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS SSL_connect", 0);
   }
   if (rc == -1) {
      sprintf(message_buffer, "%d = SSL_connect(%p);", rc, ssl);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_connect: Error", 0);
      mgtls_log_error(pweb);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }
 
   server_cert = mg_tls_so->p_SSL_get_peer_certificate(ssl);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "%p = SSL_get_peer_certificate(%p);", server_cert, ssl);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_get_peer_certificate", 0);
   }
   if (server_cert == NULL) {
      sprintf(message_buffer, "%p = SSL_get_peer_certificate(%p);", server_cert, ssl);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_get_peer_certificate: Error", 0);
      rc = CACHE_NOCON;
      goto mgtls_open_session_exit;
   }

   subject = mg_crypt_so->p_X509_NAME_oneline(mg_crypt_so->p_X509_get_subject_name(server_cert), 0, 0);
   issuer = mg_crypt_so->p_X509_NAME_oneline(mg_crypt_so->p_X509_get_issuer_name(server_cert), 0, 0);

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "subject=%s; issuer=%s;", subject ? subject : NULL, issuer ? issuer : "NULL");
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: Server Certificate", 0);
   }

   mg_crypt_so->p_CRYPTO_free(subject);
   mg_crypt_so->p_CRYPTO_free(issuer);
   mg_crypt_so->p_X509_free(server_cert);

   rc = CACHE_SUCCESS;

mgtls_open_session_exit:

   if (pweb->plog->log_tls > 0) {
      sprintf(message_buffer, "result=%d;", rc);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: mgtls_open_session", 0);
   }

   if (rc == CACHE_SUCCESS) {
      ptlscon = (DBXTLSCON *) mg_malloc(NULL, sizeof(DBXTLSCON), 0);
      if (!ptlscon) {
         if (!pweb->error[0]) {
            strcpy(pweb->error, "No Memory");
            pweb->error_code = 1009;
         }
         return CACHE_NOCON;
      }
      memset((void *) ptlscon, 0, sizeof(DBXTLSCON));
      ptlscon->ssl = ssl;
      ptlscon->ctx = ctx;
      pweb->pcon->ptlscon = ptlscon;
   }
   else {
      pweb->pcon->ptlscon = NULL;
   }

   return rc;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {
   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf(buffer, "Exception caught in f:mgtls_open_session: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif

}


int mgtls_close_session(MGWEB *pweb)
{
   DBXTLSCON *ptlscon;

   ptlscon = (DBXTLSCON *) pweb->pcon->ptlscon;

   mg_tls_so->p_SSL_free(ptlscon->ssl);

   mg_tls_so->p_SSL_CTX_free(ptlscon->ctx);

   return CACHE_SUCCESS;
}


int mgtls_recv(MGWEB *pweb, unsigned char *buffer, int size)
{
   int rc;
   char message_buffer[256];
   DBXTLSCON *ptlscon;

   ptlscon = (DBXTLSCON *) pweb->pcon->ptlscon;

   rc = mg_tls_so->p_SSL_read(ptlscon->ssl, (void *) buffer, size);

   if (pweb->plog->log_tls > 1) {
      sprintf(message_buffer, "%d = SSL_read(%p, %p, %d);", rc, ptlscon->ssl, buffer, size);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_read", -1);
   }
   if (rc == -1) {
      sprintf(message_buffer, "%d = SSL_read(%p, %p, %d);", rc, ptlscon->ssl, buffer, size);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_read: Error", -1);
      mgtls_log_error(pweb);
   }

   return rc;
}


int mgtls_send(MGWEB *pweb, unsigned char *buffer, int size)
{
   int rc;
   char message_buffer[256];
   DBXTLSCON *ptlscon;

   ptlscon = (DBXTLSCON *) pweb->pcon->ptlscon;

   rc = mg_tls_so->p_SSL_write(ptlscon->ssl, (void *) buffer, size);

   if (pweb->plog->log_tls > 1) {
      sprintf(message_buffer, "%d = SSL_write(%p, %p, %d);", rc, ptlscon->ssl, buffer, size);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_write", -1);
   }
   if (rc == -1) {
      sprintf(message_buffer, "%d = SSL_write(%p, %p, %d);", rc, ptlscon->ssl, buffer, size);
      mg_log_event(pweb->plog, pweb, message_buffer, "TLS: SSL_write: Error", -1);
      mgtls_log_error(pweb);
   }

   return rc;
}


int mgtls_log_error(MGWEB *pweb)
{
   unsigned long ecode;
   char *error;
   char buffer[256];

#ifdef _WIN32
__try {
#endif

   while ((ecode = mg_crypt_so->p_ERR_get_error())) {
      error = mg_crypt_so->p_ERR_error_string(ecode, NULL);
      if (error) {
         sprintf(buffer, "%s (%lu)", error, ecode);
         mg_log_event(pweb->plog, pweb, buffer, "SSL: Error Condition", 0);
        }
    }

   return 1;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER) {

   DWORD code;
   char buffer[256];

   __try {
      code = GetExceptionCode();
      sprintf(buffer, "Exception caught in f:mgtls_log_error: %x", code);
      mg_log_event(pweb->plog, pweb, buffer, "Error Condition", 0);
   }
   __except (EXCEPTION_EXECUTE_HANDLER ) {
      ;
   }

   return 0;
}
#endif
}


int mgtls_crypt_load_library(MGWEB *pweb)
{
   int n, n1, result, libnam_plen;
   char error_message[DBX_ERROR_SIZE];
   char fun[64], libs[256];
   char *libnam[16], *p1, *p2;
   DBXCRYPTSO *p_crypt_so;

   if (mg_crypt_so && mg_crypt_so->loaded) {
      return CACHE_SUCCESS;
   }

   if (!mg_crypt_so) {
      mg_enter_critical_section((void *) &mg_global_mutex);
      if (!mg_crypt_so) {
         mg_crypt_so = (DBXCRYPTSO *) mg_malloc(NULL, sizeof(DBXCRYPTSO), 0);
         if (!mg_crypt_so) {
            mg_leave_critical_section((void *) &mg_global_mutex);
            if (!pweb->error[0]) {
               strcpy(pweb->error, "No Memory");
               pweb->error_code = 1009;
            }
            return CACHE_NOCON;
         }
         memset((void *) mg_crypt_so, 0, sizeof(DBXCRYPTSO));
         mg_crypt_so->loaded = 0;
      }
      mg_leave_critical_section((void *) &mg_global_mutex);
   }

   if (!mg_crypt_so) {
      strcpy(pweb->error, "Error loading Crypto library");
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(mg_crypt_so->dbname, "Crypto");
   p_crypt_so = mg_crypt_so;

   libnam_plen = 0;
   if (pweb->psrv->ptls->libpath && pweb->psrv->ptls->libpath[0]) {
      strcpy(p_crypt_so->libnam, pweb->psrv->ptls->libpath);
      libnam_plen = (int) strlen(p_crypt_so->libnam);
      if (p_crypt_so->libnam[libnam_plen - 1] != '/') {
         p_crypt_so->libnam[libnam_plen] = '/';
         p_crypt_so->libnam[++ libnam_plen] = '\0';
      }
   }

   /* v2.3.22 */
   strncpy(libs, DBX_CRYPT_LIB, 250);
   libs[250] = '\0';
   p1 = libs;
   n1 = 0;
   for (n = 0; n < 16; n ++) {
      p2 = strstr(p1, " ");
      if (p2)
         *p2 = '\0';
      if (p1[0]) {
         libnam[n1 ++] = p1;
      }
      if (!p2)
         break;
      p1 = (p2 + 1);
   }
   libnam[n1] = NULL;

   for (n = 0; libnam[n]; n ++) {
      strcpy(p_crypt_so->libnam + libnam_plen, libnam[n]);
      if (pweb->plog->log_tls > 0) { /* v2.3.22 */
         mg_log_event(pweb->plog, pweb, p_crypt_so->libnam, "TLS: Attepting to load the cryptography library", 0);
      }
      p_crypt_so->p_library = mg_dso_load(p_crypt_so->libnam);
      if (p_crypt_so->p_library) {
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
         sprintf(pweb->error, "Error loading %s Library: %s; Error Code : %ld",  p_crypt_so->dbname, p_crypt_so->libnam, errorcode);
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
            strncpy(error_message, (const char *) lpMsgBuf, DBX_ERROR_SIZE - 1);
            p = strstr(error_message, "\r\n");
            if (p)
               *p = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            error_message[len1] = '\0';
            p = strstr(error_message, "%1");
            if (p) {
               *p = 'I';
               *(p + 1) = 't';
            }
            strcat(pweb->error, " (");
            strcat(pweb->error, error_message);
            strcat(pweb->error, ")");
         }
         if (lpMsgBuf) {
            LocalFree(lpMsgBuf);
         }
#else
         p = (char *) dlerror();
         sprintf(error_message, "Cannot load %s library: Error Code: %d", p_crypt_so->dbname, errno);
         len2 = strlen(pweb->error);
         if (p) {
            strncpy(error_message, p, DBX_ERROR_SIZE - 1);
            error_message[DBX_ERROR_SIZE - 1] = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            error_message[len1] = '\0';
            strcat(pweb->error, " (");
            strcat(pweb->error, error_message);
            strcat(pweb->error, ")");
         }
#endif
      }
   }

   if (!p_crypt_so->p_library) {
      goto mgtls_crypt_load_library_exit;
   }

   pweb->error[0] = '\0'; /* v2.3.22 */

   strcpy(fun, "OpenSSL_version");
   p_crypt_so->p_OpenSSL_version = (const char * (*) (int)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);

   strcpy(fun, "SSLeay_version");
   p_crypt_so->p_SSLeay_version = (const char * (*) (int)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);

   strcpy(fun, "HMAC");
   p_crypt_so->p_HMAC = (unsigned char * (*) (const EVP_MD *, const void *, int, const unsigned char *, int, unsigned char *, unsigned int *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_HMAC) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "EVP_sha1");
   p_crypt_so->p_EVP_sha1 = (const EVP_MD * (*) (void)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_EVP_sha1) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "EVP_sha256");
   p_crypt_so->p_EVP_sha256 = (const EVP_MD * (*) (void)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_EVP_sha256) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "EVP_sha512");
   p_crypt_so->p_EVP_sha512 = (const EVP_MD * (*) (void)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_EVP_sha512) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(fun, "EVP_md5");
   p_crypt_so->p_EVP_md5 = (const EVP_MD * (*) (void)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_EVP_md5) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(fun, "SHA1");
   p_crypt_so->p_SHA1 = (unsigned char * (*) (const unsigned char *, unsigned long, unsigned char *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_SHA1) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "SHA256");
   p_crypt_so->p_SHA256 = (unsigned char * (*) (const unsigned char *, unsigned long, unsigned char *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_SHA256) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "SHA512");
   p_crypt_so->p_SHA512 = (unsigned char * (*) (const unsigned char *, unsigned long, unsigned char *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_SHA512) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "MD5");
   p_crypt_so->p_MD5 = (unsigned char * (*) (const unsigned char *, unsigned long, unsigned char *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_MD5) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(fun, "X509_get_subject_name");
   p_crypt_so->p_X509_get_subject_name = (X509_NAME * (*) (X509 *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_get_subject_name) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_NAME_oneline");
   p_crypt_so->p_X509_NAME_oneline = (char * (*) (X509_NAME *, char *, int)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_NAME_oneline) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "CRYPTO_free");
   p_crypt_so->p_CRYPTO_free = (void (*) (void *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_CRYPTO_free) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_get_issuer_name");
   p_crypt_so->p_X509_get_issuer_name = (X509_NAME * (*) (X509 *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_get_issuer_name) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_free");
   p_crypt_so->p_X509_free = (void (*) (X509 *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_free) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(fun, "X509_STORE_CTX_get_current_cert");
   p_crypt_so->p_X509_STORE_CTX_get_current_cert = (X509 * (*) (X509_STORE_CTX *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_STORE_CTX_get_current_cert) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_STORE_CTX_set_error");
   p_crypt_so->p_X509_STORE_CTX_set_error = (void (*) (X509_STORE_CTX *, int)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_STORE_CTX_set_error) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_STORE_CTX_get_error");
   p_crypt_so->p_X509_STORE_CTX_get_error = (int (*) (X509_STORE_CTX *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_STORE_CTX_get_error) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_STORE_CTX_get_error_depth");
   p_crypt_so->p_X509_STORE_CTX_get_error_depth = (int (*) (X509_STORE_CTX *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_STORE_CTX_get_error_depth) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_STORE_CTX_get_ex_data");
   p_crypt_so->p_X509_STORE_CTX_get_ex_data = (void * (*) (X509_STORE_CTX *, int)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_STORE_CTX_get_ex_data) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "X509_verify_cert_error_string");
   p_crypt_so->p_X509_verify_cert_error_string = (const char * (*) (long)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_X509_verify_cert_error_string) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(fun, "ERR_get_error");
   p_crypt_so->p_ERR_get_error = (unsigned long (*) (void)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_ERR_get_error) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }
   strcpy(fun, "ERR_error_string");
   p_crypt_so->p_ERR_error_string = (char * (*) (unsigned long, char *)) mg_dso_sym(p_crypt_so->p_library, (char *) fun);
   if (!p_crypt_so->p_ERR_error_string) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_crypt_so->dbname, p_crypt_so->libnam, fun);
      goto mgtls_crypt_load_library_exit;
   }

   strcpy(error_message, "OpenSSL library loaded");
   if (p_crypt_so->p_OpenSSL_version)
      sprintf(error_message, "OpenSSL library loaded: version %s", p_crypt_so->p_OpenSSL_version(0));
   else if (p_crypt_so->p_SSLeay_version)
      sprintf(error_message, "OpenSSL library loaded: version %s", p_crypt_so->p_SSLeay_version(0));

   if (pweb->plog->log_tls > 0) {
      mg_log_event(pweb->plog, pweb, error_message, "TLS: OpenSSL Libraries", 0);
   }

   p_crypt_so->loaded = 1;
   mg_crypt_so = p_crypt_so;

mgtls_crypt_load_library_exit:

   if (pweb->error[0]) {
      if (mg_crypt_so) {
         p_crypt_so->loaded = 0;
      }
      pweb->error_code = 1009;
      result = CACHE_NOCON;
      return result;
   }

   return CACHE_SUCCESS;
}


int mgtls_crypt_unload_library(void)
{
   mg_dso_unload(mg_crypt_so->p_library);
   mg_crypt_so->p_library = NULL;
   mg_crypt_so->loaded = 0;
   return CACHE_SUCCESS;
}


int mgtls_tls_load_library(MGWEB *pweb)
{
   int n, n1, result, libnam_plen;
   char error_message[DBX_ERROR_SIZE];
   char fun[64], libs[256];
   char *libnam[16], *p1, *p2;
   DBXTLSSO *p_tls_so;

   if (mg_tls_so && mg_tls_so->loaded) {
      return CACHE_SUCCESS;
   }

   if (!mg_tls_so) {
      mg_enter_critical_section((void *) &mg_global_mutex);
      if (!mg_tls_so) {
         mg_tls_so = (DBXTLSSO *) mg_malloc(NULL, sizeof(DBXTLSSO), 0);
         if (!mg_tls_so) {
            mg_leave_critical_section((void *) &mg_global_mutex);
            if (!pweb->error[0]) {
               strcpy(pweb->error, "No Memory");
               pweb->error_code = 1009;
            }
            return CACHE_NOCON;
         }
         memset((void *) mg_tls_so, 0, sizeof(DBXTLSSO));
         mg_tls_so->loaded = 0;
      }
      mg_leave_critical_section((void *) &mg_global_mutex);
   }

   if (!mg_tls_so) {
      strcpy(pweb->error, "Error loading TLS library");
      goto mgtls_tls_load_library_exit;
   }
   strcpy(mg_tls_so->dbname, "TLS");
   p_tls_so = mg_tls_so;

   libnam_plen = 0;
   if (pweb->psrv->ptls->libpath && pweb->psrv->ptls->libpath[0]) {
      strcpy(p_tls_so->libnam, pweb->psrv->ptls->libpath);
      libnam_plen = (int) strlen(p_tls_so->libnam);
      if (p_tls_so->libnam[libnam_plen - 1] != '/') {
         p_tls_so->libnam[libnam_plen] = '/';
         p_tls_so->libnam[++ libnam_plen] = '\0';
      }
   }

   /* v2.3.22 */
   strncpy(libs, DBX_TLS_LIB, 250);
   libs[250] = '\0';
   p1 = libs;
   n1 = 0;
   for (n = 0; n < 16; n ++) {
      p2 = strstr(p1, " ");
      if (p2)
         *p2 = '\0';
      if (p1[0]) {
         libnam[n1 ++] = p1;
      }
      if (!p2)
         break;
      p1 = (p2 + 1);
   }
   libnam[n1] = NULL;

   for (n = 0; libnam[n]; n ++) {
      strcpy(p_tls_so->libnam + libnam_plen, libnam[n]);
      if (pweb->plog->log_tls > 0) { /* v2.3.22 */
         mg_log_event(pweb->plog, pweb, p_tls_so->libnam, "TLS: Attepting to load the TLS library", 0);
      }
      p_tls_so->p_library = mg_dso_load(p_tls_so->libnam);
      if (p_tls_so->p_library) {
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
         sprintf(pweb->error, "Error loading %s Library: %s; Error Code : %ld",  p_tls_so->dbname, p_tls_so->libnam, errorcode);
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
            strncpy(error_message, (const char *) lpMsgBuf, DBX_ERROR_SIZE - 1);
            p = strstr(error_message, "\r\n");
            if (p)
               *p = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            error_message[len1] = '\0';
            p = strstr(error_message, "%1");
            if (p) {
               *p = 'I';
               *(p + 1) = 't';
            }
            strcat(pweb->error, " (");
            strcat(pweb->error, error_message);
            strcat(pweb->error, ")");
         }
         if (lpMsgBuf) {
            LocalFree(lpMsgBuf);
         }
#else
         p = (char *) dlerror();
         sprintf(error_message, "Cannot load %s library: Error Code: %d", p_tls_so->dbname, errno);
         len2 = strlen(pweb->error);
         if (p) {
            strncpy(error_message, p, DBX_ERROR_SIZE - 1);
            error_message[DBX_ERROR_SIZE - 1] = '\0';
            len1 = (DBX_ERROR_SIZE - (len2 + 10));
            if (len1 < 1)
               len1 = 0;
            error_message[len1] = '\0';
            strcat(pweb->error, " (");
            strcat(pweb->error, error_message);
            strcat(pweb->error, ")");
         }
#endif
      }
   }

   if (!p_tls_so->p_library) {
      goto mgtls_tls_load_library_exit;
   }

   pweb->error[0] = '\0'; /* v2.3.22 */

   strcpy(fun, "SSL_library_init");
   p_tls_so->p_SSL_library_init = (int (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   strcpy(fun, "OPENSSL_init_ssl");
   p_tls_so->p_OPENSSL_init_ssl = (int (*) (unsigned long, void *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   strcpy(fun, "SSLv2_client_method");
   p_tls_so->p_SSLv2_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "SSLv23_client_method");
   p_tls_so->p_SSLv23_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "SSLv3_client_method");
   p_tls_so->p_SSLv3_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "TLSv1_client_method");
   p_tls_so->p_TLSv1_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "TLSv1_1_client_method");
   p_tls_so->p_TLSv1_1_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "TLSv1_2_client_method");
   p_tls_so->p_TLSv1_2_client_method = (SSL_METHOD * (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   if (!p_tls_so->p_SSLv2_client_method && !p_tls_so->p_SSLv23_client_method && !p_tls_so->p_SSLv3_client_method && !p_tls_so->p_TLSv1_client_method && !p_tls_so->p_TLSv1_1_client_method && p_tls_so->p_TLSv1_2_client_method) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : SSLv*_client_method OR TLSv*_client_method", p_tls_so->dbname, p_tls_so->libnam);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_load_error_strings");
   p_tls_so->p_SSL_load_error_strings = (void (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   strcpy(fun, "SSL_CTX_new");
   p_tls_so->p_SSL_CTX_new = (SSL_CTX * (*) (SSL_METHOD *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_new) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_CTX_set_options");
   p_tls_so->p_SSL_CTX_set_options = (long (*) (SSL_CTX *, long)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   strcpy(fun, "SSL_CTX_ctrl");
   p_tls_so->p_SSL_CTX_ctrl = (long (*) (SSL_CTX *, int, long, void *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_ctrl) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_set_cipher_list");
   p_tls_so->p_SSL_CTX_set_cipher_list = (int (*) (SSL_CTX *, const char *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_cipher_list) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_new");
   p_tls_so->p_SSL_new = (SSL * (*) (SSL_CTX *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_new) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_set_fd");
   p_tls_so->p_SSL_set_fd = (int (*) (SSL *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_set_fd) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_CTX_use_certificate_file");
   p_tls_so->p_SSL_CTX_use_certificate_file = (int (*) (SSL_CTX *, const char *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_use_certificate_file) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_use_PrivateKey_file");
   p_tls_so->p_SSL_CTX_use_PrivateKey_file = (int (*) (SSL_CTX *, const char *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_use_PrivateKey_file) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_use_RSAPrivateKey_file");
   p_tls_so->p_SSL_CTX_use_RSAPrivateKey_file = (int (*) (SSL_CTX *, const char *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_use_RSAPrivateKey_file) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_check_private_key");
   p_tls_so->p_SSL_CTX_check_private_key = (int (*) (SSL_CTX *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_check_private_key) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_CTX_set_default_passwd_cb");
   p_tls_so->p_SSL_CTX_set_default_passwd_cb = (void (*) (SSL_CTX *, pem_password_cb *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_default_passwd_cb) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_set_default_passwd_cb_userdata");
   p_tls_so->p_SSL_CTX_set_default_passwd_cb_userdata = (void (*) (SSL_CTX *, void *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_default_passwd_cb_userdata) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_accept");
   p_tls_so->p_SSL_accept = (int (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_accept) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_connect");
   p_tls_so->p_SSL_connect = (int (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_connect) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_get_current_cipher");
   p_tls_so->p_SSL_get_current_cipher = (SSL_CIPHER * (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_get_current_cipher) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CIPHER_get_name");
   p_tls_so->p_SSL_CIPHER_get_name = (const char * (*) (SSL_CIPHER *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CIPHER_get_name) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_get_peer_certificate");
   p_tls_so->p_SSL_get_peer_certificate = (X509 * (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_get_peer_certificate) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_write");
   p_tls_so->p_SSL_write = (int (*) (SSL *, const void *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_write) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_read");
   p_tls_so->p_SSL_read = (int (*) (SSL *, void *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_read) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_shutdown");
   p_tls_so->p_SSL_shutdown = (int (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_shutdown) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_free");
   p_tls_so->p_SSL_free = (void (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_free) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_free");
   p_tls_so->p_SSL_CTX_free = (void (*) (SSL_CTX *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_free) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_CTX_set_verify");
   p_tls_so->p_SSL_CTX_set_verify = (void (*) (SSL_CTX *, int, int (*) (int, X509_STORE_CTX *))) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_verify) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_set_verify_depth");
   p_tls_so->p_SSL_CTX_set_verify_depth = (void (*) (SSL_CTX *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_verify_depth) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_set_cert_verify_callback");
   p_tls_so->p_SSL_CTX_set_cert_verify_callback = (void (*) (SSL_CTX *, int (*) (X509_STORE_CTX *, void *), void *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_set_cert_verify_callback) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_CTX_load_verify_locations");
   p_tls_so->p_SSL_CTX_load_verify_locations = (int (*) (SSL_CTX *, const char *, const char *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_CTX_load_verify_locations) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_set_verify_result");
   p_tls_so->p_SSL_set_verify_result = (void (*) (SSL *, long)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_set_verify_result) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_get_verify_result");
   p_tls_so->p_SSL_get_verify_result = (long (*) (SSL *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_get_verify_result) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_set_verify");
   p_tls_so->p_SSL_set_verify = (void (*) (SSL *, int, int (*) (int, X509_STORE_CTX *))) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_set_verify) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }
   strcpy(fun, "SSL_set_verify_depth");
   p_tls_so->p_SSL_set_verify_depth = (void (*) (SSL *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   if (!p_tls_so->p_SSL_set_verify_depth) {
      sprintf(pweb->error, "Error loading %s library: %s; Cannot locate the following function : %s", p_tls_so->dbname, p_tls_so->libnam, fun);
      goto mgtls_tls_load_library_exit;
   }

   strcpy(fun, "SSL_set_ex_data");
   p_tls_so->p_SSL_set_ex_data = (int (*) (SSL *, int, void *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "SSL_get_ex_data");
   p_tls_so->p_SSL_get_ex_data = (void * (*) (SSL *, int)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "SSL_get_ex_new_index");
   p_tls_so->p_SSL_get_ex_new_index = (int (*) (long, void *, CRYPTO_EX_new *, CRYPTO_EX_dup *, CRYPTO_EX_free *)) mg_dso_sym(p_tls_so->p_library, (char *) fun);
   strcpy(fun, "SSL_get_ex_data_X509_STORE_CTX_idx");
   p_tls_so->p_SSL_get_ex_data_X509_STORE_CTX_idx = (int (*) (void)) mg_dso_sym(p_tls_so->p_library, (char *) fun);

   p_tls_so->loaded = 1;
   mg_tls_so = p_tls_so;

mgtls_tls_load_library_exit:

   if (pweb->error[0]) {
      if (mg_tls_so) {
         mg_tls_so->loaded = 0;
      }
      pweb->error_code = 1009;
      result = CACHE_NOCON;
      return result;
   }

   return CACHE_SUCCESS;
}


int mgtls_tls_unload_library(void)
{
   mg_dso_unload(mg_tls_so->p_library);
   mg_tls_so->p_library = NULL;
   mg_tls_so->loaded = 0;
   return CACHE_SUCCESS;
}


#else /* #if DBX_WITH_TLS >= 1 */


int mgtls_open_session(MGWEB *pweb)
{
   return CACHE_FAILURE;
}

int mgtls_close_session(MGWEB *pweb)
{
   return CACHE_FAILURE;
}

int mgtls_recv(MGWEB *pweb, unsigned char *buffer, int size)
{
   return CACHE_FAILURE;
}

int mgtls_send(MGWEB *pweb, unsigned char *buffer, int size)
{
   return CACHE_FAILURE;
}

int mgtls_log_error(MGWEB *pweb)
{
   return CACHE_FAILURE;
}

int mgtls_crypt_load_library(MGWEB *pweb)
{
   return CACHE_FAILURE;
}

int mgtls_crypt_unload_library(void)
{
   return CACHE_FAILURE;
}

int mgtls_tls_load_library(MGWEB *pweb)
{
   return CACHE_FAILURE;
}

int mgtls_tls_unload_library(void)
{
   return CACHE_FAILURE;
}

#endif /* #if DBX_WITH_TLS >= 1 */
