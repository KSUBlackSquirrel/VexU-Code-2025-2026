#ifndef DRIVERSUBSYSTEM_H_
#define DRIVESUBSYSTEM_H_

#include "main.h"

class DriverSubsystem : public SubsystemBase {
public:
private:

// input curve for throttle and steer inputs during driver control
lemlib::ExpoDriveCurve throttle_curve(
    globalControl::joystickDeadband, // joystick deadband out of 127
    globalControl::joystickDeadband, // minimum output where drivetrain will move out of 127  
    globalControl::expoCurve // expo curve gain
);
lemlib::ExpoDriveCurve steer_curve(
    globalControl::joystickDeadband, // joystick deadband out of 127
    globalControl::joystickDeadband, // minimum output where drivetrain will move out of 127
    globalControl::expoCurve // expo curve gain
);

pros::MotorGroup left_motor_group(globalDrive::leftMotorsID, globalDrive::driveTrainColor);    // Creates a motor group with forwards ports 1 & 3 and reversed port 2
pros::MotorGroup right_motor_group(globalDrive::rightMotorsID, globalDrive::driveTrainColor);  // Creates a motor group with forwards port 5 and reversed ports 4 & 6
lemlib::Drivetrain drivetrain(
    &left_motor_group, // left motor group
    &right_motor_group, // right motor group
    globalDrive::wheelTrack, // track width
    globalDrive::wheelDiameter, // wheel diameter
    globalDrive::wheelRPM, // wheel rpm
    globalDrive::horizontalDrift // horizontal drift is 2 (for now)
);

// imu

// horizontal tracking wheel encoder
// pros::Rotation horizontal_encoder(20);
// // vertical tracking wheel encoder
// pros::adi::Encoder vertical_encoder('C', 'D', true);
// // horizontal tracking wheel
// lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_encoder, lemlib::Omniwheel::NEW_275, -5.75);
// // vertical tracking wheel
// lemlib::TrackingWheel vertical_tracking_wheel(&vertical_encoder, lemlib::Omniwheel::NEW_275, -2.5);

// odometry settings
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            nullptr, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            nullptr // inertial sensor
);


// lateral PID controller
lemlib::ControllerSettings lateral_controller(
    18, // proportional gain (kP)
    0, // integral gain (kI)
    6, // derivative gain (kD)
    0, // anti windup
    1, // small error range, in inches
    100, // small error range timeout, in milliseconds
    3, // large error range, in inches
    500, // large error range timeout, in milliseconds
    0 // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(
    4, // proportional gain (kP)
    0, // integral gain (kI)
    29, // derivative gain (kD)
    0, // anti windup
    1, // small error range, in degrees
    100, // small error range timeout, in milliseconds
    3, // large error range, in degrees
    500, // large error range timeout, in milliseconds
    0 // maximum acceleration (slew)
);

// create the chassis
lemlib::Chassis chassis(
    drivetrain, // drivetrain settings
    lateral_controller, // lateral PID settings
    angular_controller, // angular PID settings
    sensors, // odometry sensors
    &throttle_curve, 
    &steer_curve
);
}

#endif