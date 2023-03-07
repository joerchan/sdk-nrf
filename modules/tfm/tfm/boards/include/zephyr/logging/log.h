/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ZEPHYR_LOGGING_LOG_H__
#define __ZEPHYR_LOGGING_LOG_H__

#include "tfm_sp_log.h"

#define LOG_MODULE_DECLARE(...)
#define LOG_MODULE_REGISTER(...)

#define LOG_ERR(...) LOG_ERRFMT(__VA_ARGS__)
#define LOG_WRN(...) LOG_WRNFMT(__VA_ARGS__)
#define LOG_INF(...) LOG_INFFMT(__VA_ARGS__)
#define LOG_DBG(...) LOG_DBGFMT(__VA_ARGS__)

/* TODO support hexdump logging macros. */
#define LOG_HEXDUMP_ERR(...) (void) 0
#define LOG_HEXDUMP_WRN(...) (void) 0
#define LOG_HEXDUMP_DBG(...) (void) 0
#define LOG_HEXDUMP_INF(...) (void) 0

#endif /* __ZEPHYR_LOGGING_LOG_H__ */
