#!/usr/bin/env python3

import odrive
from odrive.enums import AXIS_STATE_FULL_CALIBRATION_SEQUENCE


def main():
    print('searching odrive ... ', end='')
    odrv0 = odrive.find_any()
    print('found')
    odrv0.axis0.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE

if __name__ == '__main__':
    main()
