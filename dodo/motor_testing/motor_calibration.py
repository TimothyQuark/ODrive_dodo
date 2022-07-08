#!/usr/bin/env python3

import odrive
from odrive.enums import AXIS_STATE_FULL_CALIBRATION_SEQUENCE

# Script which calibrates the motor. Run this script every time you turn the
# ODrive on.

def main():
    print('searching odrive ... ', end='')
    odrv0 = odrive.find_any()
    if odrv0:
        print('found')
    odrv0.axis0.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE

if __name__ == '__main__':
    main()
