/**
 * @file globals.h
 * @brief Global constants, utilities, and configuration
 * 
 * This file contains global constants and utility functions used throughout
 * the robot code. Organize constants by subsystem or function in namespaces.
 * 
 * Add your robot-specific constants here:
 * - Motor ports
 * - Sensor ports  
 * - Physical dimensions (wheel diameter, gear ratios, etc.)
 * - Control parameters (PID constants, speed limits, etc.)
 */
#pragma once

#include "pros/motors.hpp"
#include "lemlib/api.hpp"


namespace global {

/**
 * @brief Utility functions for motor control
 */
namespace MotorTools {
/**
 * @brief Convert percentage to motor velocity in RPM
 * @param percent Desired speed as percentage (-100 to 100)
 * @param color Motor cartridge type (red=100RPM, green=200RPM, blue=600RPM)
 * @return Motor velocity in RPM
 * Example: percentToVelocity(50, green) returns 100 RPM
 */
inline int percentToVelocity(int percent, pros::MotorGears color) {
    int maxRpm = 0;
    switch (color) {
        case pros::MotorGearset::red:   maxRpm = 100; break;
        case pros::MotorGearset::green: maxRpm = 200; break;
        case pros::MotorGearset::blue:  maxRpm = 600; break;
        default: maxRpm = 0; break;
    }
    return static_cast<int>(percent * maxRpm / 100.0);
}
} // namespace MotorTools

/**
 * @brief Drive subsystem configuration
 */
namespace drive {
/**
 * @brief Main controller ID for driver input
 */
const static pros::controller_id_e_t kMainControllerID = pros::E_CONTROLLER_MASTER;
} // namespace drive

/**
 * @brief Example namespace for organizing subsystem constants
 * 
 * Copy this pattern when adding constants for your subsystems.
 * Organize related constants together in namespaces.
 */
namespace example {
    // const auto example = NULL;
} // namespace example


} // all global vars

