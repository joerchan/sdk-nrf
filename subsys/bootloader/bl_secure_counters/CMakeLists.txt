#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

if(CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION)

if(${provision_counter_address})
zephyr_library_compile_definitions(SECURE_COUNTERS_ADDRESS=${provision_counter_address})
else()
zephyr_library_compile_definitions(SECURE_COUNTERS_ADDRESS=${PM_MCUBOOT_PROVISION_ADDRESS})
endif()

endif()

zephyr_library_sources(bl_secure_counters.c)
