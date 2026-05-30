/**
 * @file hydraulic_temp_warning_module.cc
 * @brief hydraulic_temp_warning for the modules
 * @date 28.05.2026
 */

 #include "hydraulic_temp_warning_module.h"

 using modules::HydraulicTempWarningFunction;

 HydraulicTempWarningFunction::HydraulicTempWarningFunction(
    const Config& config,
    const Signals::TemperatureSignal& oil_temp,
    const Signals::BoolSignal& is_diag_normal, 
    framework::Manager& manager
 ):config_(config),
 temp_hysteresis_(config.temp_hysteresis_config),
 warning_time_(config.warning_time_config),
 oil_temp_(oil_temp),
 is_diag_normal_(is_diag_normal),
 warning_lamp_normal_output_(Signals::OnOffState::OFF, Signals::SignalValidity::VALID),
 warning_lamp_critical_output_(Signals::OnOffState::OFF, Signals::SignalValidity::VALID),
 status_(STATUS::state_off) {
    manager.RegisterModule(*this);
 }

 void HydraulicTempWarningFunction::Update() {

    // Signal validity check: if any input is invalid, turn off lamp and return early
    if (!oil_temp_.IsValid() || !is_diag_normal_.IsValid()) {
        warning_lamp_normal_output_ = Signals::OnOffSignal(Signals::OnOffState::OFF, Signals::SignalValidity::INVALID);
        warning_lamp_critical_output_ = Signals::OnOffSignal(Signals::OnOffState::OFF, Signals::SignalValidity::INVALID);
        status_ = STATUS::state_off;
        warning_time_.Clear();
        return;
    }

    Signals::OnOffState normal_output_;
    Signals::OnOffState critical_output_;

    temp_hysteresis_.Evaluate(oil_temp_.GetValue());
    warning_time_.Update(temp_hysteresis_.GetState() == Signals::OnOffState::ON);

    if (oil_temp_.GetValue() >= config_.critical_threshold) {
        normal_output_ = Signals::OnOffState::OFF;
        critical_output_ = Signals::OnOffState::ON;
        warning_lamp_normal_output_ = Signals::OnOffSignal(normal_output_, Signals::SignalValidity::VALID);
        warning_lamp_critical_output_ = Signals::OnOffSignal(critical_output_, Signals::SignalValidity::VALID);
        warning_time_.Clear();
        return;
    } else {
        switch (status_)
        {
        case STATUS::state_on:
            if (!is_diag_normal_.GetValue() 
                || temp_hysteresis_.GetState() != Signals::OnOffState::ON ) {
                    normal_output_ = Signals::OnOffState::OFF;
                    critical_output_ = Signals::OnOffState::OFF;
                    status_ = STATUS::state_off;
                } else {
                    normal_output_ = Signals::OnOffState::ON;
                    critical_output_ = Signals::OnOffState::OFF;
                }
            break;
        
        case STATUS::state_off:
            if (is_diag_normal_.GetValue() 
                && temp_hysteresis_.GetState() == Signals::OnOffState::ON 
                && warning_time_.IsTimeUp()) {
                    normal_output_ = Signals::OnOffState::ON;
                    critical_output_ = Signals::OnOffState::OFF;
                    status_ = STATUS::state_on;
                    warning_time_.Clear();
                } else if (temp_hysteresis_.GetState() != Signals::OnOffState::ON) {
                    normal_output_ = Signals::OnOffState::OFF;
                    critical_output_ = Signals::OnOffState::OFF;
                    warning_time_.Clear();
                } else {
                    normal_output_ = Signals::OnOffState::OFF;
                    critical_output_ = Signals::OnOffState::OFF;
                }
            break;
        }

    }

    warning_lamp_normal_output_ = Signals::OnOffSignal(normal_output_, Signals::SignalValidity::VALID);
    warning_lamp_critical_output_ = Signals::OnOffSignal(critical_output_, Signals::SignalValidity::VALID);
 }

 const Signals::OnOffSignal& HydraulicTempWarningFunction::WarningLampNormal() const {
    return warning_lamp_normal_output_;
 }

 const Signals::OnOffSignal& HydraulicTempWarningFunction::WarningLampCritical() const {
    return warning_lamp_critical_output_;
 }
