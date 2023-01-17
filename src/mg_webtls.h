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

#ifndef MG_WEBTLS_H
#define MG_WEBTLS_H

#if DBX_WITH_TLS >= 1

#include <openssl/rsa.h> 
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


/* v2.3.22 - An ordered list of OpenSSL libraries that mg_web will try to load, each name separated by a single white space */
#if defined(_WIN32)

#if defined(_WIN64)
#define DBX_CRYPT_LIB            "libcrypto-1_1-x64.dll libcrypto.dll libeay32.dll"
#define DBX_TLS_LIB              "libssl-1_1-x64.dll libssl.dll ssleay32.dll"
#else
#define DBX_CRYPT_LIB            "libcrypto-1_1.dll libcrypto.dll libeay32.dll"
#define DBX_TLS_LIB              "libssl-1_1.dll libssl.dll ssleay32.dll"
#endif

#else

#if defined(MACOSX)
#define DBX_CRYPT_LIB            "libcrypto.dylib libcrypto.so"
#define DBX_TLS_LIB              "libssl.dylib libssl.so"
#else
#define DBX_CRYPT_LIB            "libcrypto.so libcrypto.sl"
#define DBX_TLS_LIB              "libssl.so libssl.sl"
#endif

#endif


typedef struct tagDBXTLSCON {
   SSL_CTX           *ctx;
   SSL               *ssl;
} DBXTLSCON, *PDBXTLSCON;


typedef struct tagDBXCRYPTSO {
   short             loaded;
   char              libnam[256];
   char              dbname[32];
   DBXPLIB           p_library;

   const char *      (* p_OpenSSL_version)                        (int type);
   const char *      (* p_SSLeay_version)                         (int type);

   unsigned char *   (* p_HMAC)                                   (const EVP_MD *evp_md, const void *key, int key_len, const unsigned char *d, int n, unsigned char *md, unsigned int *md_len);
   const EVP_MD *    (* p_EVP_sha1)                               (void);
   const EVP_MD *    (* p_EVP_sha256)                             (void);
   const EVP_MD *    (* p_EVP_sha512)                             (void);
   const EVP_MD *    (* p_EVP_md5)                                (void);
   unsigned char *   (* p_SHA1)                                   (const unsigned char *d, unsigned long n, unsigned char *md);
   unsigned char *   (* p_SHA256)                                 (const unsigned char *d, unsigned long n, unsigned char *md);
   unsigned char *   (* p_SHA512)                                 (const unsigned char *d, unsigned long n, unsigned char *md);
   unsigned char *   (* p_MD5)                                    (const unsigned char *d, unsigned long n, unsigned char *md);

   X509_NAME *       (* p_X509_get_subject_name)                  (X509 *a);
   char *            (* p_X509_NAME_oneline)                      (X509_NAME *a, char *buf, int size);
   void              (* p_CRYPTO_free)                            (void *);
   X509_NAME *       (* p_X509_get_issuer_name)                   (X509 *a);
   void              (* p_X509_free)                              (X509 *server_cert);

   X509 *            (* p_X509_STORE_CTX_get_current_cert)        (X509_STORE_CTX *ctx);
   void              (* p_X509_STORE_CTX_set_error)               (X509_STORE_CTX *ctx, int s);
   int               (* p_X509_STORE_CTX_get_error)               (X509_STORE_CTX *ctx);
   int               (* p_X509_STORE_CTX_get_error_depth)         (X509_STORE_CTX *ctx);
   void *            (* p_X509_STORE_CTX_get_ex_data)             (X509_STORE_CTX *ctx, int idx);
   const char *      (* p_X509_verify_cert_error_string)          (long n);

   unsigned long     (* p_ERR_get_error)                          (void);
   char *            (* p_ERR_error_string)                       (unsigned long e, char *buf);

} DBXCRYPTSO, *PDBXCRYPTSO;


