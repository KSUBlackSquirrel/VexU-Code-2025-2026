#ifndef DRIVESUBSYSTEM_H_
#define DRIVESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "custom/controller.h"
#include "lemlib/api.hpp"


class DriveSubsystem : public SubsystemBase {
    public:
        DriveSubsystem()
            : throttle_curve(globalConst::drive::joystickDeadband, globalConst::drive::joystickDeadband, globalConst::drive::expoCurve),
            steer_curve(globalConst::drive::joystickDeadband, globalConst::drive::joystickDeadband, globalConst::drive::expoCurve),
            left_motor_group(globalConst::drive::leftMotorsID, globalConst::drive::driveTrainColor),
            right_motor_group(globalConst::drive::rightMotorsID, globalConst::drive::driveTrainColor),
            drivetrain(&left_motor_group, &right_motor_group, globalConst::drive::wheelTrack, globalConst::drive::wheelDiameter, globalConst::drive::wheelRPM, globalConst::drive::horizontalDrift),
            sensors(nullptr, nullptr, nullptr, nullptr, nullptr),
            lateral_controller(18, 0, 6, 0, 1, 100, 3, 500, 0),
            angular_controller(4, 0, 29, 0, 1, 100, 3, 500, 0),
            chassis(drivetrain, lateral_controller, angular_controller, sensors, &throttle_curve, &steer_curve)
        {
            chassis.calibrate(); // calibrate sensors
        }

        inline lemlib::Pose pos(){
            return chassis.getPose();
        }

        inline void tankDrive(Controller* controller, bool inverted){
            chassis.tank(
                inverted ? -controller->get_analog(globalConst::drive::rightStickY) : controller->get_analog(globalConst::drive::leftStickY),
                inverted ?  -controller->get_analog(globalConst::drive::leftStickY) : controller->get_analog(globalConst::drive::rightStickY));
        }

    private:
        // input curve for throttle and steer inputs during driver control
        lemlib::ExpoDriveCurve throttle_curve;
        lemlib::ExpoDriveCurve steer_curve;

        pros::MotorGroup left_motor_group;
        pros::MotorGroup right_motor_group;
        lemlib::Drivetrain drivetrain;

        // odometry settings
        lemlib::OdomSensors sensors;

        // lateral PID controller
        lemlib::ControllerSettings lateral_controller;
        // angular PID controller
        lemlib::ControllerSettings angular_controller;

        // create the chassis
        lemlib::Chassis chassis;
};

#endif