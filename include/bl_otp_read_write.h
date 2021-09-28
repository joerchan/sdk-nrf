/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bl_storage.h"
#include <string.h>
#include <errno.h>
#include <nrf.h>
#include <assert.h>
#include <nrfx_nvmc.h>

/** Function for reading a half-word (2 byte value) from OTP.
 *
 * @details The flash in OTP supports only aligned 4 byte read operations.
 *          This function reads the encompassing 4 byte word, and returns the
 *          requested half-word.
 *
 * @param[in]  ptr  Address to read.
 *
 * @return value
 */
uint16_t otp_read_halfword(const uint16_t *ptr);


/** Function for writing a half-word (2 byte value) to OTP.
 *
 * @details The flash in OTP supports only aligned 4 byte write operations.
 *          This function writes to the encompassing 4 byte word, masking the
 *          other half-word with 0xFFFF so it is left untouched.
 *
 * @param[in]  ptr  Address to write to.
 * @param[in]  val  Value to write into @p ptr.
 */
void otp_write_halfword(const uint16_t *ptr, uint16_t val);