typedef struct tagDBXTLSSO {
   short             loaded;
   char              libnam[256];
   char              dbname[32];
   DBXPLIB           p_library;

   int               (* p_SSL_library_init)                       (void);
   int               (* p_OPENSSL_init_ssl)                       (unsigned long opts, void *settings);
   SSL_METHOD *      (* p_SSLv2_client_method)                    (void);
   SSL_METHOD *      (* p_SSLv23_client_method)                   (void);
   SSL_METHOD *      (* p_SSLv3_client_method)                    (void);
   SSL_METHOD *      (* p_TLSv1_client_method)                    (void);
   SSL_METHOD *      (* p_TLSv1_1_client_method)                  (void);
   SSL_METHOD *      (* p_TLSv1_2_client_method)                  (void);
   void              (* p_SSL_load_error_strings)                 (void);
   SSL_CTX *         (* p_SSL_CTX_new)                            (SSL_METHOD *meth);
   long              (* p_SSL_CTX_set_options)                    (SSL_CTX *ctx, long options);
   long              (* p_SSL_CTX_ctrl)                           (SSL_CTX *ctx, int cmd, long larg, void *parg);
   int               (* p_SSL_CTX_set_cipher_list)                (SSL_CTX *ctx, const char *str);
   SSL *             (* p_SSL_new)                                (SSL_CTX *ctx);
   int               (* p_SSL_set_fd)                             (SSL *s, int fd);
   int               (* p_SSL_CTX_use_certificate_file)           (SSL_CTX *ctx, const char *file, int type);
   int               (* p_SSL_CTX_use_PrivateKey_file)            (SSL_CTX *ctx, const char *file, int type);
   int               (* p_SSL_CTX_use_RSAPrivateKey_file)         (SSL_CTX *ctx, const char *file, int type);
   int               (* p_SSL_CTX_check_private_key)              (SSL_CTX *ctx);

   void              (* p_SSL_CTX_set_default_passwd_cb)          (SSL_CTX *ctx, pem_password_cb *cb);
   void              (* p_SSL_CTX_set_default_passwd_cb_userdata) (SSL_CTX *ctx, void *u);

   int               (* p_SSL_accept)                             (SSL *ssl);
   int               (* p_SSL_connect)                            (SSL *ssl);
   SSL_CIPHER *      (* p_SSL_get_current_cipher)                 (SSL *s);
   const char *      (* p_SSL_CIPHER_get_name)                    (SSL_CIPHER *c);
   X509 *            (* p_SSL_get_peer_certificate)               (SSL *s);
   int               (* p_SSL_write)                              (SSL *ssl, const void *buf, int num);
   int               (* p_SSL_read)                               (SSL *ssl ,void *buf, int num);
   int               (* p_SSL_shutdown)                           (SSL *s);
   void              (* p_SSL_free)                               (SSL *ssl);
   void              (* p_SSL_CTX_free)                           (SSL_CTX *);

   void              (* p_SSL_CTX_set_verify)                     (SSL_CTX *ctx, int mode, int (*callback)(int, X509_STORE_CTX *));
   void              (* p_SSL_CTX_set_verify_depth)               (SSL_CTX *ctx, int depth);
   void              (* p_SSL_CTX_set_cert_verify_callback)       (SSL_CTX *ctx, int (*cb) (X509_STORE_CTX *, void *), void *arg);
   int               (* p_SSL_CTX_load_verify_locations)          (SSL_CTX *ctx, const char *CAfile, const char *CApath);

   void              (* p_SSL_set_verify_result)                  (SSL *ssl, long v);
   long              (* p_SSL_get_verify_result)                  (SSL *ssl);
   void              (* p_SSL_set_verify)                         (SSL *s, int mode, int (*callback)(int ok, X509_STORE_CTX *ctx));
   void              (* p_SSL_set_verify_depth)                   (SSL *s, int depth);

   int               (* p_SSL_set_ex_data)                        (SSL *ssl, int idx, void *data);
   void *            (* p_SSL_get_ex_data)                        (SSL *ssl, int idx);
   int               (* p_SSL_get_ex_new_index)                   (long argl, void *argp, CRYPTO_EX_new *new_func, CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func);
   int               (* p_SSL_get_ex_data_X509_STORE_CTX_idx)     (void );

} DBXTLSSO, *PDBXTLSSO;

#endif /* #if DBX_WITH_TLS >= 1 */

int mgtls_open_session           (MGWEB *pweb);
int mgtls_close_session          (MGWEB *pweb);
int mgtls_recv                   (MGWEB *pweb, unsigned char *buffer, int size);
int mgtls_send                   (MGWEB *pweb, unsigned char *buffer, int size);
int mgtls_log_error              (MGWEB *pweb);
int mgtls_crypt_load_library     (MGWEB *pweb);
int mgtls_crypt_unload_library   (void);
int mgtls_tls_load_library       (MGWEB *pweb);
int mgtls_tls_unload_library     (void);

#endif
