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
        description='Return the last address of a hex file',
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--hex-file', required=True,
                        help='The path to the intel hex')
    parser.add_argument('--output-file', required=True,
                        help='The path to output configuration file')
    return parser.parse_args()

def main():
    args = parse_args()

    provision_file = IntelHex()
    provision_file.loadhex(args.hex_file)
    with open(args.output_file, 'w') as out_file:
        out_file.write("CONFIG_MCUBOOT_SECURE_COUNTER_ADDRESS=" + hex(provision_file.maxaddr()+1))

if __name__ == '__main__':
    main()
