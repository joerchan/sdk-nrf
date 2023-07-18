/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 */

/* This file is intentionally empty.*/

/* PSA and drivers */
#cmakedefine MBEDTLS_PSA_CRYPTO_C
#cmakedefine MBEDTLS_USE_PSA_CRYPTO
#cmakedefine MBEDTLS_PSA_CRYPTO_STORAGE_C
/* MBEDTLS_PSA_CRYPTO_DRIVERS is defined to 1 by TF-M's build system. */
#cmakedefine MBEDTLS_PSA_CRYPTO_DRIVERS                         @MBEDTLS_PSA_CRYPTO_DRIVERS@
#cmakedefine MBEDTLS_PSA_CRYPTO_CLIENT
#cmakedefine MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG

/* TF-M */
#cmakedefine MBEDTLS_PSA_CRYPTO_SPM
#cmakedefine MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER

/* Platform */
#cmakedefine MBEDTLS_PLATFORM_C
#cmakedefine MBEDTLS_PLATFORM_MEMORY
#cmakedefine MBEDTLS_NO_PLATFORM_ENTROPY
#cmakedefine MBEDTLS_MEMORY_BUFFER_ALLOC_C

/* Platform configurations for _ALT defines */
#cmakedefine MBEDTLS_PLATFORM_EXIT_ALT
#cmakedefine MBEDTLS_PLATFORM_FPRINTF_ALT
#cmakedefine MBEDTLS_PLATFORM_PRINTF_ALT
#cmakedefine MBEDTLS_PLATFORM_SNPRINTF_ALT
#cmakedefine MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT
#cmakedefine MBEDTLS_ENTROPY_HARDWARE_ALT
#cmakedefine MBEDTLS_THREADING_C
#cmakedefine MBEDTLS_THREADING_ALT
#cmakedefine MBEDTLS_PLATFORM_ZEROIZE_ALT

/* Legacy configurations for _ALT defines */
#cmakedefine MBEDTLS_AES_SETKEY_ENC_ALT
#cmakedefine MBEDTLS_AES_SETKEY_DEC_ALT
#cmakedefine MBEDTLS_AES_ENCRYPT_ALT
#cmakedefine MBEDTLS_AES_DECRYPT_ALT
#cmakedefine MBEDTLS_AES_ALT
#cmakedefine MBEDTLS_CMAC_ALT
#cmakedefine MBEDTLS_CCM_ALT
#cmakedefine MBEDTLS_GCM_ALT
#cmakedefine MBEDTLS_CHACHA20_ALT
#cmakedefine MBEDTLS_POLY1305_ALT
#cmakedefine MBEDTLS_CHACHAPOLY_ALT
#cmakedefine MBEDTLS_DHM_ALT
#cmakedefine MBEDTLS_ECP_ALT
#cmakedefine MBEDTLS_ECDH_GEN_PUBLIC_ALT
#cmakedefine MBEDTLS_ECDH_COMPUTE_SHARED_ALT
#cmakedefine MBEDTLS_ECDSA_GENKEY_ALT
#cmakedefine MBEDTLS_ECDSA_SIGN_ALT
#cmakedefine MBEDTLS_ECDSA_VERIFY_ALT
#cmakedefine MBEDTLS_ECJPAKE_ALT
#cmakedefine MBEDTLS_RSA_ALT
#cmakedefine MBEDTLS_SHA1_ALT
#cmakedefine MBEDTLS_SHA224_ALT
#cmakedefine MBEDTLS_SHA256_ALT
#cmakedefine MBEDTLS_SHA384_ALT
#cmakedefine MBEDTLS_SHA512_ALT

/* Legacy configuration for RNG */
#cmakedefine MBEDTLS_ENTROPY_FORCE_SHA256
#cmakedefine MBEDTLS_ENTROPY_MAX_SOURCES                        @MBEDTLS_ENTROPY_MAX_SOURCES@
#cmakedefine MBEDTLS_NO_PLATFORM_ENTROPY

