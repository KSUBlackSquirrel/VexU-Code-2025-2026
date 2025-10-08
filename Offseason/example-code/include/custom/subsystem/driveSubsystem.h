#ifndef DRIVESUBSYSTEM_H_
#define DRIVESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "custom/controller.h"
#include "lemlib/api.hpp"


class DriveSubsystem : public SubsystemBase {
    public:
        DriveSubsystem()
            : m_throttle_curve(globalConst::drive::joystickDeadband, globalConst::drive::joystickDeadband, globalConst::drive::expoCurve),
            m_steer_curve(globalConst::drive::joystickDeadband, globalConst::drive::joystickDeadband, globalConst::drive::expoCurve),
            m_left_motor_group(globalConst::drive::kLeftMotorsID, globalConst::drive::kDriveTrainColor),
            m_right_motor_group(globalConst::drive::kRightMotorsID, globalConst::drive::kDriveTrainColor),
            m_drivetrain(&m_left_motor_group, &m_right_motor_group, globalConst::drive::kWheelTrack, globalConst::drive::kWheelDiameter, globalConst::drive::kWheelRPM, globalConst::drive::kHorizontalDrift),
            m_sensors(nullptr, nullptr, nullptr, nullptr, nullptr),
            m_lateral_controller(18, 0, 6, 0, 1, 100, 3, 500, 0),
            m_angular_controller(4, 0, 29, 0, 1, 100, 3, 500, 0),
            m_chassis(m_drivetrain, m_lateral_controller, m_angular_controller, m_sensors, &m_throttle_curve, &m_steer_curve)
        {
            m_chassis.calibrate(); // calibrate sensors
        }

        inline lemlib::Pose pos(){
            return m_chassis.getPose();
        }

        inline void tankDrive(Controller* controller, bool inverted){
            m_chassis.tank(
                inverted ? -controller->get_analog(globalConst::drive::kRightStickY) : controller->get_analog(globalConst::drive::kLeftStickY),
                inverted ?  -controller->get_analog(globalConst::drive::kLeftStickY) : controller->get_analog(globalConst::drive::kRightStickY));
        }

    private:
        // input curve for throttle and steer inputs during driver control
        lemlib::ExpoDriveCurve m_throttle_curve;
        lemlib::ExpoDriveCurve m_steer_curve;

        pros::MotorGroup m_left_motor_group;
        pros::MotorGroup m_right_motor_group;
        lemlib::Drivetrain m_drivetrain;

        // odometry settings
        lemlib::OdomSensors m_sensors;

        // lateral PID controller
        lemlib::ControllerSettings m_lateral_controller;
        // angular PID controller
        lemlib::ControllerSettings m_angular_controller;

        // create the chassis
        lemlib::Chassis m_chassis;
};

#endif // DRIVESUBSYSTEM_H_