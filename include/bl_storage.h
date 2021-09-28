/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BL_STORAGE_H_
#define BL_STORAGE_H_

#include <zephyr/types.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#define EHASHFF 113 /* A hash contains too many 0xFs. */

/** @defgroup bl_storage Bootloader storage (protected data).
 * @{
 */

/**
 * @brief Function for reading address of slot 0.
 *
 * @return Address of slot 0.
 */
uint32_t s0_address_read(void);

/**
 * @brief Function for reading address of slot 1.
 *
 * @return Address of slot 1.
 */
uint32_t s1_address_read(void);

/**
 * @brief Function for reading number of public key data slots.
 *
 * @return Number of public key data slots.
 */
uint32_t num_public_keys_read(void);

/**
 * @brief Function for reading number of public key data slots.
 *
 * @retval 0         if all keys are ok
 * @retval -EHASHFF  if one or more keys contains an aligned 0xFFFF.
 */
int verify_public_keys(void);

/**
 * @brief Function for reading public key data.
 *
 * @param[in]  key_idx  Index of key.
 * @param[out] p_buf    Pointer to area where key data will be stored.
 * @param[in]  buf_size Size of buffer, in bytes, provided in p_buf.
 *
 * @return Number of bytes written to p_buf is successful.
 * @retval -EINVAL  Key has been invalidated.
 * @retval -ENOMEM  The provided buffer is too small.
 * @retval -EFAULT  key_idx is too large. There is no key with that index.
 */
int public_key_data_read(uint32_t key_idx, uint8_t *p_buf, size_t buf_size);

/**
 * @brief Function for invalidating a public key.
 *
 * The public key will no longer be returned by @ref public_key_data_read.
 *
 * @param[in]  key_idx  Index of key.
 */
void invalidate_public_key(uint32_t key_idx);

/**
 * @brief Function for getting the counter collection pointer used by the immutable bootloader.
 *
 * The counter_collection instance stored after the provision partition.
 *
 */
struct counter_collection *bl_storage_get_counter_collection(void);


  /** @} */

#ifdef __cplusplus
}
#endif

#endif /* BL_STORAGE_H_ */
