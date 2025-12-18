10/21/2025
Today's Goals:
	Ian, Antonio, and Philip will be reorganizing the repository structure to support multiple robot projects while maintaining a shared command framework.

Today's Tasks:
	We are creating separate project folders for different purposes: competition robots, testing robots, and offseason experimentation. This organization will help us maintain stable competition code while still being able to experiment with new features.
	
	The Problem:
	Currently, all our code is in one project folder (`Test2025/command_test`). This creates several issues:
	- Can't experiment with new features without risking competition code stability
	- Different robots need different subsystems but share the same framework
	- Hard to maintain multiple versions (stable vs development)
	- Risk of accidentally deploying experimental code to competition robot
	
	The Solution - Multi-Project Structure:
	
```
VexU-Code-2025-2026/
├── tundra/*            	# Stable build of current build robot
└── development/
    ├── active-dev-code/*   # Active development and testing
    └── template-code/*     # Clean template for new projects
```
	
	Key Principles:
	
	1. tundra - Stable Competition Robot:
	- Production build for competition use
	- Contains specific for one of our current robots
	- Only updated with tested, stable features
	- This is what we deploy to tournaments
	
	2. development/active-dev-code - Framework Development & Testing:
	- Experimental workspace for testing new framework features
	- Used to develop and validate command patterns (factory methods, functional commands, etc.)
	- Contains exampleSubsystem for framework testing without requiring real hardware
	- Can be broken or unstable - it's a testing ground
	- New framework features are proven here before being added to stable builds
	
	3. development/template-code - Clean Starting Point:
	- Minimal, well-documented template for creating new robot projects
	- Contains framework essentials plus example implementations for learning
	- Includes exampleSubsystem, exampleCommand, and exampleDefaultCommand as teaching examples
	- Copy this folder when starting a new robot project, then customize for specific robot
	
	Shared vs Project-Specific:
	
	Shared Across All Projects (framework core):
	- `commandBase.h` - Command interface defining lifecycle methods
	- `subsystemBase.h` - Subsystem interface with periodic() and default command support
	- `scheduler.h` / `scheduler.cpp` - Scheduler managing command execution
	- `controller.h` / `controller.cpp` - Controller binding system for button mapping
	- `print.h` - Standardized print utilities for debugging
	- `watchdog.h` - Watchdog timer for safety monitoring
	
	Project-Specific (each robot is different):
	- Subsystem implementations: 
	  - `tundra`: driveSubsystem, intakeSubsystem, outtakeSubsystem
	  - `template-code`: driveSubsystem (template), exampleSubsystem (for learning)
	- Command implementations:
	  - `tundra`: driveCommand, runIntakeCommand, runOuttakeCommand
	  - `template-code`: driveCommand (template), exampleCommand, exampleDefaultCommand
	- `robot.cpp` - Robot-specific bindings, subsystem instances, and lifecycle functions
	- `globals.h` - Hardware port assignments and physical constants for specific robot
	
	Migration Process:
	1. Create new folder structure
	2. Copy framework files to each project
	3. Update include paths in all files
	4. Verify each project builds independently
	5. Test on actual hardware
	6. Clean any unnecessary, project-specific, files per file
	7. Verify each project builds independently
	8. Test on actual hardware


Reflection:
	We successfully reorganized the repository into a multi-project structure that exists today! Each project builds independently with the same framework core:
	
	- tundra: Contains our stable competition robot code with intake, outtake, and drive subsystems. This is a production build we deploy to competitions.
	
	- development/active-dev-code: Testing grounds for new features and experimental changes without risking competition code.
	
	- development/template-code: Clean starting point with example subsystems (exampleSubsystem) and commands (exampleCommand, exampleDefaultCommand) for learning and creating new robot projects.
	
	All three projects share the same framework architecture (commandBase, subsystemBase, scheduler, controller, watchdog) but have different subsystem and command implementations based on their purpose. The separation makes it clear which files are framework code (kept in sync) versus robot-specific implementations (customized per project).


10/22/2025
Today's Goals:
	Ian, Antonio, and Philip will be setting up the template-code project as a clean starting point for future robot projects.

Today's Tasks:
	We are creating a minimal, well-documented template that can be copied when starting a new robot project.
	
	Template Structure:
	
```
VexU-Code-2025-2026/development/template-code/           # Clean template for new projects
├── include/
│   ├── api.h            # PROS API
│   ├── main.h           # Main project header
│   └── custom/
│       ├── globals.h    # Hardware configuration template
│       ├── print.h      # Print utilities
│       ├── controller.h # Controller binding system
│       ├── scheduler.h  # Command scheduler
│       ├── watchdog.h   # Watchdog timer
│       ├── command/
│       │   ├── commandBase.h           # Command interface
│       │   ├── driveCommand.h          # Template drive command
│       │   ├── exampleCommand.h        # Example instant command
│       │   └── exampleDefaultCommand.h # Example default command
│       └── subsystem/
│           ├── subsystemBase.h    # Subsystem interface
│           ├── driveSubsystem.h   # Template drive subsystem
│           └── exampleSubsystem.h # Example subsystem for learning
├── src/
│   ├── main.cpp         # Main program entry
│   ├── robot.cpp        # Robot template configuration
│   └── custom/
│       ├── controller.cpp   # Controller implementation
│       ├── scheduler.cpp    # Scheduler implementation
│       └── subsystemBase.cpp # Subsystem base implementation
├── firmware/            # PROS libraries
└── static/              # Static assets
```
	
	globals.h Template:
	
