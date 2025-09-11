#ifndef _GLOBALS_H_
#define _GLOBALS_H_

namespace gobalDrive {
const static pros::controller_analog_e_t leftStickY = pros::E_CONTROLLER_ANALOG_LEFT_Y;
const static pros::controller_analog_e_t rightStickY = pros::E_CONTROLLER_ANALOG_RIGHT_Y;

const static std::vector<std::int8_t> leftMotorsID = {-10,-9,-8};
const static std::vector<std::int8_t> rightMotorsID = {13,11,12};
const static pros::v5::MotorGears driveTrainColor = pros::MotorGearset::green;
const static float wheelDiameter = lemlib::Omniwheel::NEW_325;
const static int wheelTrack = 13.625;
const static int wheelRPM = 400;
const static int horizontalDrift = 2;

const static int imuID = 5;
}

#endif
