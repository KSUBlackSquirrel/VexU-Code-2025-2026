09/30/2025
Today's Goals:
	Ian, Antonio, and Philip will be implementing proper command interruption based on subsystem requirements. This is a critical safety feature that prevents multiple commands from conflicting when they try to control the same hardware.

Today's Tasks:
	We are solving one of our major architectural challenges: preventing multiple commands from fighting over the same subsystem. Currently, if two commands both try to control the motor subsystem, they'll conflict and cause unpredictable behavior.
	
	**The Problem - Subsystem Conflicts:**
	Imagine this scenario:
	1. User presses UP button → "DriveForward" command starts, motors spin forward
	2. User presses DOWN button → "DriveBackward" command starts
	3. Now BOTH commands are trying to control the motors!
	4. Result: motors receive conflicting instructions every cycle, causing jerky behavior or even might cause damage
	
	We need a system where the scheduler can automatically detect these conflicts and safely stop the old command before starting the new one.
	
	**The Solution - Subsystem Requirements:**
	We implemented an `addRequirements()` system that lets commands declare which subsystems they need exclusive access to.
	
	**Updated CommandBase.h:**
	
```cpp
// commandBase.h - Added subsystem requirements tracking
class CommandBase {
protected:
    std::vector<SubsystemBase*> m_requiredSubsystems;  // Which subsystems this command needs
    
public:
    // Lifecycle methods
    virtual void initialize() {};
    virtual void execute() {};
    virtual void end(bool interrupted) {};  // interrupted parameter tells us WHY we're ending
    virtual bool isFinished() = 0;
    virtual CommandBase* clone() const = 0;
    
    // NEW: Declare which subsystems this command needs
    void addRequirements(SubsystemBase* subsystem) {
        if (subsystem != nullptr) {
            m_requiredSubsystems.push_back(subsystem);
        }
    }
    
    // Get the list of required subsystems
    const std::vector<SubsystemBase*>& getRequiredSubsystems() const {
        return m_requiredSubsystems;
    }
};
```
	
	**Updated Command Implementation:**
	Now every command can declare its requirements in the constructor:
	
```cpp
// up.h - Command that spins motor forward
class Up : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    
public:
    Up(ExampleSubsystem* sub) : m_subsystem(sub) {
        addRequirements(sub);  // Declare: "I need exclusive access to this subsystem"
    }
    
    void execute() override {
        m_subsystem->forward();  // Spin motor forward
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();  // Stop the motor when ending
        
        if (interrupted) {
            // We were stopped because another command needed the subsystem
            pros::screen::print(pros::E_TEXT_MEDIUM, 1, "Up command interrupted");
        }
    }
    
    bool isFinished() override {
        return false;  // Runs until interrupted
    }
    
    CommandBase* clone() const override {
        return new Up(*this);
    }
};
```
	
	**Enhanced Scheduler Logic:**
	We updated the scheduler to check for subsystem conflicts before starting commands:
	
```cpp
// In controller.h (scheduler functionality)
void scheduleCommand(CommandBase* cmd) {
    // Step 1: Check if any required subsystems are already in use
    for (auto* subsystem : cmd->getRequiredSubsystems()) {
        CommandBase* currentCmd = subsystem->getCurrentCommand();
        
        if (currentCmd != nullptr) {
            // Another command is using this subsystem!
            // We need to interrupt it before proceeding
            
            customPrint::printf("Interrupting %s to start %s\n", 
                              currentCmd->getName().c_str(), 
                              cmd->getName().c_str());
            
            currentCmd->end(true);  // true = interrupted (not finished normally)
            removeCommand(currentCmd);  // Remove from active commands list
        }
        
        // Mark this subsystem as now being used by the new command
        subsystem->setCurrentCommand(cmd);
    }
    
    // Step 2: Start the new command
    cmd->initialize();
    m_activeCommands.push_back(cmd);
}
```
	
	**What Happens in Practice:**
	1. User presses UP button → Starts "Up" command requiring ExampleSubsystem
	2. Up runs happily, spinning motor forward
	3. User presses DOWN button → Tries to start "Down" command (also requires ExampleSubsystem)
	4. Scheduler detects conflict: ExampleSubsystem is currently used by Up command
	5. Scheduler calls `Up::end(true)` - motor stops, command cleans up
	6. Scheduler removes Up from active commands
	7. Scheduler starts Down command - motor now spins backward
	8. Transition is smooth and safe!

