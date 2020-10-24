#include <zephyr.h>

#include <sys/byteorder.h>

#include <bluetooth/hci.h>
#include <bluetooth/buf.h>
#include <bluetooth/hci_err.h>
#include <drivers/bluetooth/hci_driver.h>

#include "ecdh.h"

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_HCI_DRIVER)
#define LOG_MODULE_NAME bt_ecdh
#include "common/log.h"

static struct k_thread ecdh_thread_data;
static K_KERNEL_STACK_DEFINE(ecdh_thread_stack, CONFIG_BT_CTLR_ECDH_STACK_SIZE);

static struct {
	uint8_t private_key_be[32];

	union {
		uint8_t public_key_be[64];
		uint8_t dhkey_be[32];
	};
} ecdh;

enum {
	GEN_PUBLIC_KEY,
	GEN_DHKEY,
	GEN_DHKEY_DEBUG,
};

/* based on Core Specification 4.2 Vol 3. Part H 2.3.5.6.1 */
static const uint8_t debug_private_key_be[32] = {
	0x3f, 0x49, 0xf6, 0xd4, 0xa3, 0xc5, 0x5f, 0x38,
	0x74, 0xc9, 0xb3, 0xe3, 0xd2, 0x10, 0x3f, 0x50,
	0x4a, 0xff, 0x60, 0x7b, 0xeb, 0x40, 0xb7, 0x99,
	0x58, 0x99, 0xb8, 0xa6, 0xcd, 0x3c, 0x1a, 0xbd,
};


#if defined(CONFIG_BT_CTLR_ECDH_LIB_OBERON)
#include <ocrypto_ecdh_p256.h>

static uint8_t public_key(void)
{
	int err;

	do {
		err = bt_rand(ecdh.private_key_be, 32);
		if (err) {
			return BT_HCI_ERR_UNSPECIFIED;
		}

		if (!memcmp(ecdh.private_key_be, debug_private_key_be, 32)) {
			err = -1;
			continue;
		}

		err = ocrypto_ecdh_p256_public_key(ecdh.public_key_be,
						   ecdh.private_key_be);
	} while (err);

	return 0;
}

static uint8_t common_secret(bool use_debug)
{
	int err;

	err = ocrypto_ecdh_p256_common_secret(ecdh.dhkey_be,
					      use_debug ? debug_private_key_be :
							  ecdh.private_key_be,
					      ecdh.public_key_be);
	/* -1: public or private key was not a valid key */
	if (err) {
		/* If the remote P-256 public key is invalid
		 * (see [Vol 3] Part H, Section 2.3.5.6.1), the Controller shall
		 * return an error and should use the error code
		 * Invalid HCI Command Parameters (0x12).
		 */
		BT_ERR("public key is not valid (err %d)", err);
		return BT_HCI_ERR_INVALID_PARAM;
	}

	return 0;
}
#endif /* defined(BT_CTLR_ECDH_LIB_OBERON) */

#if defined(CONFIG_BT_CTLR_ECDH_LIB_TINYCRYPT)
#include <tinycrypt/constants.h>
#include <tinycrypt/utils.h>
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dh.h>

int default_CSPRNG(uint8_t *dst, unsigned int len)
{
	return !bt_rand(dst, len);
}

static uint8_t public_key(void)
{
	do {
		int rc;

		rc = uECC_make_key(ecdh.public_key, ecdh.private_key,
				   &curve_secp256r1);
		if (rc == TC_CRYPTO_FAIL) {
			BT_ERR("Failed to create ECC public/private pair");
			return BT_HCI_ERR_UNSPECIFIED;
		}

		/* make sure generated key isn't debug key */
	} while (memcmp(ecdh.private_key_be, debug_private_key_be, 32) == 0);

	return 0;
}

static uint8_t common_secret(bool use_debug)
{
	int err;

	err = uECC_valid_public_key(ecdh.public_key, &curve_secp256r1);
	if (err < 0) {
		/* If the remote P-256 public key is invalid
		 * (see [Vol 3] Part H, Section 2.3.5.6.1), the Controller shall
		 * return an error and should use the error code
		 * Invalid HCI Command Parameters (0x12).
		 */
		BT_ERR("public key is not valid (err %d)", err);
		return BT_HCI_ERR_INVALID_PARAM;
	}

	err = uECC_shared_secret(ecdh.public_key,
				 use_debug ? debug_private_key_be :
					     ecdh.private_key,
				 ecdh.dhkey,
				 &curve_secp256r1);

	if (err == TC_CRYPTO_FAIL) {
		return BT_HCI_ERR_UNSPECIFIED;
	}

	return 0;
}
#endif /* defined(BT_CTLR_ECDH_LIB_TINYCRYPT) */

static struct net_buf *ecdh_p256_public_key(void)
{
	struct bt_hci_evt_le_p256_public_key_complete *evt;
	struct bt_hci_evt_le_meta_event *meta;
	struct bt_hci_evt_hdr *hdr;
	struct net_buf *buf;
	uint8_t status;

	int64_t uptime, delta;
	uptime = k_uptime_get();

	status = public_key();
	delta = k_uptime_delta(&uptime);
	BT_WARN("public key %d ms\n", (int32_t)delta);

