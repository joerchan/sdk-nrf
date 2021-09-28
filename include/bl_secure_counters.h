/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bl_storage.h"
#include <string.h>
#include <errno.h>
#include <nrf.h>
#include <assert.h>
#include <pm_config.h>
#include <nrfx_nvmc.h>

/** A single monotonic counter. It consists of a description value, a 'type',
 *  to know what this counter counts. Further, it has a number of slots
 *  allocated to it. Each time the counter is updated, a new slot is written.
 *  This way, the counter can be updated without erasing anything. This is
 *  necessary so the counter can be used in the OTP section of the UICR
 *  (available on e.g. nRF91 and nRF53).
 */
struct monotonic_counter {
	uint16_t description; /* Counter description. What the counter is used for. See COUNTER_DESC_*. */
	uint16_t num_counter_slots; /* Number of entries in 'counter_slots' list. */
	uint16_t counter_slots[1];
};

/** The second data structure in the provision page. It has unknown length since
 *  'counters' is repeated. Note that each entry in counters also has unknown
 *  length, and each entry can have different length from the others, so the
 *  entries beyond the first cannot be accessed via array indices.
 */
struct counter_collection {
	uint16_t type;         /* Must be "monotonic counter". */
	uint16_t num_counters; /* Number of entries in 'counters' list. */
	struct monotonic_counter counters[1];
};

#define TYPE_COUNTERS 1                              /* Type referring to counter collection. */
#define COUNTER_DESC_VERSION 0xA5B5                  /* Counter description value for firmware version. */
#define COUNTER_DESC_MCUBOOT_HW_COUNTERS 0x5C5D      /* Counter description value for mcuboot hw counters. */

/* Value written to the invalidation token when invalidating an entry. */
#define INVALID_VAL 0xFFFF0000

/**
 * @brief Get the number of monotonic counter slots.
 *
 * @param[in]   counter_desc Counter description
 *
 * @return The number of slots. If the provision page does not contain the
 *         information, 0 is returned.
 */
uint16_t num_monotonic_counter_slots(uint16_t counter_desc);

/** Function for getting the current value of the monotonic counter.
 *
 * @param[in]   counter_desc Counter description
 *
 * @return The current value of the counter (the highest value before the first
 *         free slot).
 */
uint16_t get_monotonic_counter(uint16_t counter_desc);


/** Function for setting the current value of the monotonic counter.
 *
 * @param[in]   counter_desc Counter description
 * @param[in]   new_counter  New counter value
 *
 * @retval 0 If successful.
 * @retval -EINVAL If the counter value is lower than the current one.
 * @retval -ENOMEM If there is no more space to save a counter.
 */
int set_monotonic_counter(uint16_t counter_desc, uint16_t new_counter);