Reflection:
	We successfully implemented subsystem requirements and command interruption! Testing confirmed that the system correctly detects conflicts and safely transitions between commands. The `interrupted` parameter in `end()` allows commands to behave differently when interrupted versus finishing normally. For example, an autonomous command that's interrupted might log an error, while one that finishes normally logs success. This feature is essential for building reliable robot behaviors and prevents the hardware conflicts that plagued earlier testing.

**[PHOTO NEEDED: Flowchart showing command interrupt sequence: Button B pressed → Scheduler detects conflict → Call Command A end(true) → Remove Command A → Start Command B]**


10/01/2025
Today's Goals:
	Ian, Antonio, and Philip will be testing the interruption system thoroughly and creating test cases to ensure it works reliably in all scenarios.

Today's Tasks:
	We are conducting comprehensive testing of the subsystem requirements system to verify it works correctly under various conditions. To make sure we can verify the test we needed to add some kinda of logging during the scheduler.

    **Debug Logging:**
	We added comprehensive logging to track command lifecycle:
	
```cpp
void scheduleCommand(CommandBase* cmd) {
    customPrint::printf("[SCHED] Scheduling: %s\n", cmd->getName().c_str());
    
    for (auto* subsystem : cmd->getRequiredSubsystems()) {
        CommandBase* currentCmd = subsystem->getCurrentCommand();
        if (currentCmd != nullptr) {
            customPrint::printf("[SCHED] Interrupting: %s\n", currentCmd->getName().c_str());
            currentCmd->end(true);
        }
    }
    
    cmd->initialize();
    customPrint::printf("[SCHED] Initialized: %s\n", cmd->getName().c_str());
}
```
	
	This logging helped us identify and fix several edge cases during testing.
	
	**Test Case 1: Simple Interruption**
	- Start Up command (forward)
	- Start Down command (backward)
	- Expected: Up interrupted cleanly, Down takes over
	- Result: PASS - Motor transitions smoothly, no conflicts
	
	**Test Case 2: Multiple Subsystems**
	We created a command that requires TWO subsystems to test multi-subsystem conflicts:
	
```cpp
class ComplexCommand : public CommandBase {
public:
    ComplexCommand(ExampleSubsystem* sub1, DriveSubsystem* sub2) {
        addRequirements(sub1);  // Needs motor subsystem
        addRequirements(sub2);  // AND drive subsystem
    }
    
    void execute() override {
        // Control both subsystems
    }
};
```
	
	- Start command requiring only subsystem A
	- Start command requiring only subsystem B  
	- Start ComplexCommand requiring both A and B
	- Expected: Both previous commands interrupted
	- Result: PASS - Both interrupted correctly
	
	**Test Case 3: Rapid Button Presses**
	- Rapidly press UP, DOWN, UP, DOWN buttons
	- Expected: Each command interrupts previous, no crashes or memory leaks
	- Result: PASS - System handles rapid changes smoothly
	
	**Test Case 4: Interrupt During Initialize**
	- Command A starts, initialize() called
	- Command B scheduled before A's execute() runs
	- Expected: A's end() still called even though execute() never ran
	- Result: PASS - Lifecycle handled correctly

Reflection:
	We completed thorough testing of the subsystem requirements system and it passed all test cases! The system reliably prevents conflicts, safely interrupts commands, and handles edge cases correctly. The debug logging proved invaluable for understanding exactly what's happening during complex scenarios. We're confident this system is production-ready and will prevent the hardware conflicts that could damage motors or cause unpredictable robot behavior.


10/04/2025
Today's Goals:
	Ian, Antonio, and Philip will be documenting the subsystem requirements system and creating guidelines for writing commands that properly use it.

Today's Tasks:
	We are creating comprehensive documentation and best practices for the subsystem requirements system.
	
	**Command Writing Guidelines:**
	
	**Guideline 1: Declare Requirements for Exclusive Subsystem Access**
	
	If your command needs exclusive control of a subsystem (most commands do), declare it in the constructor:
	
