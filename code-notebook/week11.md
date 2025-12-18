11/04/2025
Today's Goals:
	Ian, Antonio, and Philip will be transitioning from framework development to application development by implementing the drive subsystem for actual robot control.

Today's Tasks:
	We are creating the drive subsystem that will control an actual drivetrain. This marks a transition from building the framework to using it for practical robotics.
	
	DriveSubsystem Architecture:
	
```cpp
// driveSubsystem.h - Controls robot drivetrain
#ifndef DRIVESUBSYSTEM_H_
#define DRIVESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "custom/controller.h"
#include "lemlib/api.hpp"  // LemLib for advanced motion control

class DriveSubsystem : public SubsystemBase {
private:
    // Input curves for smoother control
    lemlib::ExpoDriveCurve m_throttle_curve;
    lemlib::ExpoDriveCurve m_steer_curve;
    
    // Motor groups
    pros::MotorGroup m_left_motor_group;
    pros::MotorGroup m_right_motor_group;
    
    // LemLib drivetrain
    lemlib::Drivetrain m_drivetrain;
    
    // Sensors (IMU, tracking wheels, etc.)
    lemlib::OdomSensors m_sensors;
    
    // PID controllers for autonomous
    lemlib::ControllerSettings m_lateral_controller;   // Forward/backward
    lemlib::ControllerSettings m_angular_controller;   // Turning
    
    // Complete chassis controller
    lemlib::Chassis m_chassis;
    
public:
    DriveSubsystem();
    
    // Driver control methods
    void tankDrive(Controller* controller, bool inverted);
    void stop();
    
    // Sensor access
    lemlib::Pose getPose();  // Get robot position
    
    // Subsystem periodic
    void periodic() override;
};

#endif
```
	
	What is LemLib?
	LemLib is an external library that provides advanced motion control for VEX robots:
	
	- Odometry: Tracks where the robot is on the field using wheel encoders and IMU
	- Motion Profiling: Smoothly accelerates and decelerates (no wheel slippage)
	- PID Control: Accurately reaches target speeds and positions
	- Path Following: Drives along predetermined paths for autonomous
	
	Instead of us writing all that math, we can use LemLib's working implementations.
	
	Motor Groups:
	
	A `MotorGroup` treats multiple motors as one unit. If your left drive has 3 motors, you put them in a group. Then `leftMotors.move(x)` spins all three together at the same speed and direction automatically.
	
```cpp
// In globals.h - Configuration
namespace drive {
    constexpr std::initializer_list<int8_t> kLeftMotorsID = {1, -2, 3};      // Ports 1,2,3
    constexpr std::initializer_list<int8_t> kRightMotorsID = {-4, 5, -6};  // Ports 4,5,6
    constexpr pros::MotorGearset kDriveTrainColor = pros::MotorGearset::blue; // 600 RPM cartridge
}

// In driveSubsystem.h - Constructor
DriveSubsystem::DriveSubsystem()
    : m_left_motor_group(globalConst::drive::kLeftMotorsID, globalConst::drive::kDriveTrainColor),
      m_right_motor_group(globalConst::drive::kRightMotorsID, globalConst::drive::kDriveTrainColor) {
    // Motors are now ready to use
}
```
	
	Motor Direction Configuration:
	
	Notice the negative signs on certain motor ports. This is necessary because of how the drivetrain is physically built:
	
	- Left side middle motor (port 2): Reversed due to gear orientation
	- Left side: The entire left side is mirrored opposite of the right side
	
	When you send a positive value to the motor group, all motors spin in a way that moves the robot forward, even though some are physically spinning clockwise and others counterclockwise due to the gear arrangements.
	
	Tank Drive Implementation:
	
	Tank drive: left stick controls left side, right stick controls right side.
	
```cpp
inline void DriveSubsystem::tankDrive(Controller* controller, bool inverted) {
    // When inverted, swap and negate the sides for driving backwards
    m_chassis.tank(
        inverted ? -controller->get_analog(globalConst::drive::kRightStickY) 
                 : controller->get_analog(globalConst::drive::kLeftStickY),
        inverted ? -controller->get_analog(globalConst::drive::kLeftStickY) 
                 : controller->get_analog(globalConst::drive::kRightStickY)
    );
}
```
	
	The inverted mode swaps the left and right sticks and negates them, allowing the driver to drive the robot backwards while maintaining intuitive controls of forward driving.
	
	Expo Curves:
	
	Expo curves make the robot easier to drive by making small joystick movements more precise:
	
