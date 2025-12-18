12/16/2025
Today's Goals:
Ian, Antonio, and Philip will be synchronizing framework files across all three project folders after recent improvements.

Today's Tasks:
After implementing intake/outtake subsystems and fixing various bugs, we need to synchronize the core framework files across tundra, active-dev-code, and template-code to ensure all projects have the latest improvements.

What We're Synchronizing:

Framework core files that must be identical across all projects:
- `scheduler.cpp` and `scheduler.h` - includes all recent bug fixes
- `controller.cpp` and `controller.h` - includes whileTrue() fix
- `main.cpp` - standardized structure and initialization
- `commandBase.h` - command interface
- `subsystemBase.h` and `subsystemBase.cpp` - subsystem interface
- `watchdog.h` - safety monitoring
- `print.h` - print utilities

Synchronization Process:

1. Take a reference file (most recently updated code)
2. Copy framework files to all other projects:
   - `src/custom/controller.cpp`
   - `src/custom/scheduler.cpp`
   - `src/custom/subsystemBase.cpp`
   - `include/custom/controller.h`
   - `include/custom/scheduler.h`
   - `include/custom/command/commandBase.h`
   - `include/custom/subsystem/subsystemBase.h`
   - `src/main.cpp`
   
3. Verify each project compiles independently
4. Test that robot-specific code still works

Key Files Updated:


```cpp
// controller.cpp - whileTrue() bug fix now in all projects
void ButtonBinder::poll() {
    if (m_edge == Edge::WhileTrue) {
        bool curr = m_controller->get_digital(m_button);
        if (curr && !m_runningCommand) {
            m_runningCommand = Controller::m_globalScheduler->schedule(km_command);
        } else if (!curr && m_runningCommand) {
            if (Controller::m_globalScheduler->isScheduled(m_runningCommand)) {
                Controller::m_globalScheduler->cancel(m_runningCommand);
            }
            m_runningCommand = nullptr;
        } else if (curr && m_runningCommand) {
            // Check if command finished naturally
            if (!Controller::m_globalScheduler->isScheduled(m_runningCommand)) {
                m_runningCommand = nullptr;
            }
        }
    }
}
```

What Stays Different:

Robot-specific files that remain unique per project:
- `robot.cpp` - each robot has different subsystems and bindings
- `globals.h` - each robot has different motor ports and constants
- Subsystem implementations (drive, intake, outtake, etc.)
- Command implementations specific to each robot

Reflection:
Successfully synchronized all three project codebases! Every project now has the latest framework improvements including the whileTrue() fix, improved initialization, and all bug fixes. This prevents confusion when switching between projects. Good practice to synchronize periodically after major improvements. Now we can develop features in active-dev-code, test them, then copy just the framework changes to tundra and template-code while keeping robot-specific code separate.


12/17/2025
