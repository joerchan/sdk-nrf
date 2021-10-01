#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause


from intelhex import IntelHex

import argparse
import struct
import os


def parse_args():
    parser = argparse.ArgumentParser(
        description='Generate the hex file containing the secure counters used by MCUBOOT.',
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--secure-cnt-addr', type=lambda x: int(x, 0),
                        required=True, help='Address at which to place the secure counters')
    parser.add_argument('-o', '--output', required=True,
                        help='Output file name.')
    parser.add_argument('--max-size', required=False, type=lambda x: int(x, 0), default=0x1000,
                        help='Maximum total size of the provision data, including the counter slots.')
    parser.add_argument('--mcuboot-num-counters', required=True, type=int,
                        help='Number of monotonic counters for mcuboot')
    parser.add_argument('--mcuboot-counter-slots', required=True, type=int,
                        help='Number of slots asigned to each mcuboot counter.')
    parser.add_argument('--b0-provision-hex', required=False,
                        help='The path to the hex containing provision data for B0')
    parser.add_argument('--b0-provision-counter-slots', required=False, type=int,
                        help='Number of slots asigned to the B0 counter.')
    return parser.parse_args()

def main():
    args = parse_args()
    provision_hex_path = args.b0_provision_hex
    provision_counter_slots =  args.b0_provision_counter_slots
    mcuboot_num_counters = args.mcuboot_num_counters
    mcuboot_counter_slots = args.mcuboot_counter_slots
    max_size = args.max_size
    secure_cnt_addr = args.secure_cnt_addr
    output = args.output

    provision_counter_present=False

    if(provision_hex_path != None):
        provision_counter_present = True

    all_counters = mcuboot_num_counters if not provision_counter_present else mcuboot_num_counters + 1
    sec_cnt_data = struct.pack('H', 1) # Type "counter collection"
    sec_cnt_data += struct.pack('H', all_counters)


    if(provision_counter_present):
        if provision_counter_slots != None:
            if provision_counter_slots % 2 == 1:
                provision_counter_slots += 1
                print(f'Monotonic counter slots rounded up to ${provision_counter_slots}')
        else:
            provision_counter_slots = 0


        provision_counter_slots = 2
        sec_cnt_data += struct.pack('H', 0x10 ) # == COUNTER_DESC
        sec_cnt_data += struct.pack('H', provision_counter_slots)
        sec_cnt_data += bytes(2 * provision_counter_slots * [0xFF])

    for cnt_desc in range(0, mcuboot_num_counters):
        mcuboot_counter_slots = 2
        sec_cnt_data += struct.pack('H', cnt_desc) # == COUNTER_DESC_MCUBOOT_HW_COUNTER_ID*
        sec_cnt_data += struct.pack('H', mcuboot_counter_slots)
        sec_cnt_data += bytes(2 * mcuboot_counter_slots * [0xFF])

    if(provision_hex_path != None):
        provision_file = IntelHex()
        provision_file.loadhex(provision_hex_path)
        provision_len = provision_file.maxaddr() - provision_file.minaddr()

        assert len(sec_cnt_data) + provision_len <= max_size, """Secure counters dont't fit.
            Reduce the number of counter types or counter slots and try again."""

        ih = IntelHex()
        ih.frombytes(sec_cnt_data, offset=provision_file.maxaddr()+1)
        ih.merge(provision_file, overlap='error')
        ih.write_hex_file(output)

    else:

        assert len(sec_cnt_data) <= max_size, """Secure counters dont't fit.
    Reduce the number of counter types or counter slots and try again."""

        ih = IntelHex()
        ih.frombytes(sec_cnt_data, offset=secure_cnt_addr)
        ih.write_hex_file(output)


if __name__ == '__main__':
    main()
