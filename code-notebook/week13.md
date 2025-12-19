11/18/2025
Today's Goals:
	Ian, Antonio, and Philip will be improving the command framework's code organization and creating template files for future development.

Today's Tasks:
	We are enhancing the framework's structure to make it easier for team members to add new commands and subsystems.
	
	Template Files:
	
	We created template files that show the standard structure for new commands and subsystems:
	
```cpp
// template_command.h - Copy this when creating new commands
#pragma once

#include "custom/command/commandBase.h"
#include "custom/subsystem/templateSubsystem.h"

/**
 * @brief Template for creating new commands
 * 
 * To create a new command:
 * 1. Copy this file and rename it (e.g., intakeCommand.h)
 * 2. Rename the class (TemplateCommand → IntakeCommand)
 * 3. Update the subsystem pointer type
 * 4. Implement the lifecycle methods below
 * 5. Add the command to robot.cpp
 */
class TemplateCommand : public CommandBase {
private:
    TemplateSubsystem* m_subsystem;
    // Add any member variables your command needs
    
public:
    /**
     * @brief Construct a new command
     * @param subsystem Pointer to the subsystem this command controls
     */
    TemplateCommand(TemplateSubsystem* subsystem);
    
    /**
     * @brief Called once when the command is first scheduled
     * 
     * Use this to:
     * - Initialize variables to starting values
     * - Set motors to initial positions
     * - Start timers
     */
    void initialize() override;
    
    /**
     * @brief Called repeatedly while the command is running
     * 
     * Use this to:
     * - Update motor speeds based on sensor readings
     * - Process controller input
     * - Update state machines
     */
    void execute() override;
    
    /**
     * @brief Called once when the command ends
     * @param interrupted True if the command was interrupted, false if it finished normally
     * 
     * Use this to:
     * - Stop motors
     * - Clean up resources
     * - Save final state
     */
    void end(bool interrupted) override;
    
    /**
     * @brief Determine if the command has completed
     * @return true if command should end, false if it should continue
     * 
     * Examples:
     * - return false; // Command never ends (good for default commands)
     * - return m_timer.elapsed() > 1000; // End after 1 second
     * - return m_subsystem->isAtTarget(); // End when subsystem reaches goal
     */
    bool isFinished() override;
    
    /**
     * @brief Create a copy of this command
     * @return Pointer to new command instance
     * 
     * Required for command groups and parallel commands.
     * Usually just: return new TemplateCommand(m_subsystem);
     */
    CommandBase* clone() const override;
};
```
	
```cpp
// template_subsystem.h - Copy this when creating new subsystems
#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "pros/motors.hpp"

/**
 * @brief Template for creating new subsystems
 * 
 * To create a new subsystem:
 * 1. Copy this file and rename it (e.g., intakeSubsystem.h)
 * 2. Rename the class (TemplateSubsystem → IntakeSubsystem)
 * 3. Add motor/sensor declarations
 * 4. Implement control methods
 * 5. Register the subsystem in robot.cpp
 */
class TemplateSubsystem : public SubsystemBase {
private:
    // Declare motors, sensors, and other hardware here
    pros::Motor m_motor;
    
    // Add any state variables
    bool m_isActive;
    
public:
    /**
     * @brief Construct the subsystem
     * 
     * Initialize motors and sensors here.
     * Example: m_motor(1, pros::MotorGearset::green)
     */
    TemplateSubsystem();
    
    /**
     * @brief Called repeatedly by the scheduler
     * 
     * Use this to:
     * - Update sensor readings
     * - Run control loops
     * - Update telemetry
     * - Monitor safety conditions
     */
    void periodic() override;
    
    /**
     * @brief Get the subsystem name for debugging
     */
    const char* getName() const override {
        return "TemplateSubsystem";
    }
    
    // Add control methods here
    
    /**
     * @brief Example: Set motor speed
     * @param speed Speed from -127 to 127
     */
    void setSpeed(int speed);
    
    /**
     * @brief Example: Stop all motors
     */
    void stop();
    
    /**
     * @brief Example: Check if subsystem is ready
     */
    bool isReady() const;
};
```
	
	Directory Structure Guide:
	
