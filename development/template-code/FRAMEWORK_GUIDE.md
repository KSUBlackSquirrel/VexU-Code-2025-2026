# Command Framework Directory Structure

This guide explains how the command-based framework is organized and where to put your code.

## Directory Structure

```
include/custom/
├── command/
│   ├── commandBase.h          # Base class for all commands
│   ├── template_command.h     # Template for new commands (COPY THIS!)
│   └── [your_command.h]       # Your command here
│
├── subsystem/
│   ├── subsystemBase.h        # Base class for all subsystems
│   ├── template_subsystem.h   # Template for new subsystems (COPY THIS!)
│   └── [your_subsystem.h]     # Your subsystem here
│
├── controller.h               # Controller bindings
├── scheduler.h                # Command scheduler
├── globals.h                  # Global constants and utilities
└── print.h                    # Formatted printing utilities

src/
├── main.cpp                   # PROS entry point (opcontrol, autonomous, etc.)
├── robot.cpp                  # Robot configuration (subsystems, commands, bindings)
│
└── custom/
    ├── command/
    │   ├── driveCommand.cpp       # Drive command implementation (if needed)
    │   └── [your_command.cpp]     # Your command implementation (if needed)
    │
    ├── subsystem/
    │   ├── driveSubsystem.cpp     # Drive subsystem implementation (if needed)
    │   └── [your_subsystem.cpp]   # Your subsystem implementation (if needed)
    │
    ├── controller.cpp             # Controller implementation
    └── scheduler.cpp              # Scheduler implementation
```

## How to Add New Functionality

### Adding a New Command

1. Copy `include/custom/command/template_command.h`
2. Rename the file (e.g., `intakeCommand.h`)
3. Rename the class (e.g., `TemplateCommand` → `IntakeCommand`)
4. Update the subsystem pointer type
5. Implement the lifecycle methods (initialize, execute, end, isFinished)
6. If your command has complex logic, create `src/custom/command/intakeCommand.cpp`
7. Register the command in `robot.cpp`:
   ```cpp
   std::unique_ptr<IntakeCommand> intakeCmd;
   intakeCmd = std::make_unique<IntakeCommand>(intakeSub.get());
   controller.R1().whileTrue(intakeCmd.get());
   ```

### Adding a New Subsystem

1. Copy `include/custom/subsystem/template_subsystem.h`
2. Rename the file (e.g., `intakeSubsystem.h`)
3. Rename the class (e.g., `TemplateSubsystem` → `IntakeSubsystem`)
4. Add motor/sensor declarations in the private section
5. Implement control methods (setSpeed, stop, etc.)
6. If your subsystem has complex logic, create `src/custom/subsystem/intakeSubsystem.cpp`
7. Create the subsystem in `robot.cpp`:
   ```cpp
   std::unique_ptr<IntakeSubsystem> intakeSub;
   intakeSub = std::make_unique<IntakeSubsystem>();
   ```

## Command Framework Best Practices

### Command Design

#### 1. Require Subsystems When Controlling Physical Components

Commands should call `addRequirements()` when they control physical outputs like motors or pneumatics:

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

**Why:** The scheduler uses requirements to prevent multiple commands from fighting for control of the same physical hardware. Reading sensors doesn't require exclusive access, but controlling motors does.

#### 2. Initialize in initialize(), Not Constructor

Set initial state in `initialize()`, not the constructor:

```cpp
// CORRECT
void initialize() override {
    m_timer.reset();      // Reset each time command starts
    m_subsystem->reset(); // Reset subsystem state
}

// WRONG - Constructor only runs once
MyCommand() {
    m_timer.reset();  // This only happens once, not each time command runs!
}
```

**Why:** Commands can be reused. The constructor runs once when created, but `initialize()` runs each time the command is scheduled.

#### 3. Clean Up in end() (Best Practice)

Always stop motors in `end()` for safety and predictability:

```cpp
// BEST PRACTICE - Explicit cleanup
void end(bool interrupted) override {
    m_subsystem->stop();  // Always stop, whether interrupted or finished normally
}

// WORKS BUT RISKY - Relies on default command to clean up
void end(bool interrupted) override {
    // No explicit cleanup - motors keep state until something else changes them
}
```

**Why:** While not strictly required (the next command will control the motors), explicit cleanup makes command behavior predictable and serves as a safety measure.

#### 4. Use isFinished() Appropriately

Different command types need different `isFinished()` behavior:

```cpp
// CORRECT - Command with timeout
bool isFinished() override {
    return m_timer.get_time() > 2000;  // End after 2 seconds
}

// CORRECT - whileTrue() command
bool isFinished() override {
    return false;  // Never ends on its own, cancelled when button released
}

// CORRECT - Default command
bool isFinished() override {
    return false;  // Never ends, runs until another command needs subsystem
}
```

**Why:** Commands bound with `whileTrue()` can return false (they'll be cancelled when button is released). Commands bound with `onTrue()` should have a finish condition. Default commands should return false. Best practice: give every non-default, non-whileTrue() command a finish condition to prevent running forever.

### Subsystem Design

#### 1. No Controller Access in Subsystems

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

**Why:** This keeps subsystems reusable. The same subsystem can be controlled by buttons, autonomous routines, or test scripts without changes.

#### 2. Keep Subsystem Methods Simple

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

**Why:** Simple methods are easier to understand, test, and reuse.

#### 3. Use periodic() for Monitoring

Use `periodic()` for continuous updates:

```cpp
void periodic() override {
    // Update sensor readings
    m_position = m_encoder.get_value();
    
    // Safety checks
    if (m_motor.get_temperature() > 55) {
        m_motor.move(0);
        customPrint::printf("Motor overheating!\n");
    }
    
    // Telemetry
    if (m_debugEnabled) {
        customPrint::printf("Position: %d\n", m_position);
    }
}
```

**Why:** `periodic()` runs continuously without any commands having to run. Perfect for monitoring and safety checking.

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

## Quick Start Examples

### Simple Button-Controlled Subsystem

```cpp
// 1. Create subsystem (intakeSubsystem.h)
class IntakeSubsystem : public SubsystemBase {
private:
    pros::Motor m_motor;
public:
    IntakeSubsystem() : m_motor(1, pros::MotorGearset::green) {}
    void intake() { m_motor.move(127); }
    void stop() { m_motor.move(0); }
    const char* getName() const override { return "IntakeSubsystem"; }
};

// 2. Create command (intakeCommand.h)
class IntakeCommand : public CommandBase {
private:
    IntakeSubsystem* m_subsystem;
public:
    IntakeCommand(IntakeSubsystem* sub) : m_subsystem(sub) {
        addRequirements(sub);
    }
    void execute() override { m_subsystem->intake(); }
    void end(bool interrupted) override { m_subsystem->stop(); }
    bool isFinished() override { return false; }
    CommandBase* clone() const override { return new IntakeCommand(m_subsystem); }
};

// 3. Register in robot.cpp
std::unique_ptr<IntakeSubsystem> intakeSub;
std::unique_ptr<IntakeCommand> intakeCmd;

void initialize() {
    intakeSub = std::make_unique<IntakeSubsystem>();
    intakeCmd = std::make_unique<IntakeCommand>(intakeSub.get());
    controller.R1().whileTrue(intakeCmd.get());
}
```

That's it! The subsystem runs while R1 is held, stops when released.
