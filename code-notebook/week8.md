10/21/2025
Today's Goals:
	Ian, Antonio, and Philip will be reorganizing the repository structure to support multiple robot projects while maintaining a shared command framework.

Today's Tasks:
	We are creating separate project folders for different purposes: competition robots, testing robots, and offseason experimentation. This organization will help us maintain stable competition code while still being able to experiment with new features.
	
	**The Problem:**
	Currently, all our code is in one project folder (`Test2025/command_test`). This creates several issues:
	- Can't experiment with new features without risking competition code stability
	- Different robots need different subsystems but share the same framework
	- Hard to maintain multiple versions (stable vs development)
	- Risk of accidentally deploying experimental code to competition robot
	
	**The Solution - Multi-Project Structure:**
	
```
VexU-Code-2025-2026/
├── Comp2024/
│   └── 24in-code/              # Competition robot - STABLE CODE ONLY
│       ├── include/custom/
│       ├── src/custom/
│       └── firmware/
├── Test2025/
│   └── command_test/            # Original framework development
│       ├── include/custom/
│       ├── src/custom/
│       └── firmware/
└── Offseason/
    ├── dev-code/                # Active development and testing
    │   ├── include/custom/
    │   ├── src/custom/
    │   └── firmware/
    └── template-code/           # Clean template for new projects
        ├── include/custom/
        ├── src/custom/
        └── firmware/
```
	
	**Key Principles:**
	
	**1. Comp2024 - Production Code:**
	- Only tested, working features
	- Never experimental or in-development code
	- Code freeze before competitions
	- Always deployable to competition robot
	
	**2. Test2025 - Framework Development:**
	- Original development workspace
	- Historical reference for how framework was built
	- Kept as archive of development process
	
	**3. Offseason/dev-code - Active Development:**
	- Experiment with new features
	- Test risky changes
	- Debug complex issues
	- Can be broken occasionally - that's okay!
	
	**4. Offseason/template-code - Starting Point:**
	- Clean, minimal implementation
	- Well-documented for learning
	- Used when creating new robot projects
	- Contains only framework essentials
	
	**Shared vs Project-Specific:**
	
	**Shared Across All Projects (framework core):**
	- `commandBase.h` - Command interface
	- `subsystemBase.h` - Subsystem interface
	- `scheduler.h` / `scheduler.cpp` - Scheduler implementation
	- `controller.h` / `controller.cpp` - Controller binding system
	
	**Project-Specific (each robot is different):**
	- Subsystem implementations (DriveSubsystem, IntakeSubsystem, etc.)
	- Command implementations (specific to robot mechanisms)
	- `robot.cpp` - Robot-specific bindings and configuration
	- `globals.h` - Hardware port assignments
	
	**Migration Process:**
	1. Create new folder structure
	2. Copy framework files to each project
	3. Update include paths in all files
	4. Verify each project builds independently
	5. Test on actual hardware
	
```cpp
// Updated include paths in all files
// Old:
#include "command/commandBase.h"

// New:
#include "custom/command/commandBase.h"
```

Reflection:
	We successfully reorganized the repository into a multi-project structure! Each project now builds independently and we can work on experimental features in dev-code without risking competition code stability. The separation of shared framework code from robot-specific code makes it clear which files need to be kept in sync across projects. This organization will serve us well throughout the season as we maintain competition robots while continuing to develop new features.

**[PHOTO NEEDED: Repository folder structure diagram showing four projects with their purposes labeled]**


10/22/2025
Today's Goals:
	Ian, Antonio, and Philip will be setting up the template-code project as a clean starting point for future robot projects.

Today's Tasks:
	We are creating a minimal, well-documented template that can be copied when starting a new robot project.
	
	**Template Structure:**
	
```
template-code/
├── include/
│   ├── api.h                    # PROS API
│   ├── main.h                   # Main project header
│   └── custom/
│       ├── globals.h            # Hardware configuration
│       ├── print.h              # Print utilities
│       ├── controller.h         # Controller binding
│       ├── scheduler.h          # Command scheduler
│       ├── command/
│       │   ├── commandBase.h    # Command interface
│       │   ├── exampleCommand.h # Example commands for learning
│       │   └── driveCommand.h   # Template drive command
│       └── subsystem/
│           ├── subsystemBase.h      # Subsystem interface
│           ├── exampleSubsystem.h   # Example subsystem
│           └── driveSubsystem.h     # Template drive subsystem
├── src/
│   ├── main.cpp                 # Main program entry
│   ├── robot.cpp                # Robot-specific code
│   └── custom/
│       ├── controller.cpp       # Controller implementation
│       ├── scheduler.cpp        # Scheduler implementation
│       └── subsystemBase.cpp    # Subsystem base implementation
└── firmware/                    # PROS libraries (LemLib, LVGL, etc.)
```
	
	**globals.h Template:**
	
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
	
	**robot.cpp Template:**
	
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
	
	**README.md for Template:**
	
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
	We successfully created a comprehensive template project that includes everything needed to start a new robot project. The template is well-organized, fully documented, and includes example subsystems and commands for learning. The globals.h template makes it easy to configure hardware by changing a few constants rather than searching through code. The README provides clear instructions for using the template. This will dramatically speed up the process of creating new robot projects and help new team members get started quickly.

**[PHOTO NEEDED: Template folder structure with annotations showing what each section contains]**
**[PHOTO NEEDED: globals.h template showing configuration constants with comments]**
