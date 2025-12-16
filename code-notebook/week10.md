11/04/2025
Today's Goals:
	Ian, Antonio, and Philip will be transitioning from framework development to application development by implementing the drive subsystem for actual robot control.

Today's Tasks:
	We are creating the drive subsystem - the first "real" robot subsystem that will control an actual drivetrain. This marks a transition from building the framework to using it for practical robotics.
	
	**DriveSubsystem Architecture:**
	
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
    void arcadeDrive(int forward, int turn);
    void stop();
    
    // Autonomous methods (will add later)
    void driveTo(double x, double y, double timeout);
    void turnTo(double angle, double timeout);
    
    // Sensor access
    lemlib::Pose getPose();  // Get robot position
    
    // Subsystem periodic
    void periodic() override;
};

#endif
```
	
	**What is LemLib?**
	LemLib is an external library that provides advanced motion control for VEX robots. Think of it as a "smart driving assistant" that handles complex math for us:
	
	- **Odometry**: Tracks where the robot is on the field using wheel encoders and IMU
	- **Motion Profiling**: Smoothly accelerates and decelerates (no wheel slippage)
	- **PID Control**: Accurately reaches target speeds and positions
	- **Path Following**: Drives along predetermined paths for autonomous
	
	Instead of us writing all that math, we can use LemLib's tested implementations.
	
	**Motor Groups:**
	
	A `MotorGroup` treats multiple motors as one unit. If your left drive has 3 motors, you put them in a group. Then `leftMotors.move(127)` spins all three at full speed automatically.
	
```cpp
// In globals.h - Configuration
namespace drive {
    constexpr std::initializer_list<int8_t> kLeftMotorsID = {1, 2, 3};      // Ports 1,2,3
    constexpr std::initializer_list<int8_t> kRightMotorsID = {-4, -5, -6};  // Ports 4,5,6 (reversed)
    constexpr pros::MotorGearset kDriveTrainColor = pros::MotorGearset::blue; // 600 RPM cartridge
}

// In driveSubsystem.h - Constructor
DriveSubsystem::DriveSubsystem()
    : m_left_motor_group(globalConst::drive::kLeftMotorsID, globalConst::drive::kDriveTrainColor),
      m_right_motor_group(globalConst::drive::kRightMotorsID, globalConst::drive::kDriveTrainColor) {
    // Motors are now ready to use
}
```
	
	Notice the negative signs on right motors: `-4, -5, -6`. This reverses those motors because physically, the right side motors face the opposite direction from the left side motors.
	
	**Tank Drive Implementation:**
	
	Tank drive: left stick controls left side, right stick controls right side.
	
```cpp
void DriveSubsystem::tankDrive(Controller* controller, bool inverted) {
    // Read joystick values (-127 to +127)
    int leftY = controller->get_analog(globalConst::drive::kLeftStickY);
    int rightY = controller->get_analog(globalConst::drive::kRightStickY);
    
    if (inverted) {
        // Swap sides for inverted driving
        leftY = -leftY;
        rightY = -rightY;
    }
    
    // Send to chassis (LemLib applies expo curve automatically)
    m_chassis.tank(leftY, rightY);
}
```
	
	**Expo Curves:**
	
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

Reflection:
	We successfully created the drive subsystem architecture! While we haven't tested it on hardware yet (no robot built), the code compiles and the structure is solid. The integration with LemLib will give us powerful motion control capabilities for autonomous. The motor group abstraction makes it easy to control multiple motors as one unit. Next, we need to create the drive command that will use this subsystem during driver control.

**[PHOTO NEEDED: Diagram showing DriveSubsystem → LemLib → Motor Groups architecture with arrows showing data flow]**


11/05/2025
Today's Goals:
	Ian, Antonio, and Philip will be creating the drive command and testing the complete drive system.

Today's Tasks:
	We are implementing the drive command that will run as the default command during driver control, allowing operators to drive the robot with joysticks.
	
	**DriveCommand Implementation:**
	
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
	
	**Why This Design is Beautiful:**
	
	The command is incredibly simple because all the complexity is hidden in the subsystem:
	1. Read joysticks
	2. Pass values to subsystem
	3. Subsystem handles motor control, expo curves, LemLib integration, etc.
	
	This separation of concerns means:
	- Command is easy to understand and test
	- Subsystem can be reused by other commands
	- Complex drive logic is in one place
	
	**Setting as Default Command:**
	
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
	
	Now whenever the drive subsystem is idle (no autonomous command using it), the robot responds to joystick input automatically!
	
	**Testing on Hardware:**
	We tested with an actual robot for the first time:
	
	**Test 1: Basic Tank Drive**
	- Left stick forward → Left side moves forward
	- Right stick forward → Right side moves forward
	- Both sticks forward → Robot drives straight forward
	- Result: ✅ PASS - Smooth, responsive control
	
	**Test 2: Turning**
	- Left stick forward, right stick backward → Spins right
	- Left stick backward, right stick forward → Spins left
	- Result: ✅ PASS - Turns in place smoothly
	
	**Test 3: Expo Curve**
	- Small joystick movements → Robot moves slowly (precise)
	- Large joystick movements → Robot moves fast
	- Result: ✅ PASS - Driver can make fine adjustments
	
	**Test 4: Subsystem Conflict**
	- Driving with joysticks (default command running)
	- Press button to run test command (also needs drive subsystem)
	- Expected: Drive command interrupted, test command takes over
	- Result: ✅ PASS - Smooth transition
	
	**Test 5: Default Command Resume**
	- Run test command that needs drive subsystem
	- Test command finishes
	- Expected: Drive command automatically resumes, joysticks work again
	- Result: ✅ PASS - Seamless resume of driver control
	
	**Discovered Issues and Fixes:**
	
	**Issue 1: Motors Running in Wrong Direction**
	- Problem: Left motors were reversed
	- Fix: Changed motor IDs in globals.h from `{1,2,3}` to `{-1,-2,-3}`
	
	**Issue 2: Deadband Too Large**
	- Problem: Small movements ignored (hard to drive straight)
	- Fix: Reduced deadband from 10 to 5
	
	**Issue 3: Expo Too Aggressive**
	- Problem: Hard to access full speed
	- Fix: Changed exponent from 2.0 to 1.5

Reflection:
	We successfully implemented and tested the complete drive system! The robot responds smoothly to joystick input, the expo curves make it easy to drive precisely, and the default command system works perfectly - the robot always responds to joysticks unless another command explicitly takes control. This is the first time we've used our command framework to control real hardware, and it worked beautifully. The clean separation between DriveCommand and DriveSubsystem makes the code easy to understand and maintain. This success validates all the framework design work we've done over the past months.

**[PHOTO NEEDED: Video stills showing robot responding to controller input - stopped, turning left, driving forward, turning right]**
**[PHOTO NEEDED: Code showing simple DriveCommand execute() method - just one line calling subsystem.tankDrive()]**
