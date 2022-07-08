#!/usr/bin/env python3

import odrive
from odrive.enums import AXIS_STATE_IDLE

# Script which puts the ODrive into idle mode (both axes)

def main():
    print('searching odrive ... ', end='')    
    odrv0 = odrive.find_any()
    if odrv0:
        print('found')

    odrv0.axis0.requested_state = AXIS_STATE_IDLE
    odrv0.axis1.requested_state = AXIS_STATE_IDLE

if __name__ == '__main__':
    main()