```cpp
// Linear: y = x
// Small stick movement = small robot movement
// Problem: Hard to make fine adjustments

// Exponential: y = x^1.5
// Small stick movement = VERY small robot movement (precise!)
// Large stick movement = large robot movement (full speed when needed)

lemlib::ExpoDriveCurve curve(
    5,    // Deadband: ignore movements smaller than this
    5,    // Deadband
    1.5   // Exponent: 1.5 gives good balance of precision and power
);
```
	
	With expo curves, drivers can make fine adjustments at low speeds while still accessing full power when they push the stick all the way.
	
	DriveCommand Implementation:
	
	To actually use the drive subsystem, we created a simple command that runs continuously during driver control:
	
```cpp
// driveCommand.h - Driver control for drivetrain
#ifndef DRIVECOMMAND_H_
#define DRIVECOMMAND_H_

#include "custom/command/commandBase.h"
#include "custom/subsystem/driveSubsystem.h"
#include "custom/controller.h"

class DriveCommand : public CommandBase {
private:
    DriveSubsystem* m_subsystem;
    Controller* m_controller;
    bool m_inverted;
    
public:
    DriveCommand(DriveSubsystem* subsystem, Controller* controller, bool inverted = false)
        : m_subsystem(subsystem), m_controller(controller), m_inverted(inverted) {
        addRequirements(subsystem);  // Need exclusive access to drive
    }
    
    void initialize() override {
        // Nothing to do - just start driving
    }
    
    void execute() override {
        // Read joysticks and control drive every loop (~50 times/second)
        m_subsystem->tankDrive(m_controller, m_inverted);
    }
    
    void end(bool interrupted) override {
        // Stop motors when command ends (safety)
        m_subsystem->stop();
    }
    
    bool isFinished() override {
        // Never finishes - runs as default command
        return false;
    }
    
    CommandBase* clone() const override {
        return new DriveCommand(*this);
    }
};

#endif
```
	
	The command is incredibly simple because all the complexity is hidden in the subsystem. It just reads joysticks and passes values to the subsystem every loop.
	
	Setting as Default Command:
	
```cpp
// In robot.cpp
std::unique_ptr<DriveSubsystem> driveSub = std::make_unique<DriveSubsystem>();
std::unique_ptr<DriveCommand> driveCmd = std::make_unique<DriveCommand>(
    driveSub.get(), 
    &controller, 
    false  // not inverted
);

void robotInit() {
    // Set drive command as default - it runs whenever no other command uses drive subsystem
    driveSub->setDefaultCommand(driveCmd.get());
}
```

Reflection:
	We successfully created the complete drive system architecture! The DriveSubsystem handles all the motor control and LemLib integration, while the DriveCommand simply reads joysticks and passes values to the subsystem. The code compiles cleanly and the structure is solid. The clean separation between DriveCommand and DriveSubsystem makes the code easy to understand and maintain. Tomorrow we'll test it on the driveable frame to validate that everything works correctly.

**[PHOTO NEEDED: Diagram showing DriveSubsystem → LemLib → Motor Groups architecture with arrows showing data flow]**


11/05/2025
Today's Goals:
	Ian, Antonio, and Philip will be testing the drive system on hardware for the first time.

Today's Tasks:
	After implementing the drive system architecture yesterday, today we tested it on our driveable frame.
	
	Hardware Testing:
	We loaded the code onto the robot and conducted our first tests:
	
	Test 1: Basic Tank Drive
	- Left stick forward → Left side moves forward
	- Right stick forward → Right side moves forward
	- Both sticks forward → Robot drives straight forward

	
	Test 2: Expo Curve
	- Small joystick movements → Robot moves slower then linear
	- Large joystick movements → Robot moves at max speed at max stick value
	
	Test 3: Subsystem Conflict
	- Driving with joysticks (default command running)
	- Press button to run test command (also needs drive subsystem)
	- Expected: Drive command interrupted, test command takes over
	
	Test 4: Default Command Resume
	- Run test command that needs drive subsystem
	- Test command finishes
	- Expected: Drive command automatically resumes, joysticks work again

Reflection:
	We successfully tested the complete drive system! The robot responds to joystick input, and the default command system works as designed. The clean separation between DriveCommand and DriveSubsystem proved its worth. We'll need to do some tuning of the expo curves deadbands, and pid later, but the core architecture is solid. This success validates all the framework design work we've done over the past months.