```markdown
# Command Framework Directory Structure

include/custom/
├── command/
│   ├── commandBase.h          # Base class for all commands
│   ├── template_command.h     # Template for new commands
│   ├── driveCommand.h         # Drive system command
│   └── [your_command.h]       # Your command here
│
├── subsystem/
│   ├── subsystemBase.h        # Base class for all subsystems
│   ├── template_subsystem.h   # Template for new subsystems
│   ├── driveSubsystem.h       # Drive system subsystem
│   └── [your_subsystem.h]     # Your subsystem here
│
├── controller.h               # Controller bindings
├── scheduler.h                # Command scheduler
├── globals.h                  # Global constants and utilities
└── print.h                    # Formatted printing utilities

src/custom/
├── command/
│   ├── driveCommand.cpp       # Drive command implementation
│   └── [your_command.cpp]     # Your command implementation
│
├── subsystem/
│   ├── driveSubsystem.cpp     # Drive subsystem implementation
│   └── [your_subsystem.cpp]   # Your subsystem implementation
│
├── controller.cpp             # Controller implementation
└── scheduler.cpp              # Scheduler implementation

How to add new functionality:
1. Copy template files from include/custom/command/ or subsystem/
2. Rename files and classes
3. Implement required methods
4. Create corresponding .cpp file in src/custom/
5. Register in robot.cpp
```

Reflection:
	We created comprehensive template files that make it much easier to add new functionality to the framework. The templates include detailed comments explaining when to use each method and what code should go where. The directory structure guide gives a clear map of where files belong. These resources will significantly reduce the learning curve for new team members and ensure consistency across the codebase.



11/19/2025
Today's Goals:
	Ian, Antonio, and Philip will be documenting best practices for the command framework and creating coding standards.

Today's Tasks:
	We are establishing coding standards to ensure all team members write consistent, maintainable code.
	
	Command Best Practices:
	
```markdown
# Command Framework Best Practices

## Command Design

### 1. Require Subsystems When Controlling Physical Components
Commands should call addRequirements() when they control physical outputs like motors or pneumatics:

```cpp
// CORRECT - Command controls motors, needs exclusive access
MyCommand(MySubsystem* sub) : m_subsystem(sub) {
    addRequirements(sub);  // Required - prevents multiple commands controlling same motors
}

// ALSO OK - Command only reads sensors, no physical control
SensorMonitorCommand(MySubsystem* sub) : m_subsystem(sub) {
    // No addRequirements() - only reading sensor data, not controlling anything
}
```

Why: The scheduler uses requirements to prevent multiple commands from fighting for control of the same physical hardware (motors, pneumatics, etc.). Reading sensors doesn't require exclusive access, but controlling motors does. Without addRequirements(), multiple commands could send conflicting signals to the same motors simultaneously, causing unpredictable behavior.

### 2. Initialize in initialize(), Not Constructor
Set initial state in initialize(), not the constructor:

```cpp
// CORRECT
void initialize() override {
    m_timer.reset();      // Reset each time command starts
    m_subsystem->reset(); // Reset subsystem state
}

// WRONG - Constructor only runs once when command is created
MyCommand() {
    m_timer.reset();  // This only happens once, not each time command runs!
}
```

Why: Commands can be reused. The constructor runs once when created, but initialize() runs each time the command is scheduled.

### 3. Clean Up in end() (Best Practice)
Always stop motors in end() for safety and predictability:

```cpp
// BEST PRACTICE - Explicit cleanup
void end(bool interrupted) override {
    m_subsystem->stop();  // Always stop, whether interrupted or finished normally
}

