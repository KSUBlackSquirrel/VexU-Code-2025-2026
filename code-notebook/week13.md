12/02/2025
Today's Goals:
Ian, Antonio, and Philip will be implementing sequential and parallel command groups.

Today's Tasks:
We are adding command groups that allow composing multiple commands together. This was committed on Dec 3 (commit e32beef).

**SequentialCommandGroup:**

Runs commands one after another:
```cpp
// Run command1, then when it finishes, run command2
SequentialCommandGroup group({&cmd1, &cmd2, &cmd3});
```

Use cases:
- Autonomous routines (drive forward, then turn, then intake)
- Multi-step mechanisms (extend arm, then close claw, then retract)

**ParallelCommandGroup:**

Runs multiple commands at the same time:
```cpp
// Run drive and intake simultaneously
ParallelCommandGroup group({&driveCmd, &intakeCmd});
```

Important: Commands in parallel group cannot use the same subsystem (checked at runtime to prevent conflicts).

Reflection:
Successfully implemented command groups! SequentialCommandGroup lets us chain commands for autonomous, and ParallelCommandGroup lets us run multiple mechanisms simultaneously. The subsystem conflict checking in parallel groups prevents bugs. This is a major feature that makes complex robot behaviors much easier to program.

**[PHOTO NEEDED: Diagram showing sequential vs parallel command execution]**


12/04/2025
Today's Goals:
Ian, Antonio, and Philip will be improving command utilities and testing the andThen() decorator.

Today's Tasks:
We added the andThen() method that makes chaining commands easier:

```cpp
// Instead of creating a SequentialCommandGroup manually:
SequentialCommandGroup group({&cmd1, &cmd2});

// Use andThen() for cleaner code:
cmd1.andThen(&cmd2);
```

Also updated print utilities to clear lines before printing, preventing text overlap on the controller screen.

Reflection:
The andThen() decorator makes command composition much cleaner. Print utilities are improved - controller display is now more reliable. These small improvements add up to better code quality and easier development.

**[PHOTO NEEDED: andThen() usage example in robot.cpp]**


12/06/2025
Today's Goals:
Ian, Antonio, and Philip will be preparing to split the codebase into separate projects.

