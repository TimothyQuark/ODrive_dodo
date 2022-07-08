#!/usr/bin/env python3

import argparse
import importlib

import odrive
from fibre.libfibre import ObjectLostError

# Script which parses a motor configuration (ex. config_antigravity4004),
# and configures the ODrive.

def get_args():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument('--config', help='motor config module', default='config_antigravity4004')
    return parser.parse_args()

def main():
    args = get_args()
    config = importlib.import_module(args.config)
    mgr = ODriveManager()
    save_config(mgr, config, args.config)
    print('done')

def save_config(mgr, config, config_str: str):
    print('(1/3) erasing old config')
    mgr.erase_config()
    print('(2/3) setting new config ({})'.format(config_str))
    mgr.set_config(config)
    print('(3/3) saving new config')
    mgr.save_config()

class ODriveManager:
    def __init__(self):
        self.odrv = None

    def _find_if_needed(self):
        if self.odrv is not None:
            return
        print('searching odrive ... ', end='')
        if self.odrv:
            self.odrv = odrive.find_any()
        print('found')

    def erase_config(self):
        self._find_if_needed()
        try:
            self.odrv.erase_configuration()  # causes reboot
        except ObjectLostError:
            print('reboot (erase config)')
            self.odrv = None

    def set_config(self, config):
        self._find_if_needed()
        config.set(self.odrv)

    def save_config(self):
        self._find_if_needed()
        try:
            self.odrv.save_configuration()  # causes reboot
        except ObjectLostError:
            print('reboot (saveconfig')
            self.odrv = None

if __name__ == '__main__':
    main()
