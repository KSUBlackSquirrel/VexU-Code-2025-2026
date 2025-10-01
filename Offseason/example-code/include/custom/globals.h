#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

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
const static pros::controller_id_e_t mainControllerID = pros::E_CONTROLLER_MASTER;
const static pros::controller_analog_e_t leftStickY = pros::E_CONTROLLER_ANALOG_LEFT_Y;
const static pros::controller_analog_e_t rightStickY = pros::E_CONTROLLER_ANALOG_RIGHT_Y;

const static int joystickDeadband = 1;
const static double expoCurve = 1.0;

const static std::vector<std::int8_t> leftMotorsID = {-10,-9,-8};
const static std::vector<std::int8_t> rightMotorsID = {13,11,12};
const static pros::v5::MotorGears driveTrainColor = pros::MotorGearset::green;
const static float wheelDiameter = lemlib::Omniwheel::NEW_325;
const static int wheelTrack = 13.625;
const static int wheelRPM = 400;
const static int horizontalDrift = 2;

const static int imuID = 5;
} // namespace drive vars

namespace example {
const static std::int8_t motorId = 20;
const static pros::MotorGears motorColor = pros::MotorGearset::red;
} // namespace vars used for examples

} // all global consts



namespace globalVar {

namespace pulse {

} // namespace pulse

} // all global vars


#endif
