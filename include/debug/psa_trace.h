/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
// #include "tfm_sp_log.h"
#include <zephyr/logging/log.h>

#define ALWAYS_INLINE inline __attribute__((always_inline))
/**
 * @brief Function to convert CRACEN SW library error codes to errno
 *
 * @param[in] sx_err	Error code from sxsymcrypt, silexpk or sicrypto.
 *
 * @return 0 or non-zero return codes from ernno.
 */

/**
 * @brief Function to convert CRACEN SW library error codes to strings for logging
 *
 * @param[in] sx_err	Error code from sxsymcrypt, silexpk or sicrypto.
 *
 * @return String literal representing the error code.
 */
// ALWAYS_INLINE inline
 const char* psa_status2str(int psa_status)
{
	switch(psa_status) {
	case PSA_SUCCESS: return "PSA_SUCCESS";
	case PSA_ERROR_GENERIC_ERROR: return "PSA_ERROR_GENERIC_ERROR";
	case PSA_ERROR_NOT_SUPPORTED: return "PSA_ERROR_NOT_SUPPORTED";
	case PSA_ERROR_NOT_PERMITTED: return "PSA_ERROR_NOT_PERMITTED";
	case PSA_ERROR_BUFFER_TOO_SMALL: return "PSA_ERROR_BUFFER_TOO_SMALL";
	case PSA_ERROR_ALREADY_EXISTS: return "PSA_ERROR_ALREADY_EXISTS";
	case PSA_ERROR_DOES_NOT_EXIST: return "PSA_ERROR_DOES_NOT_EXIST";
	case PSA_ERROR_BAD_STATE: return "PSA_ERROR_BAD_STATE";
	case PSA_ERROR_INVALID_ARGUMENT: return "PSA_ERROR_INVALID_ARGUMENT";
	case PSA_ERROR_INSUFFICIENT_MEMORY: return "PSA_ERROR_INSUFFICIENT_MEMORY";
	case PSA_ERROR_INSUFFICIENT_STORAGE: return "PSA_ERROR_INSUFFICIENT_STORAGE";
	case PSA_ERROR_COMMUNICATION_FAILURE: return "PSA_ERROR_COMMUNICATION_FAILURE";
	case PSA_ERROR_STORAGE_FAILURE: return "PSA_ERROR_STORAGE_FAILURE";
	case PSA_ERROR_HARDWARE_FAILURE: return "PSA_ERROR_HARDWARE_FAILURE";
	case PSA_ERROR_CORRUPTION_DETECTED: return "PSA_ERROR_CORRUPTION_DETECTED";
	case PSA_ERROR_INSUFFICIENT_ENTROPY: return "PSA_ERROR_INSUFFICIENT_ENTROPY";
	case PSA_ERROR_INVALID_SIGNATURE: return "PSA_ERROR_INVALID_SIGNATURE";
	case PSA_ERROR_INVALID_PADDING: return "PSA_ERROR_INVALID_PADDING";
	case PSA_ERROR_INSUFFICIENT_DATA: return "PSA_ERROR_INSUFFICIENT_DATA";
	case PSA_ERROR_INVALID_HANDLE: return "PSA_ERROR_INVALID_HANDLE";
	case PSA_ERROR_DATA_CORRUPT: return "PSA_ERROR_DATA_CORRUPT";
	case PSA_ERROR_DATA_INVALID: return "PSA_ERROR_DATA_INVALID";
	default: return "[Unknown PSA status code]";
}

#define PSA_TRACE_STATUS(status)                                                                   \
	do {                                                                                       \
		LOG_INF("%s: @ %s:%d : %s\r\n", __func__, __FILE__, __LINE__, psa_status2str(status)); \
	} while (0)

#define PSA_TRACE_ERROR(status)                                                                   \
	do {                                                                                       \
		if (status != PSA_SUCCESS) {                                                       \
			LOG_INF("%s: @ %s:%d : %d\r\n", __func__, __FILE__, __LINE__, psa_status2str(status)); \
		} \
	} while (0)
