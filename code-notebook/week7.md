10/14/2025
Today's Goals:
	Ian, Antonio, and Philip will be reimplementing the `whileTrue()` functionality that we removed in Week 3 due to bugs. With our improved scheduler and interruption system, we should be able to make it work reliably now.

Today's Tasks:
	We are implementing the `whileTrue()` binder that runs a command continuously while a button is held or a condition is true. This is different from `onTrue()` which triggers once when the button is first pressed.
	
	Understanding the Difference:
	
	Edge-Triggered (.onTrue() and .onFalse()):
	- Triggers once when state changes
	
```cpp
// Press button → Command starts once
// Hold button → Does nothing
// Release button → Nothing happens (unless you have onFalse binding)
controller.A().onTrue(new Pulse());
```
	
	Level-Triggered (.whileTrue()):
	- Runs while condition is true
	- Like a light switch - on while held, off when released
	
```cpp
// Press button → Command starts
// Hold button → Keeps command running, restarting if applicable
// Release button → Command automatically stops (interrupted)
controller.A().whileTrue(new RunIntake());
```
	
	Why We Need WhileTrue:
	Many robot mechanisms should only run while the operator holds a button:
	- Intake: run while button held, stop when released
	- Flywheel: spin up while button held, stop when released  
	- Manual lift control: move while button held, stop when released
	
	Without `whileTrue()`, we'd need two bindings for everything:
```cpp
// Without whileTrue - tedious and error-prone
controller.A().onTrue(new StartIntake());
controller.A().onFalse(new StopIntake());

// With whileTrue - clean and obvious
controller.A().whileTrue(new RunIntake());
```
	
	Implementation in ButtonBinder:
	
```cpp
// In controller.h
class ButtonBinder {
private:
    enum class Edge { None, Rising, Falling, WhileTrue };
    
    Controller* m_controller;
    pros::controller_digital_e_t m_button;
    const CommandBase* km_command;
    Edge m_edge;
    CommandBase* m_runningCommand = nullptr;  // Track command instance
    bool m_previousState = false;             // Track previous button state
    
public:
    ButtonBinder& whileTrue(const CommandBase* cmd) {
        km_command = cmd;
        m_edge = Edge::WhileTrue;
        m_controller->registerBinder(this);
        return *this;
    }
    
    void poll() {
        bool currentState = m_controller->get_digital(m_button);
        
        if (m_edge == Edge::WhileTrue) {
            if (currentState && !m_runningCommand) {
                // Button pressed and command not running → start it
                m_runningCommand = Controller::m_globalScheduler->schedule(km_command);
            } else if (!currentState && m_runningCommand) {
                // Button released and command is running → stop it
                Controller::m_globalScheduler->cancel(m_runningCommand);
                m_runningCommand = nullptr;
            }
        } else if (m_edge == Edge::Rising) {
            // Edge-triggered - only on press
            if (currentState && !m_previousState) {
                Controller::m_globalScheduler->schedule(km_command);
            }
        } else if (m_edge == Edge::Falling) {
            // Edge-triggered - only on release
            if (!currentState && m_previousState) {
                Controller::m_globalScheduler->schedule(km_command);
            }
        }
        
        m_previousState = currentState;
    }
};
```
	
	Example Command for WhileTrue:
	
```cpp
// moveWithoutLimit.h - Runs continuously while button held
class MoveWithoutLimit : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    
public:
    MoveWithoutLimit(ExampleSubsystem* sub) : m_subsystem(sub) {
        addRequirements(sub);
    }
    
    void execute() override {
        m_subsystem->forward();  // Runs every cycle while button held
    }
    
    bool isFinished() override {
        return false;  // Never finishes on its own
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();  // CRITICAL: Stop motor when button released
        
        if (interrupted) {
            customPrint::printf("MoveWithoutLimit interrupted (button released)\n");
        }
    }
    
    CommandBase* clone() const override {
        return new MoveWithoutLimit(*this);
    }
};
```

Reflection:
	We successfully implemented `whileTrue()` functionality! The key improvements from our first attempt in Week 3 are: (1) Proper use of the `interrupted()` lifecycle method for cleanup, (2) Improved scheduler logic for command cancellation, and (3) Better state tracking in the ButtonBinder to detect when buttons are released. Testing confirmed that commands start smoothly when buttons are pressed and stop immediately when released. The `end(interrupted=true)` is reliably called for cleanup. This functionality will be essential for implementing intuitive operator controls.

**[PHOTO NEEDED: Timing diagram showing whileTrue command lifecycle: button press → initialize → execute loop → button release → end(interrupted=true)]**


10/15/2025
Today's Goals:
	Ian, Antonio, and Philip will be thoroughly testing the whileTrue implementation and creating examples demonstrating different use cases.

Today's Tasks:
	We are creating comprehensive tests and examples for the whileTrue functionality to ensure it works reliably in various scenarios.
	
	Test Case 1: Basic Hold-to-Run
```cpp
// Simple motor control - forward while button held
controller.UP().whileTrue(exampleCmdForward.get());

// Test results:
// - Press and hold UP → Motor spins forward
// - Release UP → Motor stops immediately
// - PASS
```
	
	Test Case 2: Multiple WhileTrue Bindings
