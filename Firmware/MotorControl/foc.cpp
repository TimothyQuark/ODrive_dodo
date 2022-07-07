
#include "foc.hpp"

#include <board.h>

Motor::Error AlphaBetaFrameController::on_measurement(
    std::optional<float> vbus_voltage,
    std::optional<std::array<float, 3>> currents,
    uint32_t input_timestamp) {
    // std::optional<float2D> Ialpha_beta;

    // if (currents.has_value()) {
    //     // Clarke transform
    //     Ialpha_beta = {
    //         (*currents)[0],
    //         one_by_sqrt3 * ((*currents)[1] - (*currents)[2])};
    // }

    // Dodo Clarke transform.
    // --- Start

    // Where we store the alpha beta currents
    std::optional<float2D> Ialpha_beta;

    if (currents.has_value()) {
        // Only transform if the ODrive actually measured the motor phase currents

        // C = 2/3
        // C constant used for FOC, as described in https://link.springer.com/book/10.1007/978-3-8348-2050-1
        // The actual value of C matters for the inverse Clarke Transform, see utils.hpp
        Ialpha_beta = {
            // Alpha
            currents.value()[0],
            // div_2_by_3 * currents.value()[0] - div_1_by_3 * (currents.value()[1] - currents.value()[2]),
            // Beta
            one_by_sqrt3 * (currents.value()[1] - currents.value()[2])};
    }

    // --- End

    // This function overload can be found lower down in this file.
    return on_measurement(vbus_voltage, Ialpha_beta, input_timestamp);
}

Motor::Error AlphaBetaFrameController::get_output(
    uint32_t output_timestamp, float (&pwm_timings)[3],
    std::optional<float>* ibus) {
    std::optional<float2D> mod_alpha_beta;
    Motor::Error status = get_alpha_beta_output(output_timestamp, &mod_alpha_beta, ibus);

    if (status != Motor::ERROR_NONE) {
        return status;
    } else if (!mod_alpha_beta.has_value() || is_nan(mod_alpha_beta->first) || is_nan(mod_alpha_beta->second)) {
        return Motor::ERROR_MODULATION_IS_NAN;
    }

    auto [tA, tB, tC, success] = SVM(mod_alpha_beta->first, mod_alpha_beta->second);
    if (!success) {
        return Motor::ERROR_MODULATION_MAGNITUDE;
    }

    pwm_timings[0] = tA;
    pwm_timings[1] = tB;
    pwm_timings[2] = tC;

    return Motor::ERROR_NONE;
}

void FieldOrientedController::reset() {
    v_current_control_integral_d_ = 0.0f;
    v_current_control_integral_q_ = 0.0f;
    vbus_voltage_measured_ = std::nullopt;
    Ialpha_beta_measured_ = std::nullopt;
    power_ = 0.0f;
}

Motor::Error FieldOrientedController::on_measurement(
    std::optional<float> vbus_voltage, std::optional<float2D> Ialpha_beta,
    uint32_t input_timestamp) {
    // Store the measurements for later processing.
    i_timestamp_ = input_timestamp;
    vbus_voltage_measured_ = vbus_voltage;
    Ialpha_beta_measured_ = Ialpha_beta;

    return Motor::ERROR_NONE;
}

