/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ZEPHYR_SYS_CHECK_H
#define __ZEPHYR_SYS_CHECK_H

#include <autoconf.h>
#include <zephyr/sys/__assert.h>

/* Note:
 * CONFIG_NO_RUNTIME_CHECKS is not implemented for TF-M.
 * We don't allow exclusion of these checks.
 */
#if defined(CONFIG_ASSERT_ON_ERRORS)
#define CHECKIF(expr)               \
	__ASSERT_NO_MSG(!(expr));   \
	if (0)
#else
#define CHECKIF(expr)               \
	if (expr)
#endif


#endif /* ZEPHYR_INCLUDE_SYS_CHECK_H_ */

#endif /* __ZEPHYR_SYS_CHECK_H */
