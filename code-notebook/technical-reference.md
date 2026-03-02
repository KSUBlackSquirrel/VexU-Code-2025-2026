# Technical Reference Guide
## Command-Based Framework for VEX Robotics

*Last Updated: December 19, 2025*

---

## Table of Contents
1. [Framework Overview](#framework-overview)
2. [Core Architecture](#core-architecture)
3. [Command Lifecycle](#command-lifecycle)
4. [Subsystem Design](#subsystem-design)
5. [Button Bindings](#button-bindings)
6. [Command Groups](#command-groups)
7. [Directory Structure](#directory-structure)
8. [Best Practices](#best-practices)
9. [Common Issues & Solutions](#common-issues--solutions)
10. [API Quick Reference](#api-quick-reference)

---

## Framework Overview

This is a custom command-based framework for VEX robotics, inspired by FRC's WPILib. Built from scratch for PROS and VEX V5, it provides a modular, maintainable architecture for robot control.

### Key Features
- **Command-based architecture** - Modular, reusable robot behaviors
- **Automatic subsystem management** - Scheduler prevents resource conflicts
- **Flexible button bindings** - onTrue(), onFalse(), whileTrue() patterns
- **Command composition** - SequentialCommandGroup, ParallelCommandGroup
- **FRC-inspired patterns** - Familiar to teams with FRC experience
- **Auto-registration** - Subsystems and controllers register automatically

### Project Structure
```
VexU-Code-2025-2026/
├── tundra/                 # Stable competition robot code
├── development/
│   ├── active-dev-code/    # Active development and testing
│   └── template-code/      # Clean template for new projects
```

---

## Core Architecture

The framework consists of three main components:

### 1. Scheduler (Singleton)
- Manages all command execution
- Handles subsystem requirements and conflicts
- Runs in main loop (opcontrol, autonomous)
- Accessible via `Scheduler::getInstance()`

**Key Methods:**
```cpp
CommandBase* schedule(CommandBase* cmd);  // Schedule a command to run
void cancel(CommandBase* cmd);            // Cancel a running command
bool isScheduled(CommandBase* cmd);       // Check if command is running
void run();                               // Execute one scheduler cycle (call in loops)
```

### 2. Commands (CommandBase)
- Encapsulate robot behaviors (drive, intake, shoot, etc.)
- Have four lifecycle methods: initialize(), execute(), end(), isFinished()
- Declare subsystem requirements to prevent conflicts
- Can be composed into command groups

**Base Class:**
```cpp
class CommandBase {
public:
    virtual void initialize() {}           // Called once when command starts
    virtual void execute() {}              // Called repeatedly while command runs
    virtual void end(bool interrupted) {}  // Called when command ends
    virtual bool isFinished() = 0;         // Return true when command should end
    virtual CommandBase* clone() const = 0; // Create a copy of this command
    virtual std::string getName() const;   // Auto-generated from class name
    
    void addRequirements(SubsystemBase* subsystem);
};
```

### 3. Subsystems (SubsystemBase)
- Represent robot hardware (drive, intake, arm, etc.)
- Have a periodic() method called every loop
- Can have a default command that runs when no other command uses it
- Auto-register with scheduler on construction

**Base Class:**
```cpp
class SubsystemBase {
public:
    virtual void periodic() {}             // Called every loop
    virtual std::string getName() const;   // Auto-generated from class name
    
    void setDefaultCommand(CommandBase* cmd);
    CommandBase* getCurrentCommand() const;
};
```

---

## Command Lifecycle

### Lifecycle Stages

1. **Schedule** - Command is added to scheduler queue
2. **Initialize** - `initialize()` called once
3. **Execute Loop** - `execute()` called repeatedly every cycle
4. **Finish Check** - `isFinished()` checked after each execute
5. **End** - `end(interrupted)` called when finished or cancelled
6. **Cleanup** - Command removed from scheduler

### Visual Flow
```
Button Press
     ↓
[Schedule Command]
     ↓
[initialize()]      ← Run once
     ↓
┌─[execute()]       ← Loop
│    ↓
│ [isFinished()?] ─No─┘
│    ↓ Yes
└→[end(false)]      ← Natural finish

OR

whileTrue Button Release/Conflict
     ↓
[end(true)]         ← Interrupted
```

### When Methods Are Called

**initialize():**
- Called once when command is scheduled
- Use for: resetting timers, setting initial state, zeroing sensors
- NOT called on construction (commands are reusable)

**execute():**
- Called every scheduler cycle (~10ms in opcontrol, ~50ms in autonomous)
- Use for: reading sensors, controlling motors, updating state
- Runs until isFinished() returns true or command is cancelled

**end(interrupted):**
- Called once when command ends
- `interrupted = false` → Command finished naturally (isFinished() returned true)
- `interrupted = true` → Command was cancelled (button released, subsystem conflict, etc.)
- Use for: stopping motors, cleanup, logging results

**isFinished():**
- Called after each execute() cycle
- Return true to end the command naturally
- Return false for commands that run until interrupted (whileTrue bindings, default commands)

### Example Command
```cpp
class DriveForwardCommand : public CommandBase {
private:
    DriveSubsystem* m_drive;
    double m_startTime;
    double m_duration;
    
public:
    DriveForwardCommand(DriveSubsystem* drive, double seconds)
        : m_drive(drive), m_duration(seconds) {
        addRequirements(drive);  // Require drive subsystem
    }
    
    void initialize() override {
        m_startTime = pros::millis();
        customPrint::printf("Starting forward drive\n");
    }
    
    void execute() override {
        m_drive->tankDrive(100, 100);  // Drive forward
    }
    
    void end(bool interrupted) override {
        m_drive->stop();  // Always stop motors
        if (interrupted) {
            customPrint::printf("Drive interrupted!\n");
        } else {
            customPrint::printf("Drive complete\n");
        }
    }
    
    bool isFinished() override {
        double elapsed = (pros::millis() - m_startTime) / 1000.0;
        return elapsed >= m_duration;
    }
    
    CommandBase* clone() const override {
        return new DriveForwardCommand(m_drive, m_duration);
    }
};
```

---

## Subsystem Design

### Subsystem Purpose
- Encapsulate robot hardware (motors, sensors, pneumatics)
- Provide simple control methods for commands to use
- Should NOT contain controller access or decision logic
- Maintain hardware state and safety checks

### Subsystem Structure
```cpp
class ExampleSubsystem : public SubsystemBase {
private:
    // Hardware declarations
    pros::Motor m_motor;
    pros::Rotation m_encoder;
    
    // State tracking
    double m_position;
    bool m_isCalibrated;
    
public:
    // Constructor - initialize hardware
    ExampleSubsystem()
        : m_motor(1, pros::MotorGearset::green),
          m_encoder(2),
          m_position(0),
          m_isCalibrated(false)
    {}
    
    // Control methods - called by commands
    void setSpeed(int speed) {
        m_motor.move(speed);
    }
    
    void stop() {
        m_motor.move(0);
    }
    
    // State queries - called by commands or periodic()
    double getPosition() const {
        return m_position;
    }
    
    bool isCalibrated() const {
        return m_isCalibrated;
    }
    
    // Periodic - called every loop automatically
    void periodic() override {
        // Update sensor readings
        m_position = m_encoder.get_position() / 100.0;
        
        // Safety checks
        if (m_motor.get_temperature() > 55) {
            m_motor.move(0);
            customPrint::printf("Motor overheating!\n");
        }
        
        // Telemetry (optional)
        customPrint::screenPrint(0, "Pos: %.2f", m_position);
    }
};
```

### Key Principles

**1. No Controller Access**
```cpp
// WRONG - Subsystem reads controller
class DriveSubsystem {
    void update() {
        int speed = controller.get_analog(LEFT_Y);  // BAD!
        m_motor.move(speed);
    }
};

// CORRECT - Command reads controller, subsystem executes
class DriveSubsystem {
    void tankDrive(int left, int right) {
        m_leftMotors.move(left);
        m_rightMotors.move(right);
    }
};

class DriveCommand {
    void execute() {
        int left = m_controller->get_analog(LEFT_Y);
        int right = m_controller->get_analog(RIGHT_Y);
        m_drive->tankDrive(left, right);
    }
};
```

**2. Simple, Clear Methods**
```cpp
// GOOD - Each method does one thing
class IntakeSubsystem {
    void intake() { m_motor.move(127); }
    void outtake() { m_motor.move(-127); }
    void stop() { m_motor.move(0); }
};

// BAD - Method does too much
class IntakeSubsystem {
    void run(bool in, bool out, int speed) {
        if (in && !out) m_motor.move(speed);
        else if (out && !in) m_motor.move(-speed);
        else m_motor.move(0);
    }
};
```

**3. Use periodic() for Monitoring**
```cpp
void periodic() override {
    // Update cached sensor values
    m_position = m_encoder.get_value();
    m_velocity = m_motor.get_actual_velocity();
    
    // Safety checks
    if (m_temperature > kMaxTemperature) {
        emergencyStop();
    }
    
    // Telemetry (optional)
    if (m_debugMode) {
        customPrint::printf("Subsystem: pos=%.2f vel=%.2f\n", 
                           m_position, m_velocity);
    }
}
```

---

## Button Bindings

The framework provides three binding types, matching FRC patterns.

### onTrue() - Trigger on Press
Command starts once when button is pressed, runs until finished.

```cpp
// Command runs once per button press
controller.A().onTrue(new ShootCommand(shooterSub));

// Press A → Command starts
// Hold A → Nothing (command continues running)
// Release A → Nothing (command finishes on its own)
```

**Use Cases:**
- One-shot actions (shoot, toggle, pulse)
- Commands with built-in finish conditions
- Actions that should complete even if button released

### onFalse() - Trigger on Release
Command starts once when button is released, runs until finished.

```cpp
// Command runs once when button is released
controller.A().onFalse(new StopCommand(intakeSub));

// Press A → Nothing
// Hold A → Nothing
// Release A → Command starts
```

**Use Cases:**
- Cleanup actions
- Toggle-off behaviors
- Actions triggered by button release

### whileTrue() - Hold to Run
Command starts when button pressed, cancelled when button released.

```cpp
// Command runs while button held
controller.A().whileTrue(new IntakeCommand(intakeSub));

// Press A → Command starts
// Hold A → Command keeps running
// Release A → Command cancelled (end(true) called)
```

**Use Cases:**
- Hold-to-run actions (drive, intake, manual control)
- Actions that should stop immediately when button released
- Most common binding type for driver control

### Binding Examples
```cpp
// In robot.cpp - configureBindings()
void configureBindings() {
    // Drive control - hold to drive
    controller.UP().whileTrue(driveForwardCmd.get());
    controller.DOWN().whileTrue(driveBackwardCmd.get());
    
    // Intake - hold to run
    controller.R1().whileTrue(intakeCmd.get());
    controller.R2().whileTrue(outtakeCmd.get());
    
    // Shoot - press to shoot (finishes automatically)
    controller.A().onTrue(shootCmd.get());
    
    // Toggle pneumatic - press to toggle
    controller.B().onTrue(toggleClawCmd.get());
}
```

---

## Command Groups

Command groups allow composing simple commands into complex behaviors, following FRC's proven patterns.

### SequentialCommandGroup
Runs commands one after another in sequence.

```cpp
// Create sequential group
auto autoSequence = std::make_unique<SequentialCommandGroup>(
    driveForwardCmd.get(),
    turnRightCmd.get(),
    intakeCmd.get()
);

// Equivalent using andThen() decorator
auto autoSequence = driveForwardCmd->andThen(turnRightCmd.get())
                                    ->andThen(intakeCmd.get());
```

**Lifecycle:**
1. Command 1 initialize() → execute() loop → end(false) when finished
2. Command 2 initialize() → execute() loop → end(false) when finished
3. Command 3 initialize() → execute() loop → end(false) when finished
4. Group finishes

**Use Cases:**
- Autonomous routines (drive → turn → score)
- Multi-step mechanisms (extend → grab → retract)
- Any sequence that requires specific order

### ParallelCommandGroup
Runs multiple commands simultaneously until all finish.

```cpp
// Create parallel group
auto parallel = std::make_unique<ParallelCommandGroup>(
    driveForwardCmd.get(),
    runIntakeCmd.get()
);

// Drive and intake run at the same time
```

**Important:** Commands in parallel group CANNOT share subsystems. The scheduler checks this and will crash immediately if there's a conflict (prevents dangerous situations).

**Lifecycle:**
1. All commands initialize() at the same time
2. All commands execute() every cycle
3. Each command ends when its isFinished() returns true
4. Group finishes when ALL commands finished

**Use Cases:**
- Drive while intaking
- Multiple independent actions
- Complex autonomous (arm moving while driving)

### Command Group Examples

**Simple Autonomous:**
```cpp
// Drive forward 3 seconds, then turn 90°, then intake for 2 seconds
auto autonomous = std::make_unique<SequentialCommandGroup>(
    new DriveForwardCommand(driveSub, 3.0),
    new TurnCommand(driveSub, 90),
    new IntakeCommand(intakeSub, 2.0)
);

// Schedule in autonomous() function
scheduler.schedule(autonomous.get());
```

**Complex Autonomous:**
```cpp
// Drive forward while intaking, then turn and shoot
auto complex = std::make_unique<SequentialCommandGroup>(
    // Drive and intake simultaneously
    new ParallelCommandGroup(
        new DriveForwardCommand(driveSub, 2.0),
        new IntakeCommand(intakeSub, 2.0)
    ),
    // Then turn
    new TurnCommand(driveSub, 90),
    // Then shoot
    new ShootCommand(shooterSub)
);
```

### andThen() Decorator
Chain commands sequentially (FRC-style).

```cpp
// Instead of creating SequentialCommandGroup
auto seq = cmd1->andThen(cmd2.get())
               ->andThen(cmd3.get())
               ->andThen(cmd4.get());

// Much more readable than:
auto seq = new SequentialCommandGroup(
    cmd1.get(), cmd2.get(), cmd3.get(), cmd4.get()
);
```

---

## Directory Structure

### Project Layout
```
include/custom/
├── command/
│   ├── commandBase.h          # Base class for all commands
│   ├── template_command.h     # Template for new commands (COPY THIS!)
│   ├── driveCommand.h         # Example: Drive command
│   └── [your_command.h]       # Your commands here
│
├── subsystem/
│   ├── subsystemBase.h        # Base class for all subsystems
│   ├── template_subsystem.h   # Template for new subsystems (COPY THIS!)
│   ├── driveSubsystem.h       # Example: Drive subsystem
│   └── [your_subsystem.h]     # Your subsystems here
│
├── controller.h               # Controller bindings
├── scheduler.h                # Command scheduler
├── globals.h                  # Global constants and hardware config
└── print.h                    # Formatted printing utilities

src/
├── main.cpp                   # PROS entry point (opcontrol, autonomous)
├── robot.cpp                  # Robot configuration (subsystems, commands, bindings)
│
└── custom/
    ├── command/
    │   └── [your_command.cpp]     # Complex command implementations (optional)
    │
    ├── subsystem/
    │   └── [your_subsystem.cpp]   # Complex subsystem implementations (optional)
    │
    ├── controller.cpp             # Controller implementation
    ├── scheduler.cpp              # Scheduler implementation
    └── subsystemBase.cpp          # Subsystem base implementation
```

### Adding New Functionality

**Add a Command:**
1. Copy `include/custom/command/template_command.h`
2. Rename file and class (e.g., `IntakeCommand`)
3. Update subsystem pointer type
4. Implement lifecycle methods
5. Register in `robot.cpp`

**Add a Subsystem:**
1. Copy `include/custom/subsystem/template_subsystem.h`
2. Rename file and class (e.g., `IntakeSubsystem`)
3. Add motor/sensor declarations
4. Implement control methods
5. Create instance in `robot.cpp`

---

## Best Practices

### Command Design

#### 1. Require Subsystems When Controlling Physical Components
```cpp
// CORRECT - Controls motors, needs exclusive access
MyCommand(MySubsystem* sub) : m_subsystem(sub) {
    addRequirements(sub);  // Prevents conflicts
}

// ALSO OK - Only reads sensors
SensorMonitorCommand(MySubsystem* sub) : m_subsystem(sub) {
    // No addRequirements() - only reading, not controlling
}
```

**Why:** Scheduler uses requirements to prevent multiple commands from fighting for control. Reading sensors doesn't need exclusive access, controlling motors does.

#### 2. Initialize in initialize(), Not Constructor
```cpp
// CORRECT
void initialize() override {
    m_timer.reset();      // Reset each time command starts
    m_subsystem->reset();
}

// WRONG - Constructor only runs once
MyCommand() {
    m_timer.reset();  // Only happens once, not each schedule!
}
```

**Why:** Commands are reusable. Constructor runs once when created, initialize() runs each time command is scheduled.

#### 3. Clean Up in end() (Best Practice)
```cpp
// BEST PRACTICE - Explicit cleanup
void end(bool interrupted) override {
    m_subsystem->stop();  // Always stop motors
}

// WORKS BUT RISKY - No explicit cleanup
void end(bool interrupted) override {
    // Motors keep state until something else changes them
}
```

**Why:** While not strictly required (next command will control motors), explicit cleanup makes behavior predictable and serves as safety measure.

#### 4. Use isFinished() Appropriately
```cpp
// CORRECT - Command with timeout
bool isFinished() override {
    return m_timer.get_time() > 2000;
}

// CORRECT - whileTrue() command
bool isFinished() override {
    return false;  // Never ends on its own
}

// CORRECT - Default command
bool isFinished() override {
    return false;  // Runs until another command needs subsystem
}
```

**Why:** Commands bound with whileTrue() can return false (cancelled when button released). Commands bound with onTrue() should have finish condition. Default commands should return false.

### Naming Conventions
```cpp
// Classes: PascalCase
class DriveSubsystem {};
class IntakeCommand {};

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

---

## PID Tuning Guidelines
Because many teams rely on LemLib (or similar libraries) for closed‑loop motion, our framework includes basic support for running PID routines in both heading and linear dimensions. Proper tuning is critical were poorly‑tuned gains lead to oscillation, sluggish response, or commands that terminate immediately. The following procedure has worked reliably on our robots:

1. **Start with conservative gains** – set P to 2 and D to 10. Integral (I) is left at 0 unless significant steady‑state error is observed; our drivetrain is effectively two‑dimensional so drift is minimal.
2. **Tune derivative first.**  Increase D gradually while running the loop (either angular or linear). Continue until oscillations around the target disappears.
3. **Tune proportional next.**  Raise P and repeat tuning D until the system becomes more aggressive but still stable. If increasing P induces oscillation that D can no longer dampen, fall back to the previous P/D pair that was stable.

![pid graph](/img/pd_tuning_flowchart.png)

> **Tip:** During initial tuning, run the PID helper outside the scheduler (e.g. directly from `auto()`) to avoid unintended cancellations; once gains are stable you can wrap the call in a command `execute()` loop for integration.

---

## Common Issues & Solutions

### Scheduler Issues

**Issue: Command not starting**
- Check if command added to scheduler: `scheduler.schedule(cmd.get())`
- Verify subsystem requirements set: `addRequirements(subsystem)`
- Check if another command already using subsystem
- Verify scheduler.run() called in loop

**Issue: Commands fighting for subsystem**
- Ensure all commands call `addRequirements(subsystem)` for physical control
- Scheduler should auto-cancel conflicting commands
- Check logs for interruption messages

**Issue: whileTrue() not cancelling on button release**
- Verify whileTrue() bug fix applied (check controller.cpp)
- Ensure m_inRunLoop cleared before polling
- Check button binding in configureBindings()

### Motor Issues

**Issue: Motors running wrong direction**
- Negate motor port in globals.h: `{1, -2, 3}` → `{-1, 2, -3}`
- Check motor gearbox orientation on robot
- Verify left/right motor groups not swapped

**Issue: Robot drives backwards when pushing forward**
- Swap left and right motor groups
- Or invert both groups in globals.h

**Issue: Motors overheating**
- Check for stalled motors (blocked movement)
- Verify motor cartridge (red/green/blue) appropriate for load
- Add cooling delays between runs
- Check for mechanical friction

### Controller Issues

**Issue: Controller prints not showing**
- Add 50ms delay after each controller.print()
- Use `customPrint::controllerPrint()` helper
- Verify controller connected and master/partner correct

**Issue: Buttons not responsive**
- Check joystick deadband setting (5-10 recommended)
- Verify controller.poll() called every loop
- Test with different button (hardware issue?)

### Build Issues

**Issue: "undefined reference" errors**
- Check all .cpp files in src/custom/ directory
- Verify Makefile includes all source files
- Clean and rebuild: `pros clean`, `pros make`

**Issue: Header not found**
- Check #include paths relative to include/ directory
- Verify file exists in correct location
- Check for typos in filename or path

---

## API Quick Reference

### CommandBase Methods
```cpp
// Lifecycle (override these)
void initialize()                    // Called once when scheduled
void execute()                       // Called every cycle
void end(bool interrupted)           // Called when command ends
bool isFinished()                    // Return true to end command
CommandBase* clone() const           // Create copy of command

// Properties
std::string getName() const          // Auto-generated from class name
void addRequirements(SubsystemBase*) // Require subsystem
const std::vector<SubsystemBase*>& getRequiredSubsystems() const

// Decorators
std::unique_ptr<CommandBase> withTimeout(double seconds)
std::unique_ptr<CommandBase> withName(const std::string& name)
std::unique_ptr<CommandBase> andThen(const CommandBase* next)
```

### SubsystemBase Methods
```cpp
// Lifecycle
void periodic()                      // Called every loop automatically

// Properties
std::string getName() const          // Auto-generated from class name
void setDefaultCommand(CommandBase*) // Set default command
CommandBase* getCurrentCommand()     // Get current command

// Factory methods (convenience)
std::unique_ptr<CommandBase> runOnce(std::function<void()>)
std::unique_ptr<CommandBase> run(std::function<void()>)
std::unique_ptr<CommandBase> runUntil(std::function<void()>, std::function<bool()>)
```

### Scheduler Methods
```cpp
static Scheduler& getInstance()      // Get singleton instance
CommandBase* schedule(CommandBase*)  // Schedule command
void cancel(CommandBase*)            // Cancel command
bool isScheduled(CommandBase*)       // Check if scheduled
void run()                           // Run one cycle (call in loops)
```

### Controller Bindings
```cpp
// Get button binder
A()                             // Button A
B()                             // Button B
X()                             // Button X
Y()                             // Button Y
UP(), DOWN(), LEFT(), RIGHT()   // D-Pad Directions
L1(), L2(), R1(), R2()          // Shoulder buttons

// Binding types
onTrue(CommandBase*)                 // Trigger on press
onFalse(CommandBase*)                // Trigger on release
whileTrue(CommandBase*)              // Run while held
```

### Command Groups
```cpp
// Sequential - run commands in order
SequentialCommandGroup(std::vector<CommandBase*>)
SequentialCommandGroup(cmd1, cmd2, cmd3, ...)

// Parallel - run commands simultaneously
ParallelCommandGroup(std::vector<CommandBase*>)
ParallelCommandGroup(cmd1, cmd2, cmd3, ...)
```

### Print Utilities
```cpp
customPrint::printf(const char* fmt, ...)           // Console output
customPrint::screenPrint(int line, const char* fmt, ...) // Brain screen
customPrint::controllerPrint(Controller* ctrl, int line, const char* fmt, ...) // Controller screen (with delay)
```

---

## Getting Started Example

Complete minimal robot setup:

```cpp
// 1. Define hardware in globals.h
namespace globalConst {
    namespace controller {
        constexpr pros::controller_id_e_t kMainControllerID = pros::E_CONTROLLER_MASTER;
    }
    namespace intake {
        constexpr std::initializer_list<int8_t> kMotorPorts = {1};
        constexpr pros::MotorGearset kMotorGearset = pros::MotorGearset::green;
    }
}

// 2. Create subsystem (intakeSubsystem.h)
class IntakeSubsystem : public SubsystemBase {
private:
    pros::MotorGroup m_motor;
public:
    IntakeSubsystem() 
        : m_motor(globalConst::intake::kMotorPorts, 
                  globalConst::intake::kMotorGearset) {}
    void intake() { m_motor.move(127); }
    void stop() { m_motor.move(0); }
};

// 3. Create command (intakeCommand.h)
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
    CommandBase* clone() const override { 
        return new IntakeCommand(m_subsystem); 
    }
};

// 4. Register in robot.cpp
Controller controller(globalConst::controller::kMainControllerID);
std::unique_ptr<IntakeSubsystem> intakeSub;
std::unique_ptr<IntakeCommand> intakeCmd;

void robotInit() {
    intakeSub = std::make_unique<IntakeSubsystem>();
    intakeCmd = std::make_unique<IntakeCommand>(intakeSub.get());
}

void configureBindings() {
    controller.R1().whileTrue(intakeCmd.get());
}

// 5. Run scheduler in main.cpp
void opcontrol() {
    robotInit();
    configureBindings();
    
    while (true) {
        Scheduler::getInstance().run();
        pros::delay(10);
    }
}
```

That's it! R1 now runs the intake while held.

---

## Additional Resources

### Documentation Files
- `FRAMEWORK_GUIDE.md` - Comprehensive framework guide with examples
- `template_command.h` - Command template with detailed comments
- `template_subsystem.h` - Subsystem template with detailed comments
- Engineering notebook (weeks 2-16) - Development history and decisions

### External Resources
- [PROS API Documentation](https://pros.cs.purdue.edu/v5/api/)
- [LemLib Documentation](https://lemlib.readthedocs.io/)
- [FRC WPILib Documentation](https://docs.wpilib.org/) - Original inspiration

### Community Support
- [VEX Forum](https://www.vexforum.com/)
- [PROS Discord](https://discord.gg/pros)
- [LemLib Discord](https://discord.gg/lemlib)

---

