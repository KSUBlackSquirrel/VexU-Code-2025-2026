#ifndef GLOBALS_H_
#define GLOBALS_H_

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

const static std::vector<std::int8_t> kLeftMotorsID = {-10,-9,-8};
const static std::vector<std::int8_t> kRightMotorsID = {13,11,12};
const static pros::v5::MotorGears kDriveTrainColor = pros::MotorGearset::green;
const static float kWheelDiameter = lemlib::Omniwheel::NEW_325;
const static int kWheelTrack = 13.625;
const static int kWheelRPM = 400;
const static int kHorizontalDrift = 2;

const static int kIMUid = 5;
} // namespace drive vars

namespace example {
const static std::int8_t kMotor1Id = 18;
const static std::int8_t kMotor2Id = 19;
const static std::int8_t kMotor3Id = 20;
const static pros::MotorGears kMotorColor = pros::MotorGearset::blue;
} // namespace vars used for examples

} // all global consts



namespace globalVar {

namespace pulse {

} // namespace pulse

} // all global vars


#endif // GLOBALS_H_
