/**
 * @file gear_types.h
 * @brief gear_types for gear_display_facade in modules
 * @date 08.06.2026
 */

#ifndef GEAR_TYPES_H
#define GEAR_TYPES_H

enum class GearPosition {
    NEUTRAL,
    FORWARD_1,
    FORWARD_2,
    FORWARD_3,
    REVERSE_1,
    REVERSE_2
};

enum class DriveMode {
    ECO,
    POWER
};

#endif