Today's Tasks:
We are organizing files to split into separate project directories:
- dev-code: Active development robot code
- template-code: Clean template for new projects
- 24in-code: Competition robot (24" size class)

This organization lets us:
- Keep a clean template without experimental changes
- Have separate codebases for different robots
- Maintain stable code for competition

Reflection:
Prepared for codebase split. Having separate projects will improve organization and let us experiment in dev-code without risking the competition code. This is good engineering practice for a team with multiple robots.

**[PHOTO NEEDED: Folder structure showing three project directories]**


12/09/2025
Today's Goals:
Ian, Antonio, and Philip will be implementing intake and outtake subsystems for the competition robot.

Today's Tasks:
We are adding real game piece handling mechanisms. Based on our robot's design, we need SEPARATE intake and outtake subsystems - they're physically different mechanisms.

**IntakeSubsystem Implementation:**

```cpp
// intakeSubsystem.h - Controls game piece collection
class IntakeSubsystem : public SubsystemBase {
private:
    pros::MotorGroup m_intakeMotor;
    
public:
    IntakeSubsystem() :
        m_intakeMotor(globalConst::intake::kIntakeMotorsID, 
                      globalConst::intake::kIntakeMotorColor) {
        m_intakeMotor.set_brake_mode_all(globalConst::drive::kBreakMode);
    }
    
    void run(bool reverse=false) {
        int speed = reverse ? -100 : 100;
        m_intakeMotor.move(MotorTools::percentToVelocity(speed, kIntakeMotorColor));
    }
    
    void stop() {
        m_intakeMotor.brake();
    }
};
```

**OuttakeSubsystem Implementation:**

```cpp
// outtakeSubsystem.h - Controls game piece scoring
class OuttakeSubsystem : public SubsystemBase {
private:
    pros::MotorGroup m_outtakeMotor;
    
public:
    OuttakeSubsystem() :
        m_outtakeMotor(globalConst::intake::kOuttakeMotorsID,
                       globalConst::intake::kOuttakeMotorColor) {
        m_outtakeMotor.set_brake_mode_all(globalConst::drive::kBreakMode);
    }
    
    void run(bool reverse=false) {
        int speed = reverse ? -100 : 100;
        m_outtakeMotor.move(MotorTools::percentToVelocity(speed, kOuttakeMotorColor));
    }
    
    void stop() {
        m_outtakeMotor.brake();
    }
};
```

**Why Separate Subsystems?**

Our robot design has two distinct mechanisms:
- **Intake**: Ground-level rollers to collect game pieces
- **Outtake**: Elevated mechanism to score game pieces

They operate independently and can run simultaneously, so separate subsystems make sense.

**Command Implementation:**

```cpp
// IntakeCommand - Simple hold-to-run command
class IntakeCommand : public CommandBase {
private:
    IntakeSubsystem* m_subsystem;
    bool m_reverse;
    
public:
    IntakeCommand(IntakeSubsystem* sub, bool reverse=false) {
        m_subsystem = sub;
        m_reverse = reverse;
        addRequirements(m_subsystem);
    }
    
    void execute() override {
        m_subsystem->run(m_reverse);
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();  // Always stop when button released
    }
    
    bool isFinished() override {
        return false;  // Run until interrupted
    }
};
```

**Controller Bindings:**

```cpp
// In robot.cpp
std::unique_ptr<IntakeSubsystem> intakeSub;
std::unique_ptr<OuttakeSubsystem> outtakeSub;

std::unique_ptr<IntakeCommand> intakeForwardCommand;
std::unique_ptr<IntakeCommand> intakeBackwardCommand;
std::unique_ptr<OuttakeCommand> outtakeForwardCommand;
std::unique_ptr<OuttakeCommand> outtakeBackwardCommand;

void configureBindings() {
    // R1: Intake forward (collect)
    controller.R1().whileTrue(intakeForwardCommand.get());
    // R2: Intake backward (reject)
    controller.R2().whileTrue(intakeBackwardCommand.get());
    
    // L1: Outtake forward (score)
    controller.L1().whileTrue(outtakeForwardCommand.get());
    // L2: Outtake backward (clear jam)  
    controller.L2().whileTrue(outtakeBackwordCommand.get());
}
```

**whileTrue() Bug Fix:**

We discovered and fixed a critical bug in whileTrue()! The issue was that during controller polling, `m_inRunLoop` was true, causing schedule() to return the original command pointer instead of the cloned instance. When whileTrue() checked isScheduled(m_runningCommand), it couldn't find the clone in the queue, making it think the command vanished and re-scheduling it repeatedly.

**The Fix:**

Clear `m_inRunLoop` BEFORE polling controllers so buttons get the correct cloned pointer that can be tracked properly:

```cpp
// In scheduler.cpp - step2_pollCommandSchedulingTriggers()
m_inRunLoop = false;  // Clear BEFORE polling
pollControllerBindings();
m_inRunLoop = true;
```

This prevents duplicate scheduling and makes whileTrue() work reliably.

Reflection:
Successfully implemented intake and outtake as separate subsystems! Each has its own motors and controls, matching our robot's physical design. Commands use whileTrue() so motors run only while buttons are held - safe and intuitive. We also found and fixed a nasty bug in whileTrue() that was causing duplicate command scheduling. The fix was subtle but crucial - clearing m_inRunLoop before controller polling ensures bindings track the correct command instances. Both subsystems are working reliably on the competition robot!

**[PHOTO NEEDED: Intake and outtake mechanisms on robot]**
**[PHOTO NEEDED: Controller layout showing R1/R2 for intake, L1/L2 for outtake]**
