/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bl_storage.h"
#include "bl_secure_counters.h"
#include "bl_otp_read_write.h"
#include <string.h>
#include <errno.h>
#include <nrf.h>
#include <assert.h>
#include <pm_config.h>
#include <nrfx_nvmc.h>

/* Value written to the invalidation token when invalidating an entry. */
#define INVALID_VAL 0xFFFF0000

/** Get the counter_collection data structure in the provision data. */
static const struct counter_collection *get_counter_collection(uint16_t counter_desc)
{
	struct counter_collection *collection;

	switch (counter_desc)
	{

	case COUNTER_DESC_VERSION:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID0:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID1:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID2:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID3:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID4:
	case COUNTER_DESC_MCUBOOT_HW_COUNTER_ID5:
#ifdef CONFIG_SECURE_BOOT_STORAGE
		collection = (struct counter_collection *) bl_storage_get_counter_collection();
		break;
#elif CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION
		collection =  (struct counter_collection *) CONFIG_SECURE_COUNTERS_ADDRESS;
		break;
#else
		return NULL;
		break;
#endif
	default:
		return NULL;
		break;
	}

	return otp_read_halfword(&collection->type) == TYPE_COUNTERS
		? collection : NULL;
}


/** Get one of the (possibly multiple) counters in the provision data.
 *
 *  param[in]  description  Which counter to get. See COUNTER_DESC_*.
 */
static const struct monotonic_counter *get_counter_struct(uint16_t counter_desc)
{
	const struct counter_collection *counters = get_counter_collection(counter_desc);

	if (counters == NULL) {
		return NULL;
	}

	const struct monotonic_counter *current = counters->counters;

	for (size_t i = 0; i < otp_read_halfword(&counters->num_counters); i++) {
		uint16_t num_slots = otp_read_halfword(&current->num_counter_slots);

		if (otp_read_halfword(&current->description) == counter_desc) {
			return current;
		}

		current = (const struct monotonic_counter *)
					&current->counter_slots[num_slots];
	}
	return NULL;
}


uint16_t num_monotonic_counter_slots(uint16_t counter_desc)
{
	const struct monotonic_counter *counter
			= get_counter_struct(counter_desc);
	uint16_t num_slots = 0;

	if (counter != NULL) {
		num_slots = otp_read_halfword(&counter->num_counter_slots);
	}
	return num_slots != 0xFFFF ? num_slots : 0;
}


/** Function for getting the current value and the first free slot.
 *
 * @param[out]  free_slot  Pointer to the first free slot. Can be NULL.
 *
 * @return The current value of the counter (the highest value before the first
 *         free slot).
 */
static uint16_t get_counter(uint16_t counter_desc, const uint16_t **free_slot)
{
	uint16_t highest_counter = 0;
	const uint16_t *addr = NULL;
	const struct monotonic_counter *counter_obj = get_counter_struct(counter_desc);
	const uint16_t *slots;
	uint16_t num_slots;

	if (counter_obj == NULL){
		goto end;
	}

	slots = counter_obj->counter_slots;
	num_slots = num_monotonic_counter_slots(counter_desc);

	for (uint32_t i = 0; i < num_slots; i++) {
		uint16_t counter = ~otp_read_halfword(&slots[i]);

		if (counter == 0) {
			addr = &slots[i];
			break;
		}
		if (highest_counter < counter) {
			highest_counter = counter;
		}
	}

	end:
	if (free_slot != NULL) {
		*free_slot = addr;
	}

	return highest_counter;
}


uint16_t get_monotonic_counter(uint16_t counter_desc)
{
	const uint16_t *next_counter_addr;
	uint16_t counter = get_counter(counter_desc, &next_counter_addr);

	/* The counter coundn't be found */
	if (next_counter_addr == NULL || counter == 0xFFFF) {
		/* No more room. */
		return 0xFFFF;
	}

	return (counter);
}


int set_monotonic_counter(uint16_t counter_desc, uint16_t new_counter)
{
	const uint16_t *next_counter_addr;
	uint16_t counter = get_counter(counter_desc, &next_counter_addr);

	if (new_counter <= counter) {
		/* Counter value must increase. */
		return -EINVAL;
	}

	if (next_counter_addr == NULL) {
		/* No more room. */
		return -ENOMEM;
	}

	/* 0 is not a valid number for the counter */
	otp_write_halfword(next_counter_addr, ~new_counter);
	return 0;
}
