/**
 * @file gear_display_facade.cc
 * @brief GearDisplayFacade for gear_display_facade in modules
 * @date 08.06.2026
 */

#include "gear_display_facade.h"

// Option 1:
// using gear_display_facade::GearDisplayFacade;
//
// This only brings the class name GearDisplayFacade into the current scope.
// Other names inside the namespace, such as AcGearPositionSignal,
// AcIsEcoModeSignal, GearPositionSignal and DriveModeSignal,
// are still not directly available.

// Option 2:
namespace gear_display_facade {
// 
// Define the member functions inside the gear_display_facade namespace.
// This makes all names declared in this namespace directly available here.
// Therefore, we can write GearDisplayFacade, AcGearPositionSignal,
// GearPositionSignal, etc. without adding gear_display_facade:: each time.

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
            return;
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




