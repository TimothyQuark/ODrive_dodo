#!/usr/bin/env python3

import argparse

import odrive
from odrive.enums import AXIS_STATE_CLOSED_LOOP_CONTROL

# Short script to run the closed loop test. Takes as argument number of motor turns

def get_args():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument('--turns', help='number of turns', type=int, default=10)
    return parser.parse_args()

def main():
    args = get_args()
    run(args.turns)

def run(num_turns):
    print('searching odrive ... ', end='')
    odrv0 = odrive.find_any()
    if odrv0:
        print('found')
    odrv0.axis0.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL
    odrv0.axis0.controller.input_pos = num_turns

if __name__ == '__main__':
    main()
