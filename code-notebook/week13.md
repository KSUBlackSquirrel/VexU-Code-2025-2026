11/18/2025
Today's Goals:
	Ian, Antonio, and Philip will be improving the command framework's code organization and creating template files for future development.

Today's Tasks:
	We are enhancing the framework's structure to make it easier for team members to add new commands and subsystems.
	
	**Template Files:**
	
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
	
	**Directory Structure Guide:**
	
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
	
	**Command Best Practices:**
	
```markdown
# Command Framework Best Practices

## Command Design

### 1. Always Require Subsystems
Every command that uses a subsystem MUST call addRequirements():

```cpp
// ✅ CORRECT
MyCommand(MySubsystem* sub) : m_subsystem(sub) {
    addRequirements(sub);  // Required!
}

// ❌ WRONG - Scheduler doesn't know this command uses the subsystem
MyCommand(MySubsystem* sub) : m_subsystem(sub) {
    // Missing addRequirements()!
}
```

**Why:** The scheduler uses requirements to prevent command conflicts. Without addRequirements(), multiple commands can fight for control of the same subsystem.

### 2. Initialize in initialize(), Not Constructor
Set initial state in initialize(), not the constructor:

```cpp
// ✅ CORRECT
void initialize() override {
    m_timer.reset();      // Reset each time command starts
    m_subsystem->reset(); // Reset subsystem state
}

// ❌ WRONG - Constructor only runs once when command is created
MyCommand() {
    m_timer.reset();  // This only happens once, not each time command runs!
}
```

**Why:** Commands can be reused. The constructor runs once when created, but initialize() runs each time the command is scheduled.

### 3. Clean Up in end()
Always stop motors in end():

```cpp
// ✅ CORRECT
void end(bool interrupted) override {
    m_subsystem->stop();  // Always stop, whether interrupted or finished normally
}

// ❌ WRONG - Motors keep running after command ends
void end(bool interrupted) override {
    // No cleanup - motors still moving!
}
```

**Why:** Commands can be interrupted at any time. Always clean up properly to avoid runaway motors.

### 4. Use isFinished() for Temporary Commands
Commands that should run until a condition is met:

```cpp
// ✅ CORRECT - Command ends when goal is reached
bool isFinished() override {
    return m_subsystem->atGoal();
}

// ✅ CORRECT - Command ends after timeout
bool isFinished() override {
    return m_timer.elapsed() > 2000;  // End after 2 seconds
}

// ❌ WRONG for buttons - Command never ends
bool isFinished() override {
    return false;  // This is only correct for default commands!
}
```

**Why:** Commands bound to buttons with onTrue() should end naturally. Commands that never end prevent other commands from running.

### 5. Default Commands Never End
Commands set as defaults should run forever:

```cpp
// ✅ CORRECT for default commands
bool isFinished() override {
    return false;  // Never ends, runs until interrupted
}

// ❌ WRONG for default commands
bool isFinished() override {
    return true;  // Ends immediately, defeats the purpose!
}
```

**Why:** Default commands run when no other command needs the subsystem. They should continue until interrupted.

## Subsystem Design

### 1. No Controller Access in Subsystems
Subsystems should NOT access the controller directly:

```cpp
// ❌ WRONG - Subsystem shouldn't know about controller
class DriveSubsystem {
    void update() {
        int speed = controller.get_analog(LEFT_Y);  // BAD!
        m_motor.move(speed);
    }
};

// ✅ CORRECT - Command handles controller, subsystem does action
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

**Why:** This keeps subsystems reusable. The same subsystem can be controlled by buttons, autonomous routines, or test scripts without changes.

### 2. Keep Subsystem Methods Simple
Each method should do one clear thing:

```cpp
// ✅ CORRECT - Simple, clear methods
class IntakeSubsystem {
    void intake() { m_motor.move(127); }
    void outtake() { m_motor.move(-127); }
    void stop() { m_motor.move(0); }
};

// ❌ WRONG - Method does too many things
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

**Why:** Simple methods are easier to understand, test, and reuse.

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

**Why:** periodic() runs continuously, perfect for monitoring and safety.

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
constexpr int kMotorPort = 1;
constexpr double kWheelDiameter = 3.25;

// Parameters: camelCase (no prefix)
void setSpeed(int speed);
void DriveCommand(DriveSubsystem* subsystem);
```
	
Reflection:
	We documented critical best practices that prevent common bugs and design mistakes. These guidelines clarify when to use each command method, how to structure subsystems, and how to name code elements. Following these practices will prevent issues like command conflicts, memory leaks, and tight coupling between components. This documentation represents lessons learned from weeks of development and debugging.

**[PHOTO NEEDED: Best practices reference card showing command lifecycle]**
**[PHOTO NEEDED: Do's and Don'ts comparison showing correct vs incorrect patterns]**


11/22/2025
