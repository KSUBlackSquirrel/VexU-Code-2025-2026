#pragma once

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
            m_imu(globalConst::drive::kIMUid),
            m_sensors(
                nullptr, // vertical tracking wheel 1, set to null
                nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                nullptr, // horizontal tracking wheel 1
                nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                &m_imu // inertial sensor
            ),
            m_lateral_controller(
                globalConst::drive::kl_kp,
                globalConst::drive::kl_ki,
                globalConst::drive::kl_kd,
                globalConst::drive::kl_anti_windup,
                globalConst::drive::kl_small_error_range,
                globalConst::drive::kl_small_error_range_timeout,
                globalConst::drive::kl_large_error_range,
                globalConst::drive::kl_large_error_range_timeout,
                globalConst::drive::kl_acceleration),
            m_angular_controller(
                globalConst::drive::ka_kp,
                globalConst::drive::ka_ki,
                globalConst::drive::ka_kd,
                globalConst::drive::ka_anti_windup,
                globalConst::drive::ka_small_error_range,
                globalConst::drive::ka_small_error_range_timeout,
                globalConst::drive::ka_large_error_range,
                globalConst::drive::ka_large_error_range_timeout,
                globalConst::drive::ka_acceleration),
            m_chassis(m_drivetrain, m_lateral_controller, m_angular_controller, m_sensors, &m_throttle_curve, &m_steer_curve)
        {
            m_chassis.calibrate(); // calibrate sensors
            m_chassis.setBrakeMode(globalConst::drive::kBreakMode);
        }

        inline void tankDrive(Controller* controller, bool inverted){
            m_chassis.tank(
                inverted ? -controller->get_analog(globalConst::drive::kRightStickY) : controller->get_analog(globalConst::drive::kLeftStickY),
                inverted ?  -controller->get_analog(globalConst::drive::kLeftStickY) : controller->get_analog(globalConst::drive::kRightStickY));
        }

        inline void AngularPID() {
            // Tune Angular PID
            // set position to x:0, y:0, heading:0
            m_chassis.setPose(0, 0, 0);
            // turn to face heading 90 with a very long timeout
            m_chassis.turnToHeading(90, 1000000);
        }


    private:
        // input curve for throttle and steer inputs during driver control
        lemlib::ExpoDriveCurve m_throttle_curve;
        lemlib::ExpoDriveCurve m_steer_curve;

        pros::MotorGroup m_left_motor_group;
        pros::MotorGroup m_right_motor_group;
        lemlib::Drivetrain m_drivetrain;

        pros::Imu m_imu;

        // odometry settings
        lemlib::OdomSensors m_sensors;

        // lateral PID controller
        lemlib::ControllerSettings m_lateral_controller;
        // angular PID controller
        lemlib::ControllerSettings m_angular_controller;

        // create the chassis
        lemlib::Chassis m_chassis;
};

