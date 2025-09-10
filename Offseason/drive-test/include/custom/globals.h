#ifndef _GLOBALS_H_
#define _GLOBALS_H_

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

namespace GlobalControl {
const static pros::controller_analog_e_t leftStickY = pros::E_CONTROLLER_ANALOG_LEFT_Y;
const static pros::controller_analog_e_t rightStickY = pros::E_CONTROLLER_ANALOG_RIGHT_Y;
const static int joystickDeadband = 1;
const static double expoCurve = 1.0;
} // namespace control 


namespace GlobalMotor {
const static std::int8_t motorId = 1;
const static pros::MotorGears motorColor = pros::MotorGearset::red;
} // namespace motor 

#endif
