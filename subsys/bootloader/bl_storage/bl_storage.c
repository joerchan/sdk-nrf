/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bl_storage.h"
#include "bl_secure_counters.h"
#include <string.h>
#include <errno.h>
#include <nrf.h>
#include <assert.h>
#include <pm_config.h>
#include <nrfx_nvmc.h>


/** The first data structure in the bootloader storage. It has unknown length
 *  since 'key_data' is repeated. This data structure is immediately followed by
 *  struct counter_collection.
 */
struct bl_storage_data {
	uint32_t s0_address;
	uint32_t s1_address;
	uint32_t num_public_keys; /* Number of entries in 'key_data' list. */
	struct {
		uint32_t valid;
		uint8_t hash[CONFIG_SB_PUBLIC_KEY_HASH_LEN];
	} key_data[1];
};

static const struct bl_storage_data *p_bl_storage_data =
	(struct bl_storage_data *)PM_PROVISION_ADDRESS;

uint32_t s0_address_read(void)
{
	uint32_t addr = p_bl_storage_data->s0_address;

	__DSB(); /* Because of nRF9160 Erratum 7 */
	return addr;
}

uint32_t s1_address_read(void)
{
	uint32_t addr = p_bl_storage_data->s1_address;

	__DSB(); /* Because of nRF9160 Erratum 7 */
	return addr;
}

uint32_t num_public_keys_read(void)
{
	uint32_t num_pk = p_bl_storage_data->num_public_keys;

	__DSB(); /* Because of nRF9160 Erratum 7 */
	return num_pk;
}

struct counter_collection *bl_storage_get_counter_collection(void)
{
	return (struct counter_collection *) &p_bl_storage_data->key_data[num_public_keys_read()];
}

/* Value written to the invalidation token when invalidating an entry. */
#define INVALID_VAL 0xFFFF0000

static bool key_is_valid(uint32_t key_idx)
{
	bool ret = (p_bl_storage_data->key_data[key_idx].valid != INVALID_VAL);

	__DSB(); /* Because of nRF9160 Erratum 7 */
	return ret;
}

int verify_public_keys(void)
{
	for (uint32_t n = 0; n < num_public_keys_read(); n++) {
		if (key_is_valid(n)) {
			const uint16_t *p_key_n = (const uint16_t *)
					p_bl_storage_data->key_data[n].hash;
			size_t hash_len_u16 = (CONFIG_SB_PUBLIC_KEY_HASH_LEN/2);

			for (uint32_t i = 0; i < hash_len_u16; i++) {
				if (nrfx_nvmc_otp_halfword_read((uint32_t)(&p_key_n[i])) == 0xFFFF) {
					return -EHASHFF;
				}
			}
		}
	}
	return 0;
}

/**
 * Helper procedure for public_key_data_read()
 *
 * Copies @p src into @p dst. Reads from @p src are done 32 bits at a
 * time. Writes to @p dst are done a byte at a time.
 *
 * @param dst destination buffer
 * @param src source buffer
 * @param size number of *bytes* in src to copy into dst
 */
static void public_key_copy32(uint8_t *dst, const uint32_t *src, size_t size)
{
	while (size) {
		uint32_t val = *src++;

		*dst++ = val & 0xFF;
		*dst++ = (val >> 8U) & 0xFF;
		*dst++ = (val >> 16U) & 0xFF;
		*dst++ = (val >> 24U) & 0xFF;

		size -= 4;
	}
}

int public_key_data_read(uint32_t key_idx, uint8_t *p_buf, size_t buf_size)
{
	const uint8_t *p_key;

	if (!key_is_valid(key_idx)) {
		return -EINVAL;
	}

	if (buf_size < CONFIG_SB_PUBLIC_KEY_HASH_LEN) {
		return -ENOMEM;
	}

	if (key_idx >= num_public_keys_read()) {
		return -EFAULT;
	}

	p_key = p_bl_storage_data->key_data[key_idx].hash;

	/* Ensure word alignment, since the data is stored in memory region
	 * with word sized read limitation. Perform both build time and run
	 * time asserts to catch the issue as soon as possible.
	 */
	BUILD_ASSERT(CONFIG_SB_PUBLIC_KEY_HASH_LEN % 4 == 0);
	BUILD_ASSERT(offsetof(struct bl_storage_data, key_data) % 4 == 0);
	__ASSERT(((uint32_t)p_key % 4 == 0), "Key address is not word aligned");

	public_key_copy32(p_buf, (const uint32_t *)p_key, CONFIG_SB_PUBLIC_KEY_HASH_LEN);
	__DSB(); /* Because of nRF9160 Erratum 7 */

	return CONFIG_SB_PUBLIC_KEY_HASH_LEN;
}

void invalidate_public_key(uint32_t key_idx)
{
	const uint32_t *invalidation_token =
			&p_bl_storage_data->key_data[key_idx].valid;

	if (*invalidation_token != INVALID_VAL) {
		/* Write if not already written. */
		__DSB(); /* Because of nRF9160 Erratum 7 */
		nrfx_nvmc_word_write((uint32_t)invalidation_token, INVALID_VAL);
	}
}