```cpp
// Two different buttons controlling the same subsystem
controller.UP().whileTrue(exampleCmdForward.get());
controller.DOWN().whileTrue(exampleCmdBackward.get());

// Test results:
// - Hold UP → Forward
// - Release UP, press DOWN → Backward (smooth transition)
// - Press DOWN while holding UP → Forward interrupted, Backward starts
// - PASS - Subsystem conflict resolution works correctly
```
	
	Test Case 3: WhileTrue with Joystick
```cpp
// Run command while joystick exceeds threshold
controller.RightJoyY(20).whileTrue(driveCmd.get());

// Test results:
// - Push joystick forward → Drive command runs
// - Hold joystick → Command continues
// - Release joystick (return to neutral) → Command stops
// - PASS
```
	
	Test Case 4: Mixing Edge and Level Triggers
```cpp
// Combination of trigger types
controller.A().onTrue(startFlywheel.get());    // Press to start
controller.B().whileTrue(runIntake.get());     // Hold to run
controller.A().onFalse(stopFlywheel.get());   // Release to stop

// Test results:
// - Can use different trigger types simultaneously
// - No interference between bindings
// - ✅ PASS
```
	
	Edge Cases and Fixes:
	We discovered and fixed several edge cases:
	
	Edge Case 1: Rapid Press/Release
	- Problem: Button pressed and released within one loop cycle
	- Fix: ButtonBinder now tracks state changes correctly
	- Result: Command properly starts and stops even with rapid inputs
	
	Edge Case 2: Button Held During Mode Change
	- Problem: Button held during disabled → enabled transition
	- Fix: Scheduler clears all commands on mode change
	- Result: Command doesn't automatically start when enabled
	
	Edge Case 3: Command Scheduled While WhileTrue Active
	- Problem: Another command tries to use the same subsystem
	- Fix: Subsystem requirements system properly interrupts whileTrue command
	- Result: Conflict resolved correctly

Reflection:
	We completed comprehensive testing of whileTrue functionality and it passed all test cases, including several edge cases we discovered during testing. The practical intake control example demonstrates how this feature makes robot code more intuitive and maintainable. Instead of needing separate start/stop commands for every mechanism, we can use single whileTrue bindings that handle both starting and stopping automatically. This will significantly simplify our competition code and reduce the chance of motors being accidentally left running.


10/18/2025
Today's Goals:
	Ian, Antonio, and Philip will be documenting the whileTrue functionality and adding it to our framework examples.

Today's Tasks:
	We are creating comprehensive documentation and examples for the whileTrue feature.
	
	Documentation in controller.h:
	
```cpp
/**
 * @brief Run command continuously while button is held
 * 
 * The command starts when the button is first pressed and runs continuously
 * until the button is released. When released, the command is interrupted
 * (end(true) is called) and removed from the scheduler.
 * 
 * This is ideal for mechanisms that should only run while actively controlled:
 * - Intakes (run while button held)
 * - Manual lift control (move while button held)
 * - Variable speed controls (adjust while joystick moved)
 * 
 * @param button Which controller button to monitor
 * @param cmd Command to run while button is held
 * @return Reference to this binder for method chaining
 * 
 * @example
 * // Run intake while R1 is held, stop when released
 * controller.R1().whileTrue(
 *     new RunIntake(intakeSubsystem.get())
 * );
 */
ButtonBinder& whileTrue(pros::controller_digital_e_t button, const CommandBase* cmd);
```
	
	Updated Example Commands:
	We updated `exampleCommand.h` to include whileTrue examples:
	
```cpp
// Example showing whileTrue pattern for continuous control
class ContinuousControl : public CommandBase {
    // This command is designed to be used with whileTrue()
    // It runs continuously while a button is held
    
    void execute() override {
        // Perform action every loop
        m_subsystem->doAction();
    }
    
    bool isFinished() override {
        // Never finishes on its own - runs until interrupted
        return false;
    }
    
    void end(bool interrupted) override {
        // IMPORTANT: Always stop hardware in end()
        m_subsystem->stop();
        
        // interrupted will be true when button is released
        if (interrupted) {
            customPrint::printf("Button released - stopped action\n");
        }
    }
};
```
	
	Comparison Guide:
	We created a reference showing when to use each binding type:
	
	Use .onTrue() when:
	- Command should start once when button pressed
	- Command runs to completion on its own (has a finish condition)
	- Examples: Start autonomous routine, toggle pneumatic, fire shooter
	
	Use .onFalse() when:
	- Command should start when button is released
	- Less common, but useful for "release to fire" patterns
	- Examples: Catapult (charge while held, fire on release)
	
	Use .whileTrue() when:
	- Command should run ONLY while button is held
	- Command stops immediately when button released
	- Examples: Manual control, intakes, variable mechanisms
	
	Quick Reference Table:
```
| Binding Type | Triggers When          | Stops When             | Use For                |
|--------------|------------------------|------------------------|------------------------|
| .onTrue()    | Button pressed         | Command finishes       | One-shot actions       |
| .onFalse()   | Button released        | Command finishes       | Release-to-fire        |
| .whileTrue() | Button pressed         | Button released        | Hold-to-run controls   |
```

Reflection:
	We completed full documentation for the whileTrue feature with examples and guidelines for when to use each binding type. The quick reference table will be especially helpful for team members learning the framework. The feature is now production-ready with comprehensive documentation, tested edge cases, and clear usage examples. This completes the implementation of all three main binding types (onTrue, onFalse, whileTrue), giving us flexible control over when and how commands execute based on controller input.

