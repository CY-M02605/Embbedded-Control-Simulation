/**
 * @file gear_display_facade.h
 * @brief GearDisplayFacade for gear_display_facade in modules
 * @date 08.06.2026
 */

#ifndef GEAR_DISPLAY_FACADE_H
#define GEAR_DISPLAY_FACADE_H

#include "manager.h"
#include "module_interface.h"
#include "signal.h"
#include "gear_types.h"

namespace gear_display_facade {

typedef signals::Signal<int> AcGearPositionSignal;
typedef signals::Signal<bool> AcIsEcoModeSignal;

typedef signals::Signal<GearPosition> GearPositionSignal;
typedef signals::Signal<DriveMode> DriveModeSignal;

class GearDisplayFacade: public framework::ModuleInterface {
    public:
        GearDisplayFacade(
            const AcGearPositionSignal& ac_gear_position_signal,
            const AcIsEcoModeSignal& ac_is_eco_mode_signal,
            framework::Manager& manager
        );

        void Update() override;

        const GearPositionSignal& GearPositionOutput() const;
        const DriveModeSignal& DriveModeOutput() const;

    private:
        void UpdateGearPosition();
        void UpdateDriveMode();

        const AcGearPositionSignal& ac_gear_position_signal_;
        const AcIsEcoModeSignal& ac_is_eco_mode_signal_;

        GearPositionSignal gear_position_output_;
        DriveModeSignal drive_mode_output_;
        
};
}

#endif