```cpp
// globals.h - Hardware configuration template
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "pros/motors.hpp"

namespace globalConst {
    // Controller configuration
    namespace controller {
        constexpr pros::controller_id_e_t kMainControllerID = pros::E_CONTROLLER_MASTER;
    }
    
    // Drive subsystem configuration
    namespace drive {
        // Motor ports (CHANGE THESE for your robot)
        constexpr std::initializer_list<int8_t> kLeftMotorsID = {1, 2, 3};   // Left side ports
        constexpr std::initializer_list<int8_t> kRightMotorsID = {-4, -5, -6}; // Right side (negative = reversed)
        
        // Motor cartridge
        constexpr pros::MotorGearset kDriveTrainColor = pros::MotorGearset::blue; // 600 RPM
        
        // Drivetrain physical constants (MEASURE your robot)
        constexpr double kWheelDiameter = 3.25;     // inches
        constexpr double kWheelTrack = 12.5;        // inches between left and right wheels
        constexpr double kWheelRPM = 600;           // Motor cartridge RPM
        constexpr double kHorizontalDrift = 2.0;    // Drift correction factor
        
        // Controller settings
        constexpr pros::controller_analog_e_t kLeftStickY = pros::E_CONTROLLER_ANALOG_LEFT_Y;
        constexpr pros::controller_analog_e_t kRightStickY = pros::E_CONTROLLER_ANALOG_RIGHT_Y;
        constexpr int joystickDeadband = 5;         // Ignore small movements
        constexpr double expoCurve = 1.5;           // Exponential curve for fine control
    }
    
    // Add more subsystem configurations here as you add subsystems:
    // namespace intake { ... }
    // namespace lift { ... }
    // namespace shooter { ... }
}

#endif // GLOBALS_H_
```
	
	robot.cpp Template:
	
```cpp
// robot.cpp - Robot-specific configuration and lifecycle functions
#include "main.h"

// Controller instance
Controller controller(globalConst::controller::kMainControllerID);

// ========== SUBSYSTEM INSTANTIATION ==========
// Create all subsystems here
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();
// std::unique_ptr<DriveSubsystem> driveSub = std::make_unique<DriveSubsystem>();

// ========== COMMAND CREATION ==========
// Create commands here
std::unique_ptr<CommandBase> exampleCommand = std::make_unique<ExampleCommand>(exampleSub.get());

// ========== BUTTON BINDINGS ==========
void configureBindings() {
    // Configure all button bindings here
    // Example:
    // controller.setButtonCommand().onTrue(BUTTON_A, exampleCommand.get());
}

// ========== ROBOT LIFECYCLE FUNCTIONS ==========

void robotInit() {
    // Called once when robot powers on
    // Set default commands, initialize hardware, etc.
}

void robotDisabled() {
    // Called repeatedly while robot is disabled
}

void robotCompInit() {
    // Called when connected to field control
    // Show autonomous selector, etc.
}

void robotAuto() {
    // Called repeatedly during autonomous
}

void robotTeleop() {
    // Called repeatedly during driver control
    // Display debug info, update dashboard, etc.
}
```
	
	README.md for Template:
	
```markdown
# VEX Command-Based Framework Template

## Quick Start
1. Copy this entire folder to create a new robot project
2. Update `globals.h` with your robot's motor ports and physical constants
3. Create subsystems in `include/custom/subsystem/`
4. Create commands in `include/custom/command/`
5. Configure button bindings in `robot.cpp::configureBindings()`
6. Build and upload to robot!

## File Structure
- `include/custom/command/` - Your command classes
- `include/custom/subsystem/` - Your subsystem classes
- `src/custom/` - Implementation files (.cpp)
- `robot.cpp` - Robot configuration (bindings, lifecycle)
- `main.cpp` - Main program (rarely needs changes)
- `globals.h` - Hardware configuration

## Adding a New Subsystem
1. Create header file: `include/custom/subsystem/MySubsystem.h`
2. Inherit from `SubsystemBase`
3. Add hardware members (motors, sensors, etc.)
4. Add public methods that commands can call
5. Instantiate in `robot.cpp`

## Adding a New Command
1. Create header file: `include/custom/command/MyCommand.h`
2. Inherit from `CommandBase`
3. Implement lifecycle methods (initialize, execute, end, isFinished)
4. Add requirements in constructor
5. Create instance in `robot.cpp`
6. Bind to button in `configureBindings()`
```

Reflection:
	We successfully created a comprehensive template project that serves as the foundation for new robot projects! The template includes:
	
	Framework Components (shared across all projects):
	- Command scheduler and command base class
	- Subsystem base class with periodic execution
	- Controller binding system
	- Print utilities and watchdog safety timer
	
	Learning Examples:
	- `exampleSubsystem.h` - Demonstrates basic subsystem structure
	- `exampleCommand.h` - Shows instant command pattern
	- `exampleDefaultCommand.h` - Shows default command pattern for continuous subsystem control
	- `driveCommand.h` & `driveSubsystem.h` - Template implementations for tank drive
	
	The template is well-organized with clear separation between framework code (in custom/) and robot-specific configuration (robot.cpp, globals.h). The globals.h template provides clear placeholders for motor ports and physical constants. This structure makes it easy to start a new robot project by copying template-code and customizing only the robot-specific parts while keeping the proven framework intact.
