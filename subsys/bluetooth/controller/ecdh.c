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

#define ECDH_STACK_SIZE 1100
static struct k_thread ecc_thread_data;
static K_KERNEL_STACK_DEFINE(ecc_thread_stack, ECDH_STACK_SIZE);

static struct {
	uint8_t private_key[32];

	union {
		uint8_t public_key[64];
		uint8_t dhkey[32];
	};
} ecdh;

/* based on Core Specification 4.2 Vol 3. Part H 2.3.5.6.1 */
static const uint32_t debug_private_key[8] = {
	0xcd3c1abd, 0x5899b8a6, 0xeb40b799, 0x4aff607b, 0xd2103f50, 0x74c9b3e3,
	0xa3c55f38, 0x3f49f6d4
};

#if defined(CONFIG_BT_USE_DEBUG_KEYS)
static uint8_t public_key(void)
{
	static const uint8_t debug_public_key[64] = {
		/* X */
		0xe6, 0x9d, 0x35, 0x0e, 0x48, 0x01, 0x03, 0xcc,
		0xdb, 0xfd, 0xf4, 0xac, 0x11, 0x91, 0xf4, 0xef,
		0xb9, 0xa5, 0xf9, 0xe9, 0xa7, 0x83, 0x2c, 0x5e,
		0x2c, 0xbe, 0x97, 0xf2, 0xd2, 0x03, 0xb0, 0x20,
		/* Y */
		0x8b, 0xd2, 0x89, 0x15,	0xd0, 0x8e, 0x1c, 0x74,
		0x24, 0x30, 0xed, 0x8f, 0xc2, 0x45, 0x63, 0x76,
		0x5c, 0x15, 0x52, 0x5a, 0xbf, 0x9a, 0x32, 0x63,
		0x6d, 0xeb, 0x2a, 0x65,	0x49, 0x9c, 0x80, 0xdc
	};

	sys_memcpy_swap(&ecc.pk, debug_public_key, 32);
	sys_memcpy_swap(&ecc.pk[32], &debug_public_key[32], 32);
	sys_memcpy_swap(ecc.private_key, debug_private_key, 32);
}
#endif

#if defined(CONFIG_BT_CTLR_ECDH_LIB_OBERON)
#include <ocrypto_ecdh_p256.h>

#if !defined(CONFIG_BT_USE_DEBUG_KEYS)
static uint8_t public_key(void)
{
	int err;

	do {
		err = bt_rand(ecdh.private_key, 32);
		if (err) {
			return BT_HCI_ERR_UNSPECIFIED;
		}

		if (memcmp(ecdh.private_key, debug_private_key, 32) == 0) {
			err = -1;
			continue;
		}

		err = ocrypto_ecdh_p256_public_key(ecdh.public_key,
						   ecdh.private_key);
	} while (err == -1);

	return 0;
}
#endif /* !defined(CONFIG_BT_USE_DEBUG_KEYS) */

static uint8_t common_secret(void)
{
	int err;

	err = ocrypto_ecdh_p256_common_secret(ecdh.dhkey,
					      ecdh.private_key,
					      ecdh.public_key);
	/* -1: public or private key was not a valid key */
	if (err == -1) {
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

#if !defined(CONFIG_BT_USE_DEBUG_KEYS)
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
	} while (memcmp(ecdh.private_key, debug_private_key, 32) == 0);

	return 0;
}
#endif /* !defined(CONFIG_BT_USE_DEBUG_KEYS) */

static uint8_t common_secret(void)
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

	err = uECC_shared_secret(ecdh.public_key, ecdh.private_key, ecdh.dhkey,
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
		sys_memcpy_swap(&evt->key[0], &ecdh.public_key[0], 32);
	        /* Reverse Y */
		sys_memcpy_swap(&evt->key[32], &ecdh.public_key[32], 32);
	}

	return buf;
}

static struct net_buf *ecdh_p256_common_secret(void)
{
	struct bt_hci_evt_le_generate_dhkey_complete *evt;
	struct bt_hci_evt_le_meta_event *meta;
	struct bt_hci_evt_hdr *hdr;
	struct net_buf *buf;
	uint8_t status;

	int64_t uptime, delta;
	uptime = k_uptime_get();

	status = common_secret();
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
		sys_memcpy_swap(evt->dhkey, ecdh.dhkey, sizeof(ecdh.dhkey));
	}

	return buf;
}

struct k_poll_signal ecdh_signal;
static void ecc_thread(void *p1, void *p2, void *p3)
{
	k_poll_signal_init(&ecdh_signal);

	struct k_poll_event events[1] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
					 K_POLL_MODE_NOTIFY_ONLY,
					 &ecdh_signal),
	};

	while (true) {
		struct net_buf *buf;

		k_poll(events, 1, K_FOREVER);

		switch (events[0].signal->result) {
		case BT_HCI_OP_LE_P256_PUBLIC_KEY:
			buf = ecdh_p256_public_key();
			break;
		case BT_HCI_OP_LE_GENERATE_DHKEY:
			buf = ecdh_p256_common_secret();
			break;
		default:
			BT_WARN("Unknown HCI command 0x%04x",
				events[0].signal->result);
			buf = NULL;
			break;
		}

		events[0].signal->signaled = 0;
        	events[0].state = K_POLL_STATE_NOT_READY;

        	if (buf) {
        		bt_recv(buf);
        	}
	}
}

void hci_ecdh_init(void)
{
	k_thread_create(&ecc_thread_data, ecc_thread_stack,
			K_KERNEL_STACK_SIZEOF(ecc_thread_stack), ecc_thread,
			NULL, NULL, NULL, K_PRIO_PREEMPT(10), 0, K_NO_WAIT);
	k_thread_name_set(&ecc_thread_data, "BT CTLR ECDH");
}

uint8_t hci_cmd_le_read_local_p256_public_key(void)
{
	unsigned int signaled;
	int result;

	k_poll_signal_check(&ecdh_signal, &signaled, &result);

	if (signaled) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	k_poll_signal_raise(&ecdh_signal, BT_HCI_OP_LE_P256_PUBLIC_KEY);

	return 0;
}

uint8_t hci_cmd_le_generate_dhkey(struct bt_hci_cp_le_generate_dhkey *p_params)
{
	unsigned int signaled;
	int result;

	k_poll_signal_check(&ecdh_signal, &signaled, &result);

	if (signaled) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	sys_memcpy_swap(&ecdh.public_key[0], &p_params->key[0], 32);
	sys_memcpy_swap(&ecdh.public_key[32], &p_params->key[32], 32);

	k_poll_signal_raise(&ecdh_signal, BT_HCI_OP_LE_GENERATE_DHKEY);

	return 0;
}