```cpp
// Command that needs exclusive access to drive subsystem
class MyCommand : public CommandBase {
public:
    MyCommand(DriveSubsystem* drive) {
        m_drive = drive;
        addRequirements(drive);  // Prevents other commands from using drive simultaneously
    }
};
```
	
	However, some commands don't need requirements:
	
```cpp
// Command that just waits - no hardware control needed
class WaitCommand : public CommandBase {
public:
    WaitCommand(uint32_t ms) : m_waitTime(ms) {
        // No subsystem requirements - doesn't control any hardware
    }
};

// Command that runs in parallel without conflicts
class LogDataCommand : public CommandBase {
public:
    LogDataCommand() {
        // No requirements - just reads sensors and logs data
    }
};
```
	
	**Guideline 2: Stop Hardware in end() When Needed**
	
	If your command controls motors or actuators, always stop them in end():
	
```cpp
// Command controlling motors - MUST stop in end()
void end(bool interrupted) override {
    m_subsystem->stop();  // Critical - prevents motors from continuing to run
    
    if (interrupted) {
        customPrint::printf("Command interrupted - stopped safely\n");
    }
}
```
	
	But not all commands need to stop hardware:
	
```cpp
// Command that doesn't control motors directly
class SetTargetPosition : public CommandBase {
    void end(bool interrupted) override {
        // No motor control - just sets a target value
        // The PID controller command handles the actual motor stop
    }
};
```
	
	**Guideline 3: Use interrupted Parameter for Different Cleanup**
	
```cpp
void end(bool interrupted) override {
    m_subsystem->stop();  // Stop hardware first (if applicable)
    
    if (interrupted) {
        // Didn't complete - might want to reset state
        customPrint::printf("Command interrupted before completion\n");
        m_subsystem->resetPosition();  // Reset for next attempt
    } else {
        // Finished successfully - record completion
        customPrint::printf("Command completed successfully\n");
        m_subsystem->recordSuccess();
    }
}
```
	
	**Common Patterns:**
	
	**Pattern: Default Command**
	
	Default commands provide default behavior when a subsystem is idle. They MUST declare requirements and are automatically interrupted:
	
```cpp
// Default commands run when subsystem has nothing else to do
// They declare requirements and are interruptible by design
class DefaultDriveCommand : public CommandBase {
public:
    DefaultDriveCommand(DriveSubsystem* drive) {
        m_drive = drive;
        addRequirements(drive);  // Required for default commands
        // Default commands are always interruptible
    }
    
    void execute() override {
        // Joystick control - runs until interrupted by autonomous or other command
        m_drive->tankDrive(controller.getLeftY(), controller.getRightY());
    }
};
```
	
	**Pattern: Parallel Helper Command**
	
	Commands that run alongside others without conflicts don't need requirements:
	
```cpp
// Runs in parallel - no subsystem conflicts
class LEDStatusCommand : public CommandBase {
public:
    LEDStatusCommand() {
        // No requirements - doesn't control any subsystems
        // Can run alongside any other command
    }
    
    void execute() override {
        // Update LEDs based on robot state
        updateLEDs();
    }
};
```
	
	**Pattern: Command Groups**
	
	Commands that compose other commands inherit their requirements automatically:
	
```cpp
// Sequential group - requirements come from composed commands
SequentialCommandGroup autoRoutine({
    new DriveForward(driveSubsystem),   // Requires drive
    new Turn90(driveSubsystem),          // Requires drive
    new RunIntake(intakeSubsystem)      // Requires intake
});
// Group automatically requires both drive AND intake subsystems
```

Reflection:
	We completed comprehensive documentation for the subsystem requirements system. The guidelines and examples will help future team members write commands correctly and avoid common pitfalls. The pattern library covers the most frequent use cases we've encountered. This documentation, combined with our example commands, provides everything needed to use the requirements system effectively. The subsystem conflict resolution system is now fully documented and ready for widespread use throughout the codebase.

**[PHOTO NEEDED: Documentation page showing rules with good/bad code examples side by side]**
**[PHOTO NEEDED: Pattern library reference card for quick lookup]**
