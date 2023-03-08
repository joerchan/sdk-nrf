/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ZEPHYR_KERNEL_H
#define __ZEPHYR_KERNEL_H

#include <zephyr/sys/printk.h>

#define k_panic() tfm_core_panic()

#endif /* __ZEPHYR_KERNEL_H */
