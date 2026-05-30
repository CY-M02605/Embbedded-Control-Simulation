/**
 * @file hydraulic_temp_warning_module.h
 * @brief hydraulic_temp_warning for the modules
 * @28.05.2026
 */

#ifndef HYDRAULIC_TEMP_WARNING_MODULE_H
#define HYDRAULIC_TEMP_WARNING_MODULE_H

#include "framework/module_interface.h"
#include "framework/manager.h"
#include "signals/signals.h"
#include "utility/hysteresis.h"
#include "utility/increment_timer.h"

namespace modules {
class HydraulicTempWarningFunction: public framework::ModuleInterface {
    public:
        struct Config {
            utility::Hysteresis::Config temp_hysteresis_config;
            utility::IncrementTimer::Config warning_time_config;
            float critical_threshold;
        };

        HydraulicTempWarningFunction(
            // In embedded systems, input signals typically passed as const reference.
            // Sensor data is updated continuously and consumed by modules periodically,
            // so passing by const reference ensures efficient access to the last value
            // without unnecessary copies
            const Config& config,
            const Signals::TemperatureSignal& oil_temp,
            const Signals::BoolSignal& is_diag_normal, 
            framework::Manager& manager
        );

        // Added the override keyword to enable the compiler to check the virtual method name
        void Update() override;

        enum class STATUS {
            state_on,
            state_off
        };

        const Signals::OnOffSignal& WarningLampNormal() const;

        const Signals::OnOffSignal& WarningLampCritical() const;

    private:
        Config config_;
        utility::Hysteresis temp_hysteresis_;
        utility::IncrementTimer warning_time_;
        const Signals::TemperatureSignal& oil_temp_;
        const Signals::BoolSignal& is_diag_normal_;
        Signals::OnOffSignal warning_lamp_normal_output_;
        Signals::OnOffSignal warning_lamp_critical_output_;
        STATUS status_;
};
}


#endif
