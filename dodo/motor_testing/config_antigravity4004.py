from odrive.enums import MOTOR_TYPE_HIGH_CURRENT

# Automatic configuration of the Antigravity 4004 motor for ODrive. Do not
# run this script directly: instead use save_config.py and pass this filename
# (without .py suffix). This should also be the default motor config for
# save_config.py, so you may not even need to provide an argument at all.

# config from
# https://discourse.odriverobotics.com/t/antigravity-4004-calibration-gives-motor-error-phase-resistance-out-of-range/9058

# please sort config ascending


def set(odrv0):

    # General Board Configurations
    odrv0.config.brake_resistance = 2
    odrv0.config.enable_brake_resistor = True

    # Axis 0

    # workaround by Fajar
    odrv0.axis0.controller.config.enable_overspeed_error = False
    odrv0.axis0.controller.config.pos_gain = 100
    odrv0.axis0.controller.config.vel_gain = 0.02
    odrv0.axis0.controller.config.vel_integrator_gain = 0.02
    odrv0.axis0.controller.config.vel_limit = 10

    odrv0.axis0.encoder.config.cpr = 20000

    odrv0.axis0.motor.config.current_lim = 10
    odrv0.axis0.motor.config.motor_type = MOTOR_TYPE_HIGH_CURRENT
    odrv0.axis0.motor.config.pole_pairs = 12
    # resistance_calib_max_voltage should be set to 4 usually. However, the
    # motor heats up when idle (it should not, but it does), and this increases
    # the internal resistance of the motor. This value sets the max V that can be applied,
    # to motor during calibration, and 4 is too low when the motor is hot.
    # You can check this by checking current draw of power supply during idle,
    # if it is higher than 42mA then the motor is probably heating up
    odrv0.axis0.motor.config.resistance_calib_max_voltage = 6
    odrv0.axis0.motor.config.torque_constant = 8.27 / 300

    # Axis 1 (Board controls 2 motors, dodo legs have 2 motors)

    # workaround by Fajar
    odrv0.axis1.controller.config.enable_overspeed_error = False
    odrv0.axis1.controller.config.pos_gain = 100
    odrv0.axis1.controller.config.vel_gain = 0.02
    odrv0.axis1.controller.config.vel_integrator_gain = 0.02
    odrv0.axis1.controller.config.vel_limit = 101

    odrv0.axis1.encoder.config.cpr = 20000

    odrv0.axis1.motor.config.current_lim = 10
    odrv0.axis1.motor.config.motor_type = MOTOR_TYPE_HIGH_CURRENT
    odrv0.axis1.motor.config.pole_pairs = 12
    odrv0.axis1.motor.config.resistance_calib_max_voltage = 6
    odrv0.axis1.motor.config.torque_constant = 8.27 / 300
