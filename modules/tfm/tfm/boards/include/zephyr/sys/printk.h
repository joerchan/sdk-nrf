/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ZEPHYR_SYS_PRINTK_H
#define __ZEPHYR_SYS_PRINTK_H

#include "tfm_sp_log.h"

#define printk(fmt, ...) printf(fmt, ##__VA_ARGS__)

#endif /* __ZEPHYR_SYS_PRINTK_H */
