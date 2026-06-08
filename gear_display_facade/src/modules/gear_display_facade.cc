/**
 * @file gear_display_facade.cc
 * @brief GearDisplayFacade for gear_display_facade in modules
 * @date 08.06.2026
 */

#include "gear_display_facade.h"

// using gear_display_facade::GearDisplayFacade;

// 
namespace gear_display_facade {

    GearDisplayFacade::GearDisplayFacade(
        const AcGearPositionSignal& ac_gear_position_signal,
        const AcIsEcoModeSignal& ac_is_eco_mode_signal,
        framework::Manager& manager
    ):ac_gear_position_signal_(ac_gear_position_signal),
    ac_is_eco_mode_signal_(ac_is_eco_mode_signal),
    gear_position_output_(GearPosition::NEUTRAL, signals::ValidationStatus::VALID),
    drive_mode_output_(DriveMode::POWER, signals::ValidationStatus::VALID)
    {
        manager.RegisterModule(*this);
    }

    void GearDisplayFacade::Update() {
        UpdateGearPosition();
        UpdateDriveMode();
    }


    const GearPositionSignal& GearDisplayFacade::GearPositionOutput() const {
        return gear_position_output_;
    }
    const DriveModeSignal& GearDisplayFacade::DriveModeOutput() const {
        return drive_mode_output_;
    }

    void GearDisplayFacade::UpdateGearPosition() {
        if (!ac_gear_position_signal_.IsValid()) {
            gear_position_output_.Set(
                GearPosition::NEUTRAL, signals::ValidationStatus::INVALID
            );
            return;
        }
        switch (ac_gear_position_signal_.GetValue()) {
            case 0:gear_position_output_.Set(GearPosition::NEUTRAL, signals::ValidationStatus::VALID); break;
            case 1:gear_position_output_.Set(GearPosition::FORWARD_1, signals::ValidationStatus::VALID); break;
            case 2:gear_position_output_.Set(GearPosition::FORWARD_2, signals::ValidationStatus::VALID); break;
            case 3:gear_position_output_.Set(GearPosition::FORWARD_3, signals::ValidationStatus::VALID); break;
            case -1:gear_position_output_.Set(GearPosition::REVERSE_1, signals::ValidationStatus::VALID); break;
            case -2:gear_position_output_.Set(GearPosition::REVERSE_2, signals::ValidationStatus::VALID); break;
            default:gear_position_output_.Set(GearPosition::NEUTRAL, signals::ValidationStatus::VALID); break;
        }
    }

    void GearDisplayFacade::UpdateDriveMode() {
        if (!ac_is_eco_mode_signal_.IsValid()) {
            drive_mode_output_.Set(
                DriveMode::POWER, signals::ValidationStatus::INVALID
            );
        }
        // if (ac_is_eco_mode_signal_.GetValue() == true) {
        //     drive_mode_output_.Set(DriveMode::ECO, signals::ValidationStatus::VALID);
        // } else {
        //     drive_mode_output_.Set(DriveMode::POWER, signals::ValidationStatus::VALID);
        // }

        drive_mode_output_.Set(
            ac_is_eco_mode_signal_.GetValue()? DriveMode::ECO : DriveMode::POWER,
            signals::ValidationStatus::VALID
        );
    }
}




