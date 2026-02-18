#pragma once

#include "pros/motors.hpp"
#include "lemlib/api.hpp"
#include <vector>



namespace globalConst {

namespace MotorTools {
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
} // namespace motor tools 


namespace drive {
const static pros::controller_id_e_t kMainControllerID = pros::E_CONTROLLER_MASTER;
const static pros::controller_analog_e_t kLeftStickY = pros::E_CONTROLLER_ANALOG_LEFT_Y;
const static pros::controller_analog_e_t kRightStickY = pros::E_CONTROLLER_ANALOG_RIGHT_Y;

const static int joystickDeadband = 1;
const static double expoCurve = 1.0;

const static std::vector<std::int8_t> kLeftMotorsID = {-8,9,-10};
const static std::vector<std::int8_t> kRightMotorsID = {5,-6,7};
const static pros::v5::MotorGears kDriveTrainColor = pros::MotorGearset::blue;
const static float kWheelDiameter = lemlib::Omniwheel::NEW_275;
const static float kWheelTrack = 11.0;
const static int kWheelRPM = 600;
const static int kHorizontalDrift = 2;

const static int kIMUid = 14;

// const static pros::motor_brake_mode_e kBreakMode = pros::E_MOTOR_BRAKE_BRAKE;
const static pros::motor_brake_mode_e kBreakMode = pros::E_MOTOR_BRAKE_COAST;
} // namespace drive vars

namespace intake {
    const static std::vector<std::int8_t> kIntakeMotorsID = {-16, 12};
    const static pros::v5::MotorGears kIntakeMotorColor = pros::MotorGearset::blue;

    const static std::vector<std::int8_t> kOuttakeMotorsID = {11};
    const static pros::v5::MotorGears kOuttakeMotorColor = pros::MotorGearset::blue;
}

namespace outtake {
    const static char outtakeGateID = 'A';
    const static char outtakeLiftID = 'B';
}

} // all global consts

