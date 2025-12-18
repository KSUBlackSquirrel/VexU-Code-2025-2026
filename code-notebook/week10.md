10/28/2025
Today's Goals:
    Ian, Antonio, and Philip will be implementing the whileTrue() bug fix and creating comprehensive tests for button bindings.

Today's Tasks:
    After discovering the whileTrue() bug last week during intake/outtake testing, we're implementing the fix and creating test commands to validate all button binding behaviors.

The whileTrue() Bug:

    We discovered a critical bug in whileTrue()! The issue was that during controller polling, `m_inRunLoop` was true, causing schedule() to return the original command pointer instead of the cloned instance. When whileTrue() checked isScheduled(m_runningCommand), it couldn't find the clone in the queue, making it think the command vanished and re-scheduling it repeatedly.

    Additionally, when commands finished naturally (like timed commands), whileTrue() would hold onto a stale pointer, preventing the command from being restarted on the next button press.

Implementing the Button Binding Fix:

    We added the third condition to ButtonBinder::poll() that checks with the scheduler:

```cpp
// In controller.cpp - ButtonBinder::poll()
if (m_edge == Edge::WhileTrue) {
    bool curr = m_controller->get_digital(m_button);
    if (curr && !m_runningCommand) {
        // Button pressed, no command running - schedule it
        m_runningCommand = Controller::m_globalScheduler->schedule(km_command);
    } else if (!curr && m_runningCommand) {
        // Button released - cancel command if still running
        if (Controller::m_globalScheduler->isScheduled(m_runningCommand)) {
            Controller::m_globalScheduler->cancel(m_runningCommand);
        }
        m_runningCommand = nullptr;
    } else if (curr && m_runningCommand) {
        // Button still held - check if command finished naturally
        if (!Controller::m_globalScheduler->isScheduled(m_runningCommand)) {
            m_runningCommand = nullptr;  // Clear stale pointer
        }
    }
}
```

    This third condition (curr && m_runningCommand) handles the case where a command finishes naturally while the button is still held, preventing stale pointers.

The m_inRunLoop Fix:

    Clear `m_inRunLoop` BEFORE polling controllers so buttons get the correct cloned pointer that can be tracked properly:

```cpp
// In scheduler.cpp - step2_pollCommandSchedulingTriggers()
m_inRunLoop = false;  // Clear BEFORE polling
pollControllerBindings();
m_inRunLoop = true;
```

    This prevents duplicate scheduling by ensuring whileTrue() can track the actual cloned command instance that was added to the queue.

Creating Test Commands:

    We created test commands to validate different button binding scenarios:

```cpp
// PulseCommand - Tests natural command finish
class PulseCommand : public CommandBase {
    ExampleSubsystem* m_subsystem;
    uint32_t m_startTime;
    uint32_t m_duration;
    
public:
    PulseCommand(ExampleSubsystem* sub, uint32_t durationMs = 300) 
        : m_subsystem(sub), m_duration(durationMs) {
        addRequirements(m_subsystem);
    }
    
    void initialize() override {
        m_startTime = pros::millis();
        m_subsystem->forward();
    }
    
    bool isFinished() override {
        return (pros::millis() - m_startTime) >= m_duration;
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();
        customPrint::printf("PulseCommand ended - %s\n", 
            interrupted ? "interrupted" : "finished naturally");
    }
};
```

    Test Scenarios:

    1. onTrue() test: Command runs once when button pressed
    2. whileTrue() test: Command runs while held, restarts on repeated presses
    3. onFalse() test: Command runs once when button released
    4. Natural finish test: Command with timeout finishes on its own, can be restarted
    5. Cancel test: Command cancelled by button release works correctly

```cpp
// In robot.cpp - Test bindings
void configureBindings() {
    // Test whileTrue with natural finish
    controller.A().whileTrue(pulseCommand.get());  // 300ms pulse, should restart on repeated press
    
    // Test whileTrue with infinite command
    controller.B().whileTrue(continuousCommand.get());  // Runs until cancelled
    
    // Test onTrue
    controller.X().onTrue(instantCommand.get());  // Runs once per press
}
```

Reflection:
Reflection:
    The whileTrue() fix is working perfectly! We fixed two separate issues: the stale pointer problem (handled by the third condition in poll()) and the duplicate scheduling problem (handled by clearing m_inRunLoop before controller polling). Commands that finish naturally now properly clear their stale pointers, allowing them to be restarted. We tested extensively with both timed commands (PulseCommand) and continuous commands. The binding system is now robust and reliable. This fix was critical because without it, many common command patterns (timed operations, sensor-triggered commands) wouldn't work correctly.
10/29/2025

11/01/2025