/* Legacy configurations for mbed TLS APIs */
#cmakedefine MBEDTLS_CIPHER_C
//#define MBEDTLS_PK_C                                       @MBEDTLS_PK_C@
//#define MBEDTLS_PK_WRITE_C                                 @MBEDTLS_PK_WRITE_C@
#cmakedefine MBEDTLS_MD_C

/* Max curve bits for PSA APIs */
#cmakedefine PSA_VENDOR_ECC_MAX_CURVE_BITS                      @PSA_VENDOR_ECC_MAX_CURVE_BITS@


/* TLS/DTLS configurations */
#cmakedefine MBEDTLS_SSL_ALL_ALERT_MESSAGES
#cmakedefine MBEDTLS_SSL_DTLS_CONNECTION_ID
#cmakedefine MBEDTLS_SSL_CONTEXT_SERIALIZATION
#cmakedefine MBEDTLS_SSL_DEBUG_ALL
#cmakedefine MBEDTLS_SSL_ENCRYPT_THEN_MAC
#cmakedefine MBEDTLS_SSL_EXTENDED_MASTER_SECRET
#cmakedefine MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#cmakedefine MBEDTLS_SSL_RENEGOTIATION
#cmakedefine MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#cmakedefine MBEDTLS_SSL_PROTO_TLS1_2
#cmakedefine MBEDTLS_SSL_PROTO_DTLS
#cmakedefine MBEDTLS_SSL_ALPN
#cmakedefine MBEDTLS_SSL_DTLS_ANTI_REPLAY
#cmakedefine MBEDTLS_SSL_DTLS_HELLO_VERIFY
#cmakedefine MBEDTLS_SSL_DTLS_SRTP
#cmakedefine MBEDTLS_SSL_DTLS_CLIENT_PORT_REUSE
#cmakedefine MBEDTLS_SSL_SESSION_TICKETS
#cmakedefine MBEDTLS_SSL_EXPORT_KEYS
#cmakedefine MBEDTLS_SSL_SERVER_NAME_INDICATION
#cmakedefine MBEDTLS_SSL_VARIABLE_BUFFER_LENGTH
#cmakedefine MBEDTLS_SSL_CACHE_C
#cmakedefine MBEDTLS_SSL_TICKET_C
#cmakedefine MBEDTLS_SSL_CLI_C
#cmakedefine MBEDTLS_SSL_SRV_C
#cmakedefine MBEDTLS_SSL_TLS_C
#cmakedefine MBEDTLS_SSL_IN_CONTENT_LEN                         @MBEDTLS_SSL_IN_CONTENT_LEN@
#cmakedefine MBEDTLS_SSL_OUT_CONTENT_LEN                        @MBEDTLS_SSL_OUT_CONTENT_LEN@
#cmakedefine MBEDTLS_SSL_CIPHERSUITES                           @MBEDTLS_SSL_CIPHERSUITES@

#cmakedefine MBEDTLS_X509_RSASSA_PSS_SUPPORT
#cmakedefine MBEDTLS_X509_USE_C
#cmakedefine MBEDTLS_X509_CRT_PARSE_C
#cmakedefine MBEDTLS_X509_CRL_PARSE_C
#cmakedefine MBEDTLS_X509_CSR_PARSE_C
#cmakedefine MBEDTLS_X509_CREATE_C
#cmakedefine MBEDTLS_X509_CRT_WRITE_C
#cmakedefine MBEDTLS_X509_CSR_WRITE_C
#cmakedefine MBEDTLS_X509_REMOVE_INFO

#cmakedefine MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
#cmakedefine MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED

/* Controlling some MPI sizes */
#cmakedefine MBEDTLS_MPI_WINDOW_SIZE       @MBEDTLS_MPI_WINDOW_SIZE@ /**< Maximum window size used. */
#cmakedefine MBEDTLS_MPI_MAX_SIZE          @MBEDTLS_MPI_MAX_SIZE@ /**< Maximum number of bytes for usable MPIs. */

#include "psa/psa_crypto_config_oberon.h"
