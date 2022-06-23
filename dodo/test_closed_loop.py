#!/usr/bin/env python3

import argparse

import odrive
from odrive.enums import AXIS_STATE_CLOSED_LOOP_CONTROL


def get_args():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument('--turns', help='number of turns', type=int, default=10)
    return parser.parse_args()

def main():
    args = get_args()
    print('searching odrive ... ', end='')
    odrv0 = odrive.find_any()
    print('found')
    odrv0.axis0.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL
    odrv0.axis0.controller.input_pos = args.turns

if __name__ == '__main__':
    main()
