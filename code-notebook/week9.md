10/21/2025
Today's Goals:
Ian, Antonio, and Philip will be reorganizing the codebase to improve structure and fix controller-related bugs.

Today's Tasks:
We are moving core source files into a `custom` directory to organize our framework better. This separates our code from PROS/LemLib code.

**Directory Reorganization:**

We moved all framework files into `custom/` subdirectories:
- src/custom/ for implementation files
- include/custom/ for headers

This makes includes clearer:
```cpp
#include "custom/scheduler.h"   // Our code
#include "pros/motors.hpp"      // PROS library
```

**Controller Bug Fix:**

Fixed a bug where controller had a pointer to the scheduler. The scheduler should poll the controller, not vice versa. This follows better dependency flow and single responsibility principle.

Reflection:
Successfully reorganized the codebase. Code is now better organized with clear separation between our framework and external libraries. The controller/scheduler dependency issue is fixed, improving maintainability.

**[PHOTO NEEDED: Directory structure showing custom/ folder organization]**


10/25/2025
Today's Goals:
Ian, Antonio, and Philip will be fixing button binding behavior for PulseCommand.

Today's Tasks:
We discovered that PulseCommand (300ms timeout test command) wasn't restarting properly when bound to buttons with `whileTrue()`.

**The Problem:**

After PulseCommand finished naturally via isFinished(), pressing the button again didn't restart it. The binding still thought the command was scheduled even though the scheduler had removed it.

**The Fix:**

Updated `whileTrue()` to check with the scheduler if the command is actually still running:
```cpp
if (!m_runningCommand || !scheduler.isScheduled(m_runningCommand)) {
    m_runningCommand = scheduler.schedule(m_command);
}
```

Now bindings verify command state with the scheduler instead of trusting their own tracking.

Reflection:
Fixed the button binding issue! PulseCommand now restarts correctly on repeated button presses. The key insight: bindings must verify command state with scheduler, not just track local pointers. This makes the framework more robust for commands that finish naturally.

**[PHOTO NEEDED: Terminal showing PulseCommand restarting successfully]**


11/01/2025
Today's Goals:
Ian, Antonio, and Philip will be implementing proper cleanup in command end() methods and testing RunUntil patterns.

Today's Tasks:
Ensuring commands properly clean up via end() method. Also testing RunUntil command that runs until a condition becomes true.

**Implementing end() Methods:**

Every command needs cleanup when finishing:
```cpp
void end(bool interrupted) override {
    m_subsystem->setMotorSpeed(0);  // Stop motors
    
    if (interrupted) {
        fmt::print("Command interrupted\n");
    }
}
```

**RunUntil Command Pattern:**

Implemented command that runs until a lambda condition returns true:
```cpp
// Run until sensor threshold
RunUntilCommand(&sub, []() {
    return pros::sensor.get() > threshold;
});

// Run for 2 seconds  
uint32_t start = pros::millis();
RunUntilCommand(&sub, [start]() {
    return (pros::millis() - start) > 2000;
});
```

Reflection:
Successfully implemented proper end() methods across test commands. RunUntil pattern is very useful - lets us create condition-based commands without new classes. The end(bool interrupted) parameter gives us two behaviors: normal completion vs emergency stop.

**[PHOTO NEEDED: ExampleCommand with end() method]**
**[PHOTO NEEDED: Terminal showing "finished normally" vs "interrupted"]**
