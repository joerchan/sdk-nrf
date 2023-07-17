/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 */

#ifndef NRF_CONFIG_H
#define NRF_CONFIG_H

#if defined(MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#else
#error "MBEDTLS_USER_CONFIG_FILE expected to be set"
#endif

#define MBEDTLS_PSA_CRYPTO_CONFIG

#if defined(MBEDTLS_PSA_CRYPTO_CONFIG)
#if defined(MBEDTLS_PSA_CRYPTO_CONFIG_FILE)
#include MBEDTLS_PSA_CRYPTO_CONFIG_FILE
#else
#error "MBEDTLS_PSA_CRYPTO_CONFIG_FILE expected to be set"
#endif
#endif /* defined(MBEDTLS_PSA_CRYPTO_CONFIG) */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_MBEDTLS_DEBUG)
#define MBEDTLS_ERROR_C
#define MBEDTLS_DEBUG_C
#define MBEDTLS_SSL_DEBUG_ALL
#endif

#ifdef __cplusplus
}
#endif

#endif /* NRF_CONFIG_H */