	buf = bt_buf_get_rx(BT_BUF_EVT, K_FOREVER);

	hdr = net_buf_add(buf, sizeof(*hdr));
	hdr->evt = BT_HCI_EVT_LE_META_EVENT;
	hdr->len = sizeof(*meta) + sizeof(*evt);

	meta = net_buf_add(buf, sizeof(*meta));
	meta->subevent = BT_HCI_EVT_LE_P256_PUBLIC_KEY_COMPLETE;

	evt = net_buf_add(buf, sizeof(*evt));
	evt->status = status;

	if (status){
		(void)memset(evt->key, 0, sizeof(evt->key));
	} else {
	        /* Reverse X */
		sys_memcpy_swap(&evt->key[0], &ecdh.public_key_be[0], 32);
	        /* Reverse Y */
		sys_memcpy_swap(&evt->key[32], &ecdh.public_key_be[32], 32);
	}

	return buf;
}

static struct net_buf *ecdh_p256_common_secret(bool use_debug)
{
	struct bt_hci_evt_le_generate_dhkey_complete *evt;
	struct bt_hci_evt_le_meta_event *meta;
	struct bt_hci_evt_hdr *hdr;
	struct net_buf *buf;
	uint8_t status;

	int64_t uptime, delta;
	uptime = k_uptime_get();

	status = common_secret(use_debug);
	delta = k_uptime_delta(&uptime);
	BT_WARN("common secret %d ms\n", (int32_t)delta);

	buf = bt_buf_get_rx(BT_BUF_EVT, K_FOREVER);

	hdr = net_buf_add(buf, sizeof(*hdr));
	hdr->evt = BT_HCI_EVT_LE_META_EVENT;
	hdr->len = sizeof(*meta) + sizeof(*evt);

	meta = net_buf_add(buf, sizeof(*meta));
	meta->subevent = BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE;

	evt = net_buf_add(buf, sizeof(*evt));
	evt->status = status;

	if (status) {
		memset(evt->dhkey, 0xff, sizeof(evt->dhkey));
	} else {
		sys_memcpy_swap(evt->dhkey, ecdh.dhkey_be,
				sizeof(ecdh.dhkey_be));
	}

	return buf;
}

struct k_poll_signal ecdh_signal;
void ecdh_signal_handle(struct k_poll_event *event)
{
	struct net_buf *buf;

	switch (event->signal->result) {
	case GEN_PUBLIC_KEY:
		buf = ecdh_p256_public_key();
		break;
	case GEN_DHKEY:
		buf = ecdh_p256_common_secret(false);
		break;
	case GEN_DHKEY_DEBUG:
		buf = ecdh_p256_common_secret(true);
		break;
	default:
		BT_WARN("Unknown HCI command 0x%04x", event->signal->result);
		buf = NULL;
		break;
	}

	event->signal->signaled = 0;
	event->state = K_POLL_STATE_NOT_READY;

	if (buf) {
		bt_recv(buf);
	}
}

static void ecdh_thread(void *p1, void *p2, void *p3)
{
	struct k_poll_event events[1] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
					 K_POLL_MODE_NOTIFY_ONLY,
					 &ecdh_signal),
	};

	while (true) {
		k_poll(events, 1, K_FOREVER);
		ecdh_signal_handle(&events[0]);
	}
}

void hci_ecdh_init(void)
{
	k_poll_signal_init(&ecdh_signal);

	k_thread_create(&ecdh_thread_data, ecdh_thread_stack,
			K_KERNEL_STACK_SIZEOF(ecdh_thread_stack), ecdh_thread,
			NULL, NULL, NULL, K_PRIO_PREEMPT(10), 0, K_NO_WAIT);
	k_thread_name_set(&ecdh_thread_data, "BT CTLR ECDH");
}

uint8_t hci_cmd_le_read_local_p256_public_key(void)
{
	unsigned int signaled;
	int result;

	k_poll_signal_check(&ecdh_signal, &signaled, &result);

	if (signaled) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	k_poll_signal_raise(&ecdh_signal, GEN_PUBLIC_KEY);

	return 0;
}

uint8_t cmd_le_generate_dhkey(uint8_t *key, uint8_t key_type)
{
	unsigned int signaled;
	int result;

	k_poll_signal_check(&ecdh_signal, &signaled, &result);

	if (signaled) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	sys_memcpy_swap(&ecdh.public_key_be[0], &key[0], 32);
	sys_memcpy_swap(&ecdh.public_key_be[32], &key[32], 32);

	k_poll_signal_raise(&ecdh_signal, key_type ? GEN_DHKEY_DEBUG :
						     GEN_DHKEY);

	return 0;
}

uint8_t hci_cmd_le_generate_dhkey(struct bt_hci_cp_le_generate_dhkey *p_params)
{
	return cmd_le_generate_dhkey(p_params->key,
				     BT_HCI_LE_KEY_TYPE_GENERATED);
}

uint8_t hci_cmd_le_generate_dhkey_v2(
	struct bt_hci_cp_le_generate_dhkey_v2 *p_params)
{
	if (p_params->key_type > BT_HCI_LE_KEY_TYPE_DEBUG) {
		return BT_HCI_ERR_INVALID_PARAM;
	}

	return cmd_le_generate_dhkey(p_params->key, p_params->key_type);
}