// WORKS BUT RISKY - Relies on default command or next command to clean up
void end(bool interrupted) override {
    // No explicit cleanup - motors keep state until something else changes them
}
```

Why: While not strictly required (the next command or default command will control the motors), explicit cleanup in end() is best practice. It makes command behavior predictable, prevents motors from running in unexpected states between commands, and serves as a safety measure if something goes wrong with command scheduling.

### 4. Use isFinished() Appropriately
Different command types need different isFinished() behavior:

```cpp
// CORRECT - Command with timeout ends naturally
bool isFinished() override {
    return m_timer.elapsed() > 2000;  // End after 2 seconds
}

// CORRECT - Command ends when goal reached
bool isFinished() override {
    return m_subsystem->atGoal();
}

// ALSO CORRECT - whileTrue() command runs until button released
bool isFinished() override {
    return false;  // Never ends on its own, waits for button release
}

// CORRECT - Default command runs forever until interrupted
bool isFinished() override {
    return false;  // Never ends, runs until another command needs subsystem
}
```

Why: Commands bound with whileTrue() can return false (they'll be cancelled when button is released). Commands bound with onTrue() should have a finish condition so they don't run forever. Default commands should return false. However, it's best practice to give every non-default, non-whileTrue() command a finish condition (timeout, goal reached, sensor threshold, etc.) to prevent commands from accidentally running forever if something goes wrong.

## Subsystem Design

### 1. No Controller Access in Subsystems
Subsystems should NOT access the controller directly:

```cpp
// WRONG - Subsystem shouldn't know about controller
class DriveSubsystem {
    void update() {
        int speed = controller.get_analog(LEFT_Y);  // BAD!
        m_motor.move(speed);
    }
};

// CORRECT - Command handles controller, subsystem does action
class DriveSubsystem {
    void setSpeed(int speed) {
        m_motor.move(speed);
    }
};

class DriveCommand {
    void execute() {
        int speed = m_controller->get_analog(LEFT_Y);
        m_subsystem->setSpeed(speed);  // Command reads controller, subsystem executes
    }
};
```

Why: This keeps subsystems reusable. The same subsystem can be controlled by buttons, autonomous routines, or test scripts without changes.

### 2. Keep Subsystem Methods Simple
Each method should do one clear thing:

```cpp
// CORRECT - Simple, clear methods
class IntakeSubsystem {
    void intake() { m_motor.move(127); }
    void outtake() { m_motor.move(-127); }
    void stop() { m_motor.move(0); }
};

// WRONG - Method does too many things
class IntakeSubsystem {
    void run(bool intaking, bool outtaking) {
        if (intaking && !outtaking) {
            m_motor.move(127);
        } else if (outtaking && !intaking) {
            m_motor.move(-127);
        } else {
            m_motor.move(0);
        }
    }
};
```

Why: Simple methods are easier to understand, test, and reuse.

### 3. Use periodic() for Monitoring
Use periodic() for continuous updates:

```cpp
void periodic() override {
    // Update sensor readings
    m_position = m_encoder.get_value();
    
    // Safety checks
    if (m_motor.get_temperature() > 55) {
        m_motor.move(0);
        fmt::print("Motor overheating!\n");
    }
    
    // Telemetry
    if (m_debugEnabled) {
        fmt::print("Position: {}\n", m_position);
    }
}
```

Why: periodic() runs continuously without any commands having to run. This is perfect for any monitoring and safety checking.

## Naming Conventions

```cpp
// Classes: PascalCase
class DriveSubsystem { };
class IntakeCommand { };

// Methods: camelCase
void tankDrive();
void setSpeed();

// Member variables: m_camelCase
int m_speed;
DriveSubsystem* m_subsystem;

// Constants: kPascalCase
const int kMotorPort = 1;
const double kWheelDiameter = 3.25;

// Parameters: camelCase (no prefix)
void setSpeed(int speed);
void DriveCommand(DriveSubsystem* subsystem);
```
	
Reflection:
	We documented critical best practices that prevent common bugs and design mistakes. These guidelines clarify when to use each command method, how to structure subsystems, and how to name code elements. Following these practices will prevent issues like command conflicts, memory leaks, and tight coupling between components. This documentation represents lessons learned from weeks of development and debugging.


11/22/2025
