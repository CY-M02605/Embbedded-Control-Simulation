/**
 * @file main.cc
 * @brief Entry point for vehicle monitoring system simulation
 * @date 29.05.2026
 */

#include <iostream>
#include "signals/signals.h"
#include "framework/manager.h"
#include "modules/hydraulic_temp_warning_module.h"

const char* SelectStr(Signals::OnOffState s_) {
    switch (s_) {
    case Signals::OnOffState::ON:
        return "ON";
    case Signals::OnOffState::OFF:
        return "OFF";
    default:
        return "UNKNOWN";
    }
}

int main() {
    // Create configuration with hardcoded values
    modules::HydraulicTempWarningFunction::Config config;
    config.temp_hysteresis_config.high_threshold = 95.0f;
    config.temp_hysteresis_config.low_threshold = 85.0f;
    config.warning_time_config.threshold = 3.0f;
    config.warning_time_config.step = 1.0f;
    config.critical_threshold = 100.0;

    // Create input signals
    Signals::TemperatureSignal oil_temp(80.0f, Signals::SignalValidity::VALID);
    Signals::BoolSignal is_diag_normal(true, Signals::SignalValidity::VALID);

    // Create manager
    framework::Manager manager;

    // Create warning modules (automatically registered)
    modules::HydraulicTempWarningFunction WarningModules(
        config,
        oil_temp,
        is_diag_normal,
        manager);

    // Create mimic temperature data array
    struct InputSignals {
        float temp;
        bool diag_info;
        Signals::OnOffState expected_lamp_normal;
        Signals::OnOffState expected_lamp_critical;
    };

    InputSignals InputSignalsArray [] = {
        {81.0f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {82.7f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {83.4f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {84.2f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {85.1f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {85.9f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {86.8f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {87.6f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {88.8f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {90.4f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {91.5f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {92.8f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {93.7f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {94.4f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {95.3f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {96.2f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {97.3f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {98.6f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {99.3f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {100.1f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {100.2f, true, Signals::OnOffState::OFF, Signals::OnOffState::ON},{100.0f, true, Signals::OnOffState::OFF, Signals::OnOffState::ON}, 
        {99.7f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {99.9f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {100.1f, false, Signals::OnOffState::OFF, Signals::OnOffState::ON}, {99.5f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {99.1f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {100.2f, false, Signals::OnOffState::OFF, Signals::OnOffState::ON}, 
        {99.8f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {99.4f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {98.2f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {97.2f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {95.9f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {94.2f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {93.6f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {93.1f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF},
        {92.8f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {91.7f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {91.0f, false, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {90.5f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {89.7f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {88.4f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {87.8f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {86.9f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {86.4f, false, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {86.1f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {85.7f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, {85.1f, true, Signals::OnOffState::ON, Signals::OnOffState::OFF}, 
        {84.2f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {83.6f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {83.3f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {82.7f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {82.1f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {81.4f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, 
        {80.8f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}, {80.1f, true, Signals::OnOffState::OFF, Signals::OnOffState::OFF}  
    };

    int length_array = sizeof(InputSignalsArray) / sizeof(InputSignalsArray[0]);

    for (int i = 0; i < length_array; ++i) {
        oil_temp.SetValue(InputSignalsArray[i].temp);
        is_diag_normal.SetValue(InputSignalsArray[i].diag_info);
        manager.UpdateAll();

        bool pass_normal = (InputSignalsArray[i].expected_lamp_normal == WarningModules.WarningLampNormal().GetValue());
        bool pass_critical = (InputSignalsArray[i].expected_lamp_critical == WarningModules.WarningLampCritical().GetValue());

        std::cout << "Circle " << (i+1) << " : "
                  << "Temp = " << InputSignalsArray[i].temp << " "
                  << "Diag = " << (InputSignalsArray[i].diag_info? "Good" : "NG") << " "
                //   << "Lamp = " << (WarningModules.WarningLamp().GetValue() == Signals::OnOffState::ON? "ON" : "OFF")
                  << "Normal_lamp = " << SelectStr(WarningModules.WarningLampNormal().GetValue()) << " "
                  << "Normal_judge : " << (pass_normal ? "PASS" : "FAIL") << " "
                  << "Critical_lamp = " << SelectStr(WarningModules.WarningLampCritical().GetValue()) << " "
                  << "Critical_judge : " << (pass_critical ? "PASS" : "FAIL") << " "
                  << std::endl;
    }

    return 0;
}
