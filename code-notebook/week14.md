12/11/2025
Today's Goals:
Ian, Antonio, and Philip will be testing and refining the intake/outtake subsystems in practice.

Today's Tasks:
We are testing the newly implemented intake and outtake subsystems on the actual robot. Making sure the bindings work correctly and motors respond as expected.

**Testing Process:**

1. Verified motor directions - both intake and outtake spin correct directions
2. Tested reverse functionality - both can run backwards to clear jams
3. Confirmed whileTrue() works correctly after bug fix
4. Checked that motors stop immediately when buttons released

**Example Testing Code:**

```cpp
// Testing intake direction
controller.R1().whileTrue(intakeForwardCommand.get());  // Collect game pieces
controller.R2().whileTrue(intakeBackwardCommand.get()); // Eject/clear jams

// Testing outtake direction  
controller.L1().whileTrue(outtakeForwardCommand.get());  // Score game pieces
controller.L2().whileTrue(outtakeBackwardCommand.get()); // Reverse to clear
```

**Motor Configuration:**

```cpp
// In globals.h - Motor speed constants
namespace intake {
    constexpr int kIntakeSpeed = 100;  // Full speed for intake
    constexpr int kOuttakeSpeed = 100; // Full speed for outtake
}
```

**Minor Adjustments:**

- Tuned motor speeds for optimal game piece handling
- Adjusted brake modes for better control
- Verified no conflicts between intake and outtake (separate subsystems work great)

Reflection:
Intake and outtake subsystems are working perfectly on the robot! The separate subsystem design was the right choice - we can run both simultaneously without conflicts. The whileTrue() bug fix from Dec 9 made everything reliable. Ready for competition practice.

**[PHOTO NEEDED: Team testing intake/outtake on practice field]**


12/13/2025
Today's Goals:
Ian, Antonio, and Philip will be synchronizing the codebase across all three project folders.

Today's Tasks:
We are moving files around and updating all three project codebases (dev-code, template-code, 24in-code) to match. This ensures scheduler, controller, and main files are consistent across projects.

**What We Synchronized:**

- scheduler.cpp and scheduler.h (including whileTrue bug fix)
- controller.cpp and controller.h  
- main.cpp structure
- Command and subsystem base classes

**Key Files Updated Across All Projects:**

```cpp
// scheduler.cpp - whileTrue() bug fix synchronized
void Scheduler::step2_pollCommandSchedulingTriggers() {
    m_inRunLoop = false;  // Critical fix - clear BEFORE polling
    pollControllerBindings();
    m_inRunLoop = true;
}
```

```cpp
// controller.h - Button binding methods
class ControllerButton {
public:
    void whileTrue(CommandBase* command);   // Run while button held
    void onTrue(CommandBase* command);      // Run once on press
    void onFalse(CommandBase* command);     // Run once on release
    void toggleOnTrue(CommandBase* command); // Toggle on/off each press
};
```

**Synchronization Process:**

1. Started with dev-code as the reference (most up-to-date)
2. Copied core framework files to template-code
3. Copied core framework files to 24in-code
4. Verified compilation in all three projects
5. Tested that robot-specific code still works correctly

This is maintenance work but important - keeps all projects up to date with bug fixes and improvements.

Reflection:
Successfully synchronized all three codebases. Now dev-code, template-code, and 24in-code all have the latest framework improvements and bug fixes. This will prevent confusion when switching between projects. Good practice to keep codebases in sync periodically.

**[PHOTO NEEDED: File comparison showing synchronized code]**
