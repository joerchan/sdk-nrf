#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include <shell/shell_uart.h>

int test_run(const struct shell *shell,
	     const struct bt_le_conn_param *conn_param,
	     const struct bt_conn_le_phy_param *phy,
	     const struct bt_conn_le_data_len_param *data_len);

void scan_start(void);

void adv_start(void);
