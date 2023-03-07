/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ZEPHYR_SYS__ASSERT_H
#define __ZEPHYR_SYS__ASSERT_H

#include <autoconf.h>
#include "tfm_sp_log.h"

#ifdef CONFIG_ASSERT
/* Use same print mode as non-secure. */
#define __ASSERT_VERBOSE CONFIG_ASSERT_VERBOSE
#define __ASSERT_NO_MSG_INFO CONFIG_ASSERT_NO_MSG_INFO
#define __ASSERT_NO_COND_INFO CONFIG_ASSERT_NO_COND_INFO
#define __ASSERT_NO_FILE_INFO CONFIG_ASSERT_NO_FILE_INFO
#else
/* Set default ASSERT print mode if asserts are disabled in non-secure.
 * Calling psa_core_panic will not log any information.
 */
#define __ASSERT_VERBOSE 1
#define __ASSERT_NO_MSG_INFO 1
#define __ASSERT_NO_COND_INFO 1
#endif

#define _STRINGIFY(s) #s

#if defined(__ASSERT_VERBOSE)
#define __ASSERT_PRINT(fmt, ...)                                               \
	printf(fmt, ##__VA_ARGS__)
#else
#define __ASSERT_PRINT(fmt, ...)
#endif

#ifdef __ASSERT_NO_MSG_INFO
#define __ASSERT_MSG_INFO(fmt, ...)
#else
#define __ASSERT_MSG_INFO(fmt, ...)                                            \
	__ASSERT_PRINT("\t" fmt "\r\n", ##__VA_ARGS__)
#endif

#define __ASSERT_POST_ACTION() tfm_core_panic()
#define __ASSERT_UNREACHABLE __builtin_unreachable()

#if !defined(__ASSERT_NO_COND_INFO) && !defined(__ASSERT_NO_FILE_INFO)
#define __ASSERT_LOC(test)                                                     \
	__ASSERT_PRINT("ASSERTION FAIL [%s] @ %s:%d\r\n",                      \
		_STRINGIFY(test),                                              \
		__FILE__, __LINE__)
#endif

#if defined(__ASSERT_NO_COND_INFO) && !defined(__ASSERT_NO_FILE_INFO)
#define __ASSERT_LOC(test)                                                     \
	__ASSERT_PRINT("ASSERTION FAIL @ %s:%d\r\n",                           \
		__FILE__, __LINE__)
#endif

#if !defined(__ASSERT_NO_COND_INFO) && defined(__ASSERT_NO_FILE_INFO)
#define __ASSERT_LOC(test)                                                     \
	__ASSERT_PRINT("ASSERTION FAIL [%s]\r\n",                              \
		_STRINGIFY(test))
#endif

#if defined(__ASSERT_NO_COND_INFO) && defined(__ASSERT_NO_FILE_INFO)
#define __ASSERT_LOC(test)                                                     \
	__ASSERT_PRINT("ASSERTION FAIL\r\n")
#endif

#define __ASSERT_NO_MSG(test)                                                  \
	do {                                                                   \
		if (!(test)) {                                                 \
			__ASSERT_LOC(test);                                    \
			__ASSERT_POST_ACTION();                                \
			__ASSERT_UNREACHABLE;                                  \
		}                                                              \
	} while (false)

#define __ASSERT(test, fmt, ...)                                               \
	do {                                                                   \
		if (!(test)) {                                                 \
			__ASSERT_LOC(test);                                    \
			__ASSERT_MSG_INFO(fmt, ##__VA_ARGS__);                 \
			__ASSERT_POST_ACTION();                                \
			__ASSERT_UNREACHABLE;                                  \
		}                                                              \
	} while (false)

#endif /* __ZEPHYR_SYS__ASSERT_H */
