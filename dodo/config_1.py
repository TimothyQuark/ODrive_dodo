from odrive.enums import MOTOR_TYPE_HIGH_CURRENT

# config from
# https://discourse.odriverobotics.com/t/antigravity-4004-calibration-gives-motor-error-phase-resistance-out-of-range/9058

# please sort config ascending
def set(odrv0):
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
    odrv0.axis0.motor.config.resistance_calib_max_voltage = 4
    odrv0.axis0.motor.config.torque_constant = 8.27 / 300

    odrv0.config.brake_resistance = 2

    odrv0.config.enable_brake_resistor = True
