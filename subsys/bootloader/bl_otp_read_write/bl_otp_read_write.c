/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bl_storage.h"
#include "bl_otp_read_write.h"
#include <string.h>
#include <errno.h>
#include <nrf.h>
#include <assert.h>
#include <stdbool.h>
#include <nrfx_nvmc.h>

uint16_t otp_read_halfword(const uint16_t *ptr)
{
	bool top_half = ((uint32_t)ptr % 4); /* Addr not div by 4 */
	uint32_t target_addr = (uint32_t)ptr & ~3; /* Floor address */
	uint32_t val32 = *(uint32_t *)target_addr;
	__DSB(); /* Because of nRF9160 Erratum 7 */

	return (top_half ? (val32 >> 16) : val32) & 0x0000FFFF;
}

void otp_write_halfword(const uint16_t *ptr, uint16_t val)
{
	bool top_half = (uint32_t)ptr % 4; /* Addr not div by 4 */
	uint32_t target_addr = (uint32_t)ptr & ~3; /* Floor address */

	uint32_t val32 = (uint32_t)val | 0xFFFF0000;
	uint32_t val32_shifted = ((uint32_t)val << 16) | 0x0000FFFF;

	nrfx_nvmc_word_write(target_addr, top_half ? val32_shifted : val32);
}
