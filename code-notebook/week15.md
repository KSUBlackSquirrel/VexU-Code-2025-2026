12/09/2025
Today's Goals:
Ian, Antonio, and Philip will be implementing intake and outtake subsystems for the competition robot.

Today's Tasks:
We are adding real game piece handling mechanisms. Based on our robot's design, we need separate intake and outtake subsystems due to them being physically different mechanisms.

IntakeSubsystem Implementation:

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

OuttakeSubsystem Implementation:

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

Why Separate Subsystems?

Our robot design has two distinct mechanisms:
- Intake: Ground-level rollers to collect game pieces
- Outtake: Elevated mechanism to score game pieces

They operate independently and can run simultaneously, so separate subsystems make sense.

Command Implementation:

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

Controller Bindings:

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

Reflection:
Successfully implemented intake and outtake as separate subsystems! The implementation is quick and straightforward. Each subsystem has basic run() and stop() methods with forward/reverse support. This gets us functional game piece handling on the competition robot right away. The commands use whileTrue() so motors run only while buttons are held. We can refine and optimize the implementation later as needed, but for now this simple approach works well for practice.

**[PHOTO NEEDED: Intake and outtake mechanisms on robot]**
**[PHOTO NEEDED: Controller layout showing R1/R2 for intake, L1/L2 for outtake]**


12/10/2025
Today's Goals:
Ian, Antonio, and Philip will be testing and refining the intake/outtake subsystems in practice.

Today's Tasks:
We are testing the newly implemented intake and outtake subsystems on the actual robot. Making sure the bindings work correctly and motors respond as expected.

Testing Process:

1. Verified motor directions - both intake and outtake spin correct directions
2. Tested reverse functionality - both can run backwards to clear jams
3. Confirmed whileTrue() works correctly after bug fix
4. Checked that motors stop immediately when buttons released

Example Testing Code:

```cpp
// Testing intake direction
controller.R1().whileTrue(intakeForwardCommand.get());  // Collect game pieces
controller.R2().whileTrue(intakeBackwardCommand.get()); // Eject/clear jams

// Testing outtake direction  
controller.L1().whileTrue(outtakeForwardCommand.get());  // Score game pieces
controller.L2().whileTrue(outtakeBackwardCommand.get()); // Reverse to clear
```

Motor Configuration:

```cpp
// In globals.h - Motor speed constants
namespace intake {
    const int kIntakeSpeed = 100;  // Full speed for intake
    const int kOuttakeSpeed = 100; // Full speed for outtake
}
```

Minor Adjustments:

- Tuned motor speeds for optimal game piece handling
- Adjusted brake modes for better control
- Verified no conflicts between intake and outtake (separate subsystems work great)

Reflection:
Intake and outtake subsystems are working perfectly on the robot! The separate subsystem design was the right choice. We can run both simultaneously without conflicts. The simple implementation is proving effective for competition use. Ready for competition practice, and we can add more advanced features (current limiting, sensor feedback, etc.) in future iterations as needed.

**[PHOTO NEEDED: Team testing intake/outtake on practice field]**