ODriveIntf::MotorIntf::Error FieldOrientedController::get_alpha_beta_output(
    uint32_t output_timestamp, std::optional<float2D>* mod_alpha_beta,
    std::optional<float>* ibus) {
    if (!vbus_voltage_measured_.has_value() || !Ialpha_beta_measured_.has_value()) {
        // FOC didn't receive a current measurement yet.
        return Motor::ERROR_CONTROLLER_INITIALIZING;
    } else if (abs((int32_t)(i_timestamp_ - ctrl_timestamp_)) > MAX_CONTROL_LOOP_UPDATE_TO_CURRENT_UPDATE_DELTA) {
        // Data from control loop and current measurement are too far apart.
        return Motor::ERROR_BAD_TIMING;
    }

    // TODO: improve efficiency in case PWM updates are requested at a higher
    // rate than current sensor updates. In this case we can reuse mod_d and
    // mod_q from a previous iteration.

    if (!Vdq_setpoint_.has_value()) {
        return Motor::ERROR_UNKNOWN_VOLTAGE_COMMAND;
    } else if (!phase_.has_value() || !phase_vel_.has_value()) {
        return Motor::ERROR_UNKNOWN_PHASE_ESTIMATE;
    } else if (!vbus_voltage_measured_.has_value()) {
        return Motor::ERROR_UNKNOWN_VBUS_VOLTAGE;
    }

    auto [Vd, Vq] = *Vdq_setpoint_;
    float phase = *phase_;
    float phase_vel = *phase_vel_;
    float vbus_voltage = *vbus_voltage_measured_;

    // Idq.has_value() will return false if this has not been filled in by the Park Transform code
    // (i.e. no Ialpha_beta was measured)
    // This is used in other parts of the code for error checking.
    std::optional<float2D> Idq;

    // // Park transform
    // if (Ialpha_beta_measured_.has_value()) {
    //     auto [Ialpha, Ibeta] = *Ialpha_beta_measured_;
    //     float I_phase = phase + phase_vel * ((float)(int32_t)(i_timestamp_ - ctrl_timestamp_) / (float)TIM_1_8_CLOCK_HZ);
    //     float c_I = our_arm_cos_f32(I_phase);
    //     float s_I = our_arm_sin_f32(I_phase);
    //     Idq = {
    //         c_I * Ialpha + s_I * Ibeta,
    //         c_I * Ibeta - s_I * Ialpha};
    //     // Dodo: wtf is this code below. I_measured_report_filter_k_ = 1.0 in header, so this calculation undoes itself
    //     Id_measured_ += I_measured_report_filter_k_ * (Idq->first - Id_measured_);
    //     Iq_measured_ += I_measured_report_filter_k_ * (Idq->second - Iq_measured_);
    // } else {
    //     Id_measured_ = 0.0f;
    //     Iq_measured_ = 0.0f;
    // }

    // Dodo Park Transform
    // --- Start
    if (Ialpha_beta_measured_.has_value()) {
        auto [Ialpha, Ibeta] = Ialpha_beta_measured_.value();
        // Calculate the current phase (not measured, calculated using the hardware clock)
        // float I_phase = phase + phase_vel * static_cast<float>(i_timestamp_ - ctrl_timestamp_) / static_cast<float>(TIM_1_8_CLOCK_HZ);
        float I_phase = phase + phase_vel * ((float)(int32_t)(i_timestamp_ - ctrl_timestamp_) / (float)TIM_1_8_CLOCK_HZ);

        // Calculate cos(phase) and sin(phase). Since this is a small embedded system, it uses a custom made library for this
        float cos_phase = our_arm_cos_f32(I_phase);
        float sin_phase = our_arm_sin_f32(I_phase);

        // Idq itself is used in other parts of the code for error checking, hence we will keep it even if
        // this struct is pretty useless in the Park Transform
        Idq = {
            // Id
            (Ialpha * cos_phase) + (Ibeta * sin_phase),
            // cos_phase * Ialpha + sin_phase * Ibeta,
            // Iq
            (Ibeta * cos_phase) - (Ialpha * sin_phase)
            // cos_phase * Ibeta - sin_phase * Ialpha
            };

        // Copy Idq values to FieldOrientedController class variables.
        Id_measured_ = Idq->first;
        Iq_measured_ = Idq->second;
        // Id_measured_ += I_measured_report_filter_k_ * (Idq->first - Id_measured_);
        // Iq_measured_ += I_measured_report_filter_k_ * (Idq->second - Iq_measured_);

    } else {
        // No Ialpha_beta measured, so use default values
        Id_measured_ = 0.0f;
        Iq_measured_ = 0.0f;
    }
    // --- End

    float mod_to_V = (2.0f / 3.0f) * vbus_voltage;
    float V_to_mod = 1.0f / mod_to_V;
    // FOC (i.e. the transforms we did above) do not actually do any control. Instead, current or voltage
    // control will modify Idq currents, i.e. modulate Idq
    float mod_d;
    float mod_q;

    // DODO TODO: Write our own current control logic
    if (enable_current_control_) {
        // Current control mode

        if (!pi_gains_.has_value()) {
            return Motor::ERROR_UNKNOWN_GAINS;
        } else if (!Idq.has_value()) {
            return Motor::ERROR_UNKNOWN_CURRENT_MEASUREMENT;
        } else if (!Idq_setpoint_.has_value()) {
            return Motor::ERROR_UNKNOWN_CURRENT_COMMAND;
        }

        auto [p_gain, i_gain] = *pi_gains_;
        auto [Id, Iq] = *Idq;
        auto [Id_setpoint, Iq_setpoint] = *Idq_setpoint_;

        float Ierr_d = Id_setpoint - Id;
        float Ierr_q = Iq_setpoint - Iq;

        // Apply PI control (V{d,q}_setpoint act as feed-forward terms in this mode)
        mod_d = V_to_mod * (Vd + v_current_control_integral_d_ + Ierr_d * p_gain);
        mod_q = V_to_mod * (Vq + v_current_control_integral_q_ + Ierr_q * p_gain);

        // Vector modulation saturation, lock integrator if saturated
        // TODO make maximum modulation configurable
        float mod_scalefactor = 0.80f * sqrt3_by_2 * 1.0f / std::sqrt(mod_d * mod_d + mod_q * mod_q);
        if (mod_scalefactor < 1.0f) {
            mod_d *= mod_scalefactor;
            mod_q *= mod_scalefactor;
            // TODO make decayfactor configurable
            v_current_control_integral_d_ *= 0.99f;
            v_current_control_integral_q_ *= 0.99f;
        } else {
            v_current_control_integral_d_ += Ierr_d * (i_gain * current_meas_period);
            v_current_control_integral_q_ += Ierr_q * (i_gain * current_meas_period);
        }

    } else {
        // Voltage control mode
        mod_d = V_to_mod * Vd;
        mod_q = V_to_mod * Vq;
    }

    // // Inverse park transform
    // float pwm_phase = phase + phase_vel * ((float)(int32_t)(output_timestamp - ctrl_timestamp_) / (float)TIM_1_8_CLOCK_HZ);
    // float c_p = our_arm_cos_f32(pwm_phase);
    // float s_p = our_arm_sin_f32(pwm_phase);
    // float mod_alpha = c_p * mod_d - s_p * mod_q;
    // float mod_beta = c_p * mod_q + s_p * mod_d;

    // Dodo Inverse Park Transform
    // --- Start
    // PWM Phase. Note this is not the same as the phase calculated in the Park Transform
    float pwm_phase = phase + phase_vel * static_cast<float>(output_timestamp - ctrl_timestamp_) / static_cast<float>(TIM_1_8_CLOCK_HZ);
    float cos_pwm = our_arm_cos_f32(pwm_phase);
    float sin_pwm = our_arm_sin_f32(pwm_phase);

    // We do not use the raw transformed values of Idq. Instead we use the modulated values, which have been modified
    // either by current or voltage control (see above)
    float mod_alpha = cos_pwm * mod_d - sin_pwm * mod_q;
    float mod_beta = sin_pwm * mod_d + cos_pwm * mod_q;
    // --- End

    // Report final applied voltage in stationary frame (for sensorless estimator)
    final_v_alpha_ = mod_to_V * mod_alpha;
    final_v_beta_ = mod_to_V * mod_beta;

    *mod_alpha_beta = {mod_alpha, mod_beta};

    if (Idq.has_value()) {
        auto [Id, Iq] = *Idq;
        *ibus = mod_d * Id + mod_q * Iq;
        power_ = vbus_voltage * (*ibus).value();
    }

    return Motor::ERROR_NONE;
}

void FieldOrientedController::update(uint32_t timestamp) {
    // DODO TODO: Update our important variables that we want to send to user.
    CRITICAL_SECTION() {
        ctrl_timestamp_ = timestamp;
        enable_current_control_ = enable_current_control_src_;
        Idq_setpoint_ = Idq_setpoint_src_.present();
        Vdq_setpoint_ = Vdq_setpoint_src_.present();
        phase_ = phase_src_.present();
        phase_vel_ = phase_vel_src_.present();
    }
}
