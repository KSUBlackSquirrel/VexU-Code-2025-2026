## Technical Reference

**Framework Components:**

| Component | Purpose | Key Files | Lines of Code |
|-----------|---------|-----------|---------------|
| Scheduler | Manages command execution and lifecycle | `scheduler.h/cpp` | ~300 |
| CommandBase | Interface all commands implement | `commandBase.h` | ~50 |
| SubsystemBase | Interface all subsystems implement | `subsystemBase.h/cpp` | ~80 |
| Controller | Button/joystick input handling | `controller.h/cpp` | ~200 |
| ButtonBinder | Maps buttons to commands | `controller.cpp` | ~60 |
| JoystickBinder | Maps joystick movements to commands | `controller.cpp` | ~70 |

**Command Lifecycle:**
1. **Schedule** - Command added to scheduler via button press or autonomous routine
2. **Initialize** - `initialize()` called once when command starts
3. **Execute Code** - `execute()` called every ~20ms while active
4. **Finish Check** - `isFinished()` checked each cycle
5. **End** - `end()` called when command completes normally
6. **Interrupted** - `interrupted()` called if command cancelled by subsystem conflict

**Scheduler Execution Steps (Every Rotbot Loop):**
1. **Subsystem Periodic** - Update sensor data, check limits
2. **Poll Triggers** - Check button states, schedule new commands
3. **Execute Commands** - Run active command logic, finish completed commands
4. **Default Commands** - Start defaults for idle subsystems

**Dependencies:**
- **PROS 3.x** - VEX V5 firmware and HAL
- **LemLib 0.5.6** - Motion control, odometry, PID control
- **LVGL 9.2.0** - Graphics and GUI
- **fmt** - Modern C++ string formatting

**Repository Structure:**
```
VexU-Code-2025-2026/
├── Comp2024/              # Competition robot code
## Week 12
### Controller ↔ Scheduler: global auto-registration (actual behavior)

This week we fixed a subtle but important coupling issue between `Controller` and `Scheduler`. The implementation in the repo uses a global auto-registration model so controllers and binders work correctly regardless of construction order.

**Key implementation details (from `src/custom/controller.cpp` and `src/custom/scheduler.cpp`):**

- A single, shared scheduler pointer exists: `static Scheduler* Controller::m_globalScheduler = nullptr;`.
- Controllers created before a scheduler is available are stored in `Controller::m_pendingControllers`.
- `Controller::Controller(pros::controller_id_e_t id)` registers the controller with `m_globalScheduler` if it's already set; otherwise it pushes the controller onto `m_pendingControllers`.
- `Controller::setScheduler(Scheduler* sch)` sets `m_globalScheduler` and calls `registerPendingControllers()` to register any controllers created earlier.

**How binders schedule commands (short):**

- Binders (`ButtonBinder` / `JoystickBinder`) are created via `controller.A()`, `controller.LEFT()`, etc., and store the command and edge type.
- During `Controller::poll()`, each binder's `poll()` queries the controller inputs and—if an event is detected—uses `Controller::m_globalScheduler` to `schedule()` or `cancel()` the command, but only if the global scheduler is set.

Example excerpt (behavioral):
```cpp
// In ButtonBinder::poll()
if (m_edge == Edge::Rising) {
    bool pressed = m_controller->get_digital_new_press(m_button);
    if (pressed && Controller::m_globalScheduler && km_command) {
        Controller::m_globalScheduler->schedule(km_command);
    }
}
```

This means the framework supports a simple global registration model: controllers and binders operate independently of scheduler construction order, and the scheduler is "injected" globally via `Controller::setScheduler(...)`.

**Recommended usage patterns (seen in the repo):**

1) Explicit global registration (recommended for clarity):

```cpp
Scheduler scheduler;                             // create scheduler
Controller::setScheduler(&scheduler);            // set global scheduler
Controller controller(E_CONTROLLER_MASTER);     // controller will auto-register

// Bind normally (binder uses Controller::m_globalScheduler internally)
controller.A().onTrue(new IntakeCommand());
```

2) Use the Scheduler singleton (already present in `src/custom/scheduler.cpp`):

```cpp
Controller::setScheduler(&Scheduler::getInstance()); // enable auto-registration to singleton
Controller controller(E_CONTROLLER_MASTER);
controller.A().onTrue(new IntakeCommand());
```

Notes:
- If you create `Controller` instances before calling `Controller::setScheduler(...)`, the constructor pushes them onto `m_pendingControllers`. Calling `setScheduler()` later registers those pending controllers with the scheduler (see `registerPendingControllers()`).
- Binders call into `Controller::m_globalScheduler` when `poll()` detects events; scheduling only happens if the global scheduler pointer is non-null.
- There is no per-binder `Scheduler*` stored in the current implementation; the code relies on the global/static scheduler pointer for scheduling.

This design keeps controller creation simple for the rest of the code while ensuring that bindings work regardless of initialization order. It also provides a single clear place to set or swap the scheduler at startup.

---

## Week 13
**Before:**
```cpp
controller.setButtonCommand(pros::E_CONTROLLER_DIGITAL_LEFT, new Down(motorSub));
```

**After:**
```cpp
controller.setButtonCommand().onTrue(pros::E_CONTROLLER_DIGITAL_LEFT, new Down(motorSub));
```

**What's the difference?**
The new syntax reads more like English: "set up a button command that, when true (pressed), does this action." The old syntax was more ambiguous - it wasn't clear if the command would run when the button was pressed, released, or held.

Method chaining works by having each method return an object that you can call another method on. It's like giving instructions: "Go to the store" → "Buy milk" → "Come home." Each step leads to the next step.

The new syntax mirrors FRC's WPILib API, making the intent clearer. We're creating a binder (something that connects a button to an action), then specifying what happens on a rising edge (when the button goes from unpressed to pressed, we call this `onTrue`) or falling edge (when it goes from pressed to unpressed, which we call `onFalse`).

Now we can write:
```cpp
// Run command when button pressed
controller.setButtonCommand().onTrue(BUTTON_A, new IntakeCommand());

// Run command when button released
controller.setButtonCommand().onFalse(BUTTON_A, new StopIntakeCommand());
```

This makes it immediately obvious when each command will run.

**Command Interruption System:**
We implemented proper command interruption based on subsystem requirements. This is a critical safety and functionality feature that prevents multiple commands from fighting over the same hardware.

**The Problem:**
Imagine two commands both trying to control the drive motors at the same time:
- Command A wants to drive forward at 50% speed
- Command B wants to turn in place at 100% speed

Without a system to handle this, both commands would be telling the motors to do different things every robot cycle, causing erratic, unpredictable behavior. The robot might jerk around, oscillate, or even damage hardware from conflicting signals.

**The Solution - Subsystem Requirements:**
We implemented an `addRequirements()` system that lets commands declare which subsystems they need exclusive access to. Think of it like checking out a library book - only one person can have it at a time.

When a command declares which subsystems it needs, the scheduler can:
1. Check if any currently running command is using those subsystems
2. If so, call the `interrupted()` function on the old command to safely shut it down
3. Remove the old command from the scheduler
4. Add the new command, giving it exclusive access to the subsystem

**Implementation:**
We added a new lifecycle method to CommandBase called `interrupted()`:

```cpp
class Down : public CommandBase {
public:
    Down(MotorSubsystem* sub) {
        subsystem = sub;
        addRequirements(subsystem);  // Declare we need exclusive motor access
    }
    
    void execute() override {
        subsystem->spinDown();  // Spin motor backwards
    }
    
    void interrupted() override {
        subsystem->stop();  // Safety: stop motor if interrupted
    }
    
    void end() override {
        subsystem->stop();  // Stop motor when command finishes normally
    }
};
```

**What happens in practice:**
1. User presses Button A - starts "DriveForward" command which requires the DriveSubsystem
2. DriveForward runs happily, moving the robot forward
3. User presses Button B - tries to start "TurnInPlace" command which also requires the DriveSubsystem
4. Scheduler sees the conflict and calls DriveForward's `interrupted()` method
5. DriveForward's `interrupted()` stops the drive motors (safety!)
6. Scheduler removes DriveForward from the active commands list
7. Scheduler adds TurnInPlace, which now has exclusive access to the DriveSubsystem
8. Robot smoothly transitions from driving forward to turning in place

**The difference between `end()` and `interrupted()`:**
- `end()` is called when the command finishes on its own (like when `isFinished()` returns true)
- `interrupted()` is called when the command is forcibly stopped because something else needs its subsystem

Often they do the same thing (stop the motors), but having them separate allows for more sophisticated behavior. For example, `interrupted()` might record that it was interrupted for logging purposes, while `end()` might record successful completion.

**Files Changed:** 10 files total
- `controller.h` - Complete API redesign + scheduler conflict handling
- `commandBase.h` - Added `interrupted()` method and requirements tracking
- All command headers (`up.h`, `down.h`, `pulse.h`, etc.) - Updated with `interrupted()` and requirements
- `main.h` - Added missing includes

**[PHOTO NEEDED: Side-by-side code comparison showing old vs new API]**
**[PHOTO NEEDED: Flowchart showing command interrupt sequence: Button B pressed → Scheduler detects conflict → Call Command A interrupted() → Remove Command A → Start Command B]**

---

## Week 5
### Bug Fixes and WhileTrue Implementation

**Commits: "issue was a memory buffer" and "whileTrue without bugs" (July 18)**

This week we debugged a controller screen issue and successfully implemented the `whileTrue()` functionality - a feature we had attempted earlier but removed due to bugs.

**Controller Screen Buffer Fix:**
We encountered a frustrating problem where the controller screen would occasionally fail to display all outputs. For example, we'd try to print both "Seconds: 5" on line 0 and "Pulse: 3" on line 1, but only the first line would show up. The second print would just disappear.

After extensive debugging and reaching out to the LemLib community discord, a team member from team 781x (named andrew) helped us understand what was happening. They explained that the PROS controller has a message queue system - think of it like a mailbox that can only hold one letter at a time. Messages to update the controller screen are sent every 50 milliseconds. If you try to send a new message while one is already waiting to be delivered (queued), the new message gets thrown away and never reaches the controller.

**The Fix:**
We needed to add delays between consecutive print calls to give the message queue time to clear:

```cpp
bool result1 = controller->print(0, 0, "Seconds: %d", count);
pros::delay(50);  // Allow print buffer time to clear - wait 50ms
bool result2 = controller->print(1, 0, "Pulse: %d", countPulse);
pros::delay(50);  // Wait again before any other prints
```

By waiting 50 milliseconds between prints, we ensure the first message has been sent before we try to send the second one. It's like waiting for someone to finish reading your first letter before handing them the second one.

**WhileTrue Implementation:**
The `whileTrue()` feature allows a command to run continuously as long as a button is held or a condition is met. This is essential for controls like "hold button A to run intake" or "hold the right joystick forward to drive."

**What makes it different from onTrue?**
- **`onTrue()`**: Triggers once when button is first pressed (edge-triggered). Like a doorbell - it dings once when you press it.
- **`whileTrue()`**: Runs continuously while button is held (level-triggered). Like a light switch - the light stays on as long as the switch is held in the "on" position.

Think about an intake motor:
- With `onTrue()`: Press button A → intake spins. Release button A → intake keeps spinning (not what we want!)
- With `whileTrue()`: Press button A → intake spins. Hold button A → intake keeps spinning. Release button A → intake stops (exactly what we want!)

**Example Command:**
```cpp
class MoveWithoutLimit : public CommandBase {
    void execute() override {
        subsystem->move();  // Runs every cycle (50 times per second) while button held
    }
    
    bool isFinished() override {
        return false;  // Never finishes on its own - only when interrupted
    }
    
    void interrupted() override {
        subsystem->stop();  // Critical: stop the motor when button released
    }
};
```

**Why It Works Now (The First Attempt Failed):**
In Week 3, we tried to implement `whileTrue()` but it caused timing bugs and conflicts with the scheduler. Here's what we fixed:

1. **Proper use of `interrupted()` lifecycle method for cleanup:** 
   - Before: When the button was released, the command would sometimes keep running because we didn't have a proper cleanup method
   - After: The `interrupted()` method reliably stops the motor when the button is released

2. **Improved Scheduler logic for command cancellation on button release:**
   - The Scheduler now properly detects when a button is released and immediately calls `interrupted()` on the command
   - This happens within one robot cycle (20ms), so the response feels instant

3. **Separation of edge-triggered and level-triggered bindings:**
   - Edge-triggered: `.onTrue()` and `.onFalse()` - respond to button state changes
   - Level-triggered: `whileTrue()` - respond to button being held
   - By keeping these separate, we avoid confusion about when commands should start and stop

**How the ButtonBinder tracks this:**
```cpp
class ButtonBinder {
    bool previousState = false;
    
    void poll() {
        bool currentState = controller->get_digital(button);
        
        if (currentState && whileTrueCommand) {
            // Button is currently held - keep running the command
            if (!scheduler->isScheduled(whileTrueCommand)) {
                scheduler->scheduleCommand(whileTrueCommand);
            }
        } else if (!currentState && previousState && whileTrueCommand) {
            // Button just released - cancel the command
            scheduler->cancelCommand(whileTrueCommand);
        }
        
        previousState = currentState;
    }
};
```

This code runs every loop:
1. Check if button is currently pressed
2. If yes and command isn't running → start it
3. If no but button WAS pressed last loop → stop the command (it's just been released)
4. Remember current state for next loop

**Files Changed:** 11 files
- `controllerScreenSubsystem.h` - Buffer timing fixes with delays
- `myTime.h` - Changed `addRequirements()` to `addUsedSubsystem()` for better naming
- `controller.h` - Added `whileTrue()` method and continuous command tracking
- `moveWithoutLimit.h` - New command template demonstrating whileTrue pattern
- Various other files updated to support the new functionality

**Custom Print Utility System:**

While debugging the controller screen issues, we realized we needed better control over printing to different outputs. We created a custom print utility (`print.h`) that provides consistent, reliable printing throughout the framework.

**Why We Needed This:**

The standard C++ `printf()` and PROS `pros::screen::print()` functions work, but they have some quirks:
- Different syntax for console vs screen printing
- No centralized place to add debugging features (timestamps, filtering, etc.)
- Harder to temporarily disable all debug prints in competition

We created a unified printing interface:

```cpp
namespace customPrint {
    // Print to console (serial / RTT) - for debugging on computer
    void printf(const char* fmt, ...) {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        ::printf("%s", buf);
    }

    // Print to the V5 screen at a specific line - for debugging on robot
    void screenPrint(int line, const char* fmt, ...) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        pros::screen::print(pros::E_TEXT_MEDIUM, line, "%s", buf);
    }
}
```

**What's happening here:**

`va_list` and variadic functions (`...`) allow functions to accept any number of arguments, just like `printf()`. The process:
1. `va_start(args, fmt)` - Start reading the variable arguments after `fmt`
2. `vsnprintf(buf, sizeof(buf), fmt, args)` - Format the string safely into a buffer
3. `va_end(args)` - Clean up the argument list
4. Call the actual print function with the formatted buffer

**Benefits:**

1. **Consistent API:** All our printing goes through one place with the same syntax
2. **Buffer safety:** We use `vsnprintf()` which prevents buffer overflows (trying to write more characters than the buffer can hold)
3. **Easy to extend:** Want to add timestamps? Filter by severity? Log to SD card? Just modify these two functions
4. **Easy to disable:** In competition, we can comment out the print bodies to eliminate all debug output without changing dozens of files

**Usage throughout the codebase:**

```cpp
// In scheduler.cpp - logging command lifecycle
customPrint::printf("[SCHEDULER] Command initialized: %s\n", cmd->getName().c_str());

// In main.cpp - debugging loop information
customPrint::printf("[%s] Commands in scheduler: %zu\n", functionName.c_str(), commandCount);
customPrint::screenPrint(8, "[%s] Commands: %zu", functionName.c_str(), commandCount);

// In robot.cpp - showing robot state
customPrint::screenPrint(8, "interrupted type: %s", interrupted ? "true" : "false");
```

**Future Expansion Possibilities:**

With this foundation, we could easily add:
- Log levels (DEBUG, INFO, WARNING, ERROR)
- Filtering by subsystem or component
- Writing to SD card for post-match analysis
- Network streaming to driver station
- Toggle debug prints with a controller button combo

This might seem like a small addition, but having good debugging tools is critical for diagnosing issues quickly, especially during competition when time is limited.

**[PHOTO NEEDED: Timing diagram showing whileTrue command lifecycle: button press → initialize → execute loop (repeating) → button release → interrupted → end]**
**[PHOTO NEEDED: Side-by-side comparison showing controller screen before fix (one line) vs after fix (two lines)]**

---

## Week 6
### Project Organization and Code Cleanup

**Commits: "organization" (September 3) and "Update main.h" (September 10)**

This week we reorganized the repository to support multiple robot projects and cleaned up our code structure for better maintainability.

**Repository Restructuring:**

We created separate folders for different purposes:
```
VexU-Code-2024-2025/
├── Comp2024/
│   └── 24in Auto tests/     # Competition-ready code - DO NOT BREAK!
├── Test2025/
│   └── command_test/         # Original framework development
└── Offseason/
    └── command_test/         # Offseason testing and experiments
```

**Why This Matters:**

Because we work on many bots each year having different workspaces help keep thing clean:
- **Comp2024** is where the competition robot code lives. This code has been tested and works reliably. We NEVER experiment here or make risky changes right before a competition.
- **Test2025** is where we developed the framework. This is where the original experimentation happened.
- **Offseason** is where we can try new ideas, break things, and learn without any risk to competition code.

This separation provides several benefits:
- **Safety:** We can't accidentally break competition code while experimenting with new features
- **Flexibility:** Different robots can have different subsystems while sharing the same core framework
- **Comparison:** We can look at how different robots implement the same concept
- **Learning:** New team members can experiment in the Offseason folder without fear of breaking anything important

For example, if we want to try a new autonomous routine, we develop it in the Offseason folder first. Once it's proven to work, we can carefully copy it to the Comp2024 folder.

**Code Cleanup:**

We reorganized the `#include` statements in `main.h` for better readability and maintainability. In C++, `#include` statements bring in code from other files. The order matters because sometimes one file depends on another.

**Before (messy):**
```cpp
#include "custom/command/up.h"
#include "lemlib/api.hpp"
#include "custom/globals.h"
#include "pros/colors.hpp"
#include "custom/command/down.h"
// ... files in random order
```

**After (organized):**
```cpp
// External libraries (from PROS and LemLib)
#include "lemlib/api.hpp"
#include "pros/colors.hpp"

// Framework core (our base classes)
#include "custom/globals.h"
#include "custom/subsystem/subsystemBase.h"
#include "custom/command/commandBase.h"

// Command implementations (specific commands we've written)
#include "custom/command/instantCommand.h"
#include "custom/command/up.h"
#include "custom/command/down.h"
#include "custom/command/pulse.h"
```

**Why Include Order Matters:**

Imagine building a house. You need to:
1. Pour the foundation (external libraries like PROS)
2. Build the frame (our base classes)
3. Add the rooms (specific implementations)

You can't build the rooms before you have a frame, and you can't build the frame before you have a foundation. Similarly, our command implementations depend on `commandBase.h`, which depends on PROS libraries.

Proper include organization:
- **Prevents circular dependencies:** File A includes File B includes File A = infinite loop! By ordering includes carefully, we avoid this.
- **Makes compilation faster:** The compiler can process files in the right order without having to backtrack.
- **Helps other programmers:** Anyone looking at `main.h` can immediately see the project structure - what are the external dependencies, what are the core components, and what are the specific implementations.
- **Makes errors clearer:** If there's a compilation error, having organized includes makes it easier to find where the problem is.

**Additional Benefits:**

By grouping related includes together with comments, we create a mental map of the project:
- "Oh, we're using LemLib for motion control"
- "Our framework core has globals, subsystems, and commands"
- "We have several different command types implemented"

This is especially helpful when onboarding new team members - they can look at `main.h` and understand the project's structure in 30 seconds.

**Files Changed:** Over 300 files for reorganization + include cleanup

The 300+ files changed because we copied the entire framework (including all libraries) into each folder. This seems like a lot, but it's mostly just copying - we only actually modified a handful of files like `main.h` and the Makefile.

**[PHOTO NEEDED: Repository folder structure diagram showing the three separate folders with their purposes labeled]**
**[PHOTO NEEDED: Screenshot of well-organized main.h with sections labeled and color-coded]**

---

## Week 7
### Drive System Development

**Commits: "fall back" and "this works so far..." (September 17)**

This week we transitioned from building the framework itself to using it to control actual robot hardware. We developed the drive subsystem and drive command, proving that our framework could handle real-world robot control.

**New Files Created:**

**driveSubsystem.h** - Encapsulates all drivetrain hardware and LemLib integration
This file represents everything related to the robot's drivetrain - the wheels, motors, sensors, and all the logic for moving the robot around. By putting all this in one place, any command that needs to control the drive just interacts with this subsystem.

**driveCommand.h** - Tank drive command for teleop control
This command reads the controller's joysticks and translates them into motor movements. During the teleop period (when drivers control the robot), this command runs continuously as the drive subsystem's default command.

**Globals reorganization** - Moved `globals.h` to `custom/globals.h` and fixed namespace typo (`gobalDrive` → `globalDrive`)
The globals file contains constants used throughout the project, like motor port numbers, wheel diameters, and gear ratios. We found and fixed a typo where we had misspelled "global" as "gobal" - these kinds of typos can be frustrating to debug, which is why having good organization and code review is important!

**DriveSubsystem Features:**

```cpp
class DriveSubsystem : public SubsystemBase {
    // Manages motor groups for left and right drive
    pros::MotorGroup leftMotors;
    pros::MotorGroup rightMotors;
    
    // Integrates with LemLib chassis controller
    lemlib::Chassis chassis;
    
    // Provides methods that commands can call:
    void tankDrive(int left, int right);    // Left stick controls left side, right stick controls right side
    void arcadeDrive(int forward, int turn); // One stick for forward/back, one for turning
    void stop();                             // Stop all motors
    void periodic();                         // Update odometry and sensors
};
```

**What are motor groups?**
A motor group is a collection of motors that should all spin together. For example, if your left side of the drivetrain has 3 motors, you'd put them in a `leftMotors` group. Then when you tell the group to spin at 50% speed, all three motors automatically spin at 50% speed. This is much easier than controlling each motor individually.

**Integration with LemLib:**

LemLib is an external library that provides advanced motion control features. Think of it as a "smart driving assistant" for your robot. Our drive subsystem wraps (encapsulates) LemLib's chassis controller, which gives us:

- **Odometry tracking:** The robot knows where it is on the field. Using wheel encoders and potentially an inertial sensor, it can track "I've moved 2.5 feet forward and 1 foot to the left, so I'm at position (2.5, 1.0)."

- **Motion profiling for smooth movements:** Instead of instantly jumping to full speed (which can cause wheel slippage and tip-overs), motion profiling gradually accelerates and decelerates. It's like how a car smoothly speeds up rather than jerking forward.

- **PID-tuned drive curves:** PID (Proportional-Integral-Derivative) is a control algorithm that helps motors reach the desired speed smoothly and accurately. These values need to be calibration per robot as any hardware could change how the whole bot handles. Because we don't have a full robot for testing we kept them defualt.

- **Autonomous path following support:** In autonomous mode, the robot can follow pre-programmed paths (like "drive to coordinate (3,4), then turn 90 degrees, then drive to (5,6)"). LemLib handles all the math for this.

**DriveCommand Implementation:**

```cpp
class DriveCommand : public CommandBase {
    void execute() override {
        // Read joystick values (-127 to +127)
        int leftY = controller->get_analog(ANALOG_LEFT_Y);
        int rightY = controller->get_analog(ANALOG_RIGHT_Y);
        
        // Send to drive subsystem
        driveSubsystem->tankDrive(leftY, rightY);
    }
    
    bool isFinished() override {
        return false;  // Runs continuously as default command
    }
};
```

This command is beautifully simple because all the complexity is hidden in the subsystem. The command just:
1. Reads the joysticks
2. Passes those values to the drive subsystem
3. Never finishes (returns false) so it keeps running

**Why does it run continuously?**
This command is set as the drive subsystem's "default command." Remember from Week 2: when a subsystem has no other command using it, the Scheduler automatically runs its default command. This means whenever the driver isn't running a special command (like an automated turn), the robot responds to their joystick inputs.

**Testing Results:**

We tested the drive system and verified:
- ✅ Robot responds to joystick input smoothly and predictably
- ✅ Drive command properly registered as default (robot always responds to joysticks)
- ✅ No subsystem conflicts (commands properly request exclusive access)
- ✅ Smooth motor control through framework (no jerking or stuttering)

**Files Changed:** 5 files
- Created `custom/subsystem/driveSubsystem.h` - Full drive subsystem implementation
- Created `custom/command/driveCommand.h` - Teleop drive command
- Updated `globals.h` - Fixed namespace and added drive constants
- Updated `main.h` and `main.cpp` - Added includes and instantiation

**Motor Utility Functions:**

While working with the drive motors, we encountered a common problem: translating user-friendly percentages (0-100%) into motor velocities that PROS understands. Different motor cartridges (red, green, blue) have different maximum RPMs, so "50% speed" means different things depending on the cartridge.

We added a utility function to `globals.h` to handle this conversion:

```cpp
namespace globalConst {
namespace MotorTools {
    inline int percentToVelocity(int percent, pros::MotorGears color) {
        int maxRpm = 0;
        switch (color) {
            case pros::MotorGearset::red:   maxRpm = 100; break;
            case pros::MotorGearset::green: maxRpm = 200; break;
            case pros::MotorGearset::blue:  maxRpm = 600; break;
            default: maxRpm = 0; break;
        }
        return static_cast<int>(percent * maxRpm / 100.0);
    }
}
}
```

**Understanding Motor Cartridges:**

VEX V5 motors come with three different gear cartridges that trade off speed for torque:

- **Red (100 RPM):** High torque, low speed. Use for lifts, claws, heavy mechanisms
- **Green (200 RPM):** Balanced. Common choice for drivetrains
- **Blue (600 RPM):** High speed, low torque. Use for intakes, flywheels

**Why This Function Matters:**

Without this utility, every time we want to set a motor speed, we'd have to write:
```cpp
// Without utility - repetitive and error-prone
if (motorCartridge == red) {
    motor.move_velocity(percent * 100 / 100);
} else if (motorCartridge == green) {
    motor.move_velocity(percent * 200 / 100);
} else if (motorCartridge == blue) {
    motor.move_velocity(percent * 600 / 100);
}
```

With the utility:
```cpp
// With utility - clean and reusable
motor.move_velocity(MotorTools::percentToVelocity(percent, motorCartridge));
```

**Practical Example:**

```cpp
// I want my green-cartridge drive motors to run at 75% speed
int velocity = MotorTools::percentToVelocity(75, pros::MotorGearset::green);
// Result: 75 * 200 / 100 = 150 RPM

leftMotors.move_velocity(velocity);   // All left motors: 150 RPM
rightMotors.move_velocity(velocity);  // All right motors: 150 RPM
```

**Why use `static_cast<int>`?**

The division `percent * maxRpm / 100.0` produces a floating-point number (like 150.0). PROS motor functions expect integers, so we explicitly convert (cast) the result to an integer (whole number). This prevents compiler warnings and makes our intent clear.

**Design Pattern: Utility Namespaces**

We organized this inside nested namespaces:
- `globalConst` - contains all global constants and utilities
- `MotorTools` - specifically for motor-related helper functions

This keeps the global namespace clean and makes it obvious where to find motor utilities: `globalConst::MotorTools::percentToVelocity()`. Later, we might add more tools like `MotorTools::rpmToVelocity()` or `MotorTools::getCurrentDraw()`.

This pattern of creating small, reusable utility functions is crucial for reducing code duplication and making the codebase easier to maintain. Instead of copying the percentage-to-RPM conversion logic into every command that needs it, we write it once and use it everywhere.

**[PHOTO NEEDED: Diagram showing DriveSubsystem → LemLib → Motor Groups architecture with arrows showing data flow]**
**[PHOTO NEEDED: Video stills showing robot responding to controller input - perhaps a sequence showing: stopped, turning left, driving forward, turning right]**

---

## Week 8
### Framework Documentation and Code Organization

**Commit: "buildable drive test" (September 17)**

A major refactoring week focused on improving code maintainability and preparing for team-wide adoption. "Refactoring" means reorganizing code to make it cleaner and more maintainable without changing what it actually does.

**1. Separated Implementation from Interface:**

In C++, we typically split code into two types of files:
- **Header files (.h):** Define the interface - what methods exist and what they do
- **Implementation files (.cpp):** Contain the actual code that makes those methods work

We created new `.cpp` files to move implementation details out of header files:
- **`controller.cpp`** - Moved all controller binding logic implementation
- **`scheduler.cpp`** - Extracted scheduler implementation  
- **`subsystemBase.cpp`** - Base subsystem functionality implementation

**Before (everything in .h files):**
```cpp
// controller.h
class Controller {
    void poll() {
        // 50 lines of implementation code here...
        // This makes the header file long and hard to read
    }
};
```

**After (separate .h and .cpp):**
```cpp
// controller.h
class Controller {
    void poll();  // Just the declaration - tells you it exists
};

// controller.cpp
void Controller::poll() {
    // 50 lines of implementation code here
    // Now the header is clean and easy to read
}
```

**Benefits of this separation:**

- **Faster compilation:** When you change implementation in a .cpp file, only that file needs to recompile. Before, changing anything meant recompiling every file that included the header. On large projects, this can save minutes or even hours of compile time per day.

- **Cleaner header files:** Headers now read like documentation - you can quickly see what methods are available without wading through implementation details. It's like reading a table of contents versus reading the entire book.

- **Better IDE performance:** IDEs (like VS Code) can parse smaller header files faster, making autocomplete and code navigation snappier.

- **Reduced circular dependency risks:** When implementation is in headers, it's easy to accidentally create circular dependencies (File A includes File B which includes File A). Separate implementation files make this nearly impossible.

**2. Split Controller Responsibilities:**

Previously, `controller.h` was trying to do two completely different jobs:
1. Managing button bindings (connecting buttons to commands)
2. Scheduling commands (deciding when to run them)

This is not ideal because adding too much functionality in one place can lead to whats called a "God class." We split these responsibilities:

**Before:** One file doing everything
```cpp
// controller.h (trying to do too much)
class Controller {
    void poll();                    // Check buttons
    void scheduleCommand();         // Run commands
    void cancelCommand();           // Stop commands
    void runSubsystemPeriodic();   // Update subsystems
    // ... 500 lines of code doing many different things
};
```

**After:** Two focused files, each with a single responsibility
```cpp
// controller.h - ONLY handles button/joystick input
class Controller {
    void poll();  // Check buttons and joysticks
    // ... button binding methods
};

// scheduler.h - ONLY handles command execution
class Scheduler {
    void run();              // Main scheduling loop
    void scheduleCommand();  // Add command to run
    void cancelCommand();    // Remove command
    // ... command management methods
};
```

**Why This Matters - The Single Responsibility Principle:**

Each class should have one reason to change. If controller hardware changes (like switching from V5 controller to a different input device), we only modify `controller.h`. If we want to change how commands are scheduled (like adding priority levels), we only modify `scheduler.h`. They're independent.

This also makes testing easier. We can test the scheduler's command management without needing a real controller, and we can test button detection without needing the whole scheduler running.

It better matches FRC WPILib architecture too, making it easier for team members with FRC experience to understand our code.

**3. Added Code Documentation:**

We added comprehensive comments to all public interfaces explaining what each method does and when to use it:

```cpp
// CommandBase.h
// Abstract base class for all commands in the scheduler system.
// Commands encapsulate actions and can require or use subsystems.
//
// Example usage:
//   class DriveForward : public CommandBase {
//       void execute() override { drive->forward(); }
//   };

class CommandBase {
    // Lifecycle methods called by scheduler:
    
    virtual void initialize() {}  
    // Called once when command starts
    // Use this to: record starting positions, reset timers, prepare hardware
    
    virtual void execute() {}     
    // Called repeatedly while active (~50 times per second)
    // Use this to: perform the main action of the command
    
    virtual bool isFinished() { return false; }  
    // Return true to end command
    // Use this to: check if goal reached, timeout expired, sensor detected something
    
    virtual void end() {}         
    // Called once when command finishes normally
    // Use this to: stop motors, log completion, clean up resources
    
    virtual void interrupted() {} 
    // Called once if command is interrupted
    // Use this to: safely shut down, stop motors, log that we were interrupted
};
```

**Why documentation matters:**

Imagine joining the team next year. Without comments, you'd have to read hundreds of lines of code to figure out when `initialize()` gets called and what you're supposed to do in it. With comments, you know immediately: "Oh, it's called once at the start, and I should use it to set things up."

Good documentation also helps your future self. You might write a command today and think "I'll obviously remember how this works," but come back in 3 months and have no idea what you were thinking!

**4. Fixed Critical Build Issue:**

We discovered a Makefile configuration issue that was preventing the project from building correctly.

**The Problem:**
The Makefile had this setting:
```makefile
# Set to 1 to enable hot/cold linking
USE_PACKAGE:=1
```

Hot/cold linking is a PROS feature that speeds up downloads to the robot by only uploading code that changed. However, it was causing linker errors with our framework - the linker couldn't find some of our functions because they were split across .cpp files in a way that confused the hot/cold system.

**The Solution:**
```makefile
# Set to 1 to enable hot/cold linking
USE_PACKAGE:=0  # Changed from 1 to 0 - disable hot/cold linking
```

Disabling hot/cold linking means uploads take a few seconds longer (we upload everything every time), but the project builds successfully. This is a worthwhile trade-off, especially during development when reliability matters more than upload speed.

**Files Changed:** 13 files
- Created 3 new `.cpp` implementation files (controller, scheduler, subsystemBase)
- Split `controller.h` into `controller.h` and `scheduler.h`
- Added documentation comments to all public interfaces
- Fixed `Makefile` configuration

**Impact:**

This refactoring made the framework more professional and maintainable:
- New team members can understand the code faster
- Changes are less likely to break things
- Compilation is faster
- The code is ready for open-source release

**[PHOTO NEEDED: Before/after file structure diagram showing one big controller.h splitting into controller.h, controller.cpp, scheduler.h, scheduler.cpp]**
**[PHOTO NEEDED: Screenshot of documented code with comments highlighted, showing how comments explain each method]**

---

## Week 9
### Example Framework Creation

**Commit: "renamed root folder" (September 20)**

This week we restructured the repository to create a clean example project that other teams (including future versions of ourselves) could use as a starting point for their own robots.

**Why Create Examples?**

Imagine trying to learn a new language from just a dictionary. You'd know what words mean, but not how to use them in sentences. Examples are like having a phrasebook, they show you how to actually use the framework in practice.

We renamed the project from `command_test` (which sounds like an experiment) to `example-code` (which clearly indicates it's meant to be learned from) and added extensive template files.

**New Structure:**
```
VexU-Code-2025-2026/
├── Offseason/
│   └── example-code/         # ← Renamed from command_test
│       ├── include/custom/
│       │   ├── command/
│       │   │   ├── commandBase.h         # Framework base
│       │   │   ├── driveCommand.h        # Real drive control
│       │   │   └── exampleCommand.h      # ← New: teaching templates
│       │   ├── subsystem/
│       │   │   ├── subsystemBase.h       # Framework base
│       │   │   ├── driveSubsystem.h      # Real drive subsystem
│       │   │   └── exampleSubsystem.h    # ← New: teaching templates
│       │   ├── controller.h
│       │   ├── scheduler.h
│       │   └── globals.h
│       └── src/custom/         # Implementation files
```

**New Example Templates:**

**exampleCommand.h** - Shows common command patterns with detailed comments:

This file contains multiple example commands, each demonstrating a different common pattern:

**1. Up/Down Commands (Simple Motor Control):**
```cpp
class Up : public CommandBase {
    // Example: Run a motor in one direction
    // Pattern: Continuous action until interrupted
    
    void execute() override {
        subsystem->spinUp();  // Spin motor forward
    }
    
    bool isFinished() override {
        return false;  // Never finishes - runs until interrupted
    }
    
    void interrupted() override {
        subsystem->stop();  // Always stop motor when command ends!
    }
};
```

This demonstrates the simplest command pattern: do one thing continuously. It's perfect for things like "run intake while button held" or "spin flywheel at constant speed."

**2. Pulse Command (Timed Action):**
```cpp
class Pulse : public CommandBase {
    // Example: Do something for a specific amount of time
    // Pattern: Action with timeout
    
    uint32_t startTime;
    uint32_t duration = 300;  // milliseconds
    
    void initialize() override {
        startTime = pros::millis();  // Record when we started
        subsystem->spinUp();
    }
    
    bool isFinished() override {
        return (pros::millis() - startTime) >= duration;  // Done after 300ms
    }
    
    void end() override {
        subsystem->stop();  // Stop when time's up
    }
};
```

This shows how to use timers for timed actions. Notice how `initialize()` records the start time, and `isFinished()` checks if enough time has elapsed. This pattern is useful for things like "intake for 0.5 seconds" or "reverse outtake for 0.2 seconds."

**exampleSubsystem.h** - Shows subsystem best practices:

```cpp
class ExampleSubsystem : public SubsystemBase {
    // Hardware (private - commands shouldn't directly access this)
private:
    pros::Motor motor;
    pros::ADIDigitalIn limitSwitch;
    
    // Public methods commands can call
public:
    ExampleSubsystem() {
        // Initialize hardware in constructor
        motor = pros::Motor(1);  // Motor on port 1
        limitSwitch = pros::ADIDigitalIn('A');  // Limit switch on port A
    }
    
    void periodic() override {
        // Update sensors every loop
        // Check for dangerous conditions
        if (motor.get_temperature() > 55) {
            motor.move(0);  // Overheat protection
        }
    }
    
    // Simple methods for commands to use
    void spinUp() { motor.move(127); }      // Full speed forward
    void spinDown() { motor.move(-127); }   // Full speed reverse
    void stop() { motor.move(0); }          // Stop
    
    // Sensor access
    bool isAtLimit() { return limitSwitch.get_value(); }
};
```

This template shows several important subsystem patterns:

1. **Hardware Encapsulation:** All hardware objects (motors, sensors) are private. Commands can't directly access them - they must use the public methods. This prevents accidental misuse.

2. **Periodic Updates:** The `periodic()` method runs every scheduler loop, checking sensors and implementing safety features (like overheat protection).

3. **Simple Interface:** Commands call simple methods like `spinUp()` rather than needing to know motor port numbers or speed values.

4. **Sensor Access Methods:** Instead of giving commands direct sensor access, we provide methods like `isAtLimit()` that return meaningful answers.

**Purpose - Teaching Through Examples:**

By providing these clear, documented examples, we enable:

1. **Learning by reading:** New programmers can read `exampleCommand.h` and understand "Oh, so a timed command works by recording start time in initialize() and checking elapsed time in isFinished()."

2. **Copy and modify:** Instead of writing a new command from scratch, they can copy the Pulse template and change the duration and action. This reduces errors and saves time.

3. **Best practices:** The examples demonstrate proper patterns like always stopping motors in `interrupted()`, using `periodic()` for safety checks, and encapsulating hardware in subsystems.

4. **Quick start:** A new team member can look at these examples and have a working command on the robot within 30 minutes, even if they've never used the framework before.

**Real-World Impact:**

When we bring in new programmers at the start of the season, we can point them to `example-code` and say: "Read through these files, see how commands work, then try modifying the Pulse command to run for 500ms instead of 300ms." Within an hour, they're productively contributing.

**Robot Lifecycle Architecture:**

As the framework matured, we needed a better way to organize robot code across different competition modes (autonomous, teleop, disabled). We created `robot.cpp` to separate robot-specific logic from framework infrastructure.

**The Problem We Solved:**

Previously, all robot setup, command bindings, and mode-specific code lived in `main.cpp`. This mixed two concerns:
1. **Framework infrastructure:** The run loops, scheduler integration, timing enforcement
2. **Robot-specific behavior:** Which commands to run, button bindings, autonomous routines

This made it hard to:
- Understand what code runs in which competition mode
- Reuse the framework infrastructure for different robots
- Onboard new team members ("where do I add my button binding?")

**The Solution - Separation of Concerns:**

```
main.cpp  → Framework infrastructure (run loops, timing, mode switching)
robot.cpp → Robot-specific behavior (bindings, commands, subsystems)
```

**robot.cpp Structure:**

```cpp
// ========== SUBSYSTEM INSTANTIATION ==========
// All subsystems are created here with smart pointers
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();

// ========== COMMAND CREATION ==========
// Commands are created using various patterns:

// 1. Subsystem factory methods (inline lambdas functions to create a command without a dedicated file)
std::unique_ptr<CommandBase> run = exampleSub->run([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> runOnce = exampleSub->runOnce([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> runUntil = exampleSub->runUntil(
    []{ exampleSub->forward(); }, 
    []{ return exampleSub->getPosition() > 300; }
); 
std::unique_ptr<CommandBase> runFor = exampleSub->runFor([]{ exampleSub->forward(); }, 2.0);

// 2. FunctionalCommand (build commands using lambdas without making a dedicated file)
std::unique_ptr<CommandBase> functionalCommand = std::make_unique<FunctionalCommand>(
    [](){ exampleSub->forward(); },     // initialize
    [](){ /* execute logic */ },         // execute  
    [](bool interrupted){ exampleSub->stop(); }, // end
    [](){ return false; },               // isFinished
    std::initializer_list<SubsystemBase*>{exampleSub.get()},
    "Functional"
);

// 3. Dedicated command classes
std::unique_ptr<CommandBase> pulseCommand = std::make_unique<Pulse>(exampleSub.get());


// ========== LIFECYCLE FUNCTIONS ==========

void configureBindings() {
    // Bind commands to controller buttons
    controller.X().onTrue(pulseCommand.get());
    controller.UP().onTrue(run.get());
    controller.RIGHT().onTrue(runOnce.get());
}

void robotInit() {
    // Runs once when robot powers on
    // Set default commands here
}

void robotDisabled() {
    // Runs repeatedly while robot is disabled
    customPrint::screenPrint(1, "--------DISABLED--------");
}

void robotCompInit() {
    // Runs before competition starts (field management connected)
    // Show autonomous selector, etc.
}

void robotAuto() {
    // Runs repeatedly during autonomous period
    // Schedule autonomous command groups here
}

void robotTeleop() {
    // Runs repeatedly during driver control period
    // Display debug info, monitor systems, etc.
}
```

**Understanding Subsystem Factory Methods:**

Factory methods are functions that create and return new command objects. SubsystemBase provides several that make creating simple commands extremely easy:

```cpp
// run() - Creates a command that runs an action repeatedly
auto cmd = subsystem->run([]{ subsystem->doSomething(); });
// Equivalent to writing a full command class with execute() calling doSomething()

// runOnce() - Creates a command that runs an action once then finishes
auto cmd = subsystem->runOnce([]{ subsystem->reset(); });
// Perfect for "press button to reset" type actions

// runUntil() - Runs action until condition becomes true
auto cmd = subsystem->runUntil(
    []{ subsystem->spinMotor(); },              // Action
    []{ return subsystem->getPosition() > 500; } // Stop condition
);

// runFor() - Runs action for specified time (seconds)
auto cmd = subsystem->runFor([]{ subsystem->spinMotor(); }, 2.5);  // 2.5 seconds
```

**Why This Pattern Matters:**

The factory methods let us create commands without writing full command classes for simple actions. It also lets the programmer more options on how they want to write the code. Compare:

**Without factories (verbose):**
```cpp
class SpinForwardCommand : public CommandBase {
public:
    SpinForwardCommand(Subsystem* sub) : subsystem(sub) {
        addRequirements(sub);
    }
    void execute() override { subsystem->forward(); }
    bool isFinished() override { return false; }
    void end(bool interrupted) override { subsystem->stop(); }
    CommandBase* clone() const override { return new SpinForwardCommand(*this); }
private:
    Subsystem* subsystem;
};

auto cmd = std::make_unique<SpinForwardCommand>(subsystem.get());
```

**With factories (concise):**
```cpp
auto cmd = subsystem->run([]{ subsystem->forward(); });
```

Both do the same thing, but the factory version is one line instead of fifteen!

**FunctionalCommand Pattern:**

You can also write more complex inline commands using, `FunctionalCommand`. This lets you build commands from lambda functions:

```cpp
auto customCommand = std::make_unique<FunctionalCommand>(
    // initialize - runs once at start
    [](){ 
        startTime = pros::millis();
        subsystem->prepare();
    },
    
    // execute - runs every loop
    [](){ 
        double power = calculatePower();  // Your custom logic
        subsystem->setPower(power);
    },
    
    // end - runs once when finished
    [](bool interrupted){ 
        subsystem->stop();
        if (interrupted) logWarning("Command cancelled!");
    },
    
    // isFinished - checked every loop
    [](){ 
        return pros::millis() - startTime > 3000;  // 3 second timeout
    },
    
    // requirements - which subsystems this command needs
    std::initializer_list<SubsystemBase*>{subsystem.get()},
    
    // name - for debugging
    "MyCustomCommand"
);
```

This provides the full flexibility of a command class without needing to create a separate `.h` file.

**Benefits of robot.cpp Architecture:**

1. **Clear organization:** Anyone can open robot.cpp and immediately see all subsystems, commands, and bindings
2. **Framework reusability:** main.cpp can stay identical across different robots
3. **Easy onboarding:** "Want to add a button? Go to configureBindings() in robot.cpp"
4. **Mode-specific logic:** Clear separation between auto/teleop/disabled behaviors
5. **Testing friendly:** Can easily swap out robot.cpp for test fixtures

**Smart Pointer Ownership:**

Notice we use `std::unique_ptr<>` for commands and subsystems:

```cpp
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();
```

A `unique_ptr` is a smart pointer that automatically deletes the object when the pointer goes out of scope. Benefits:
- **No memory leaks:** We don't have to remember to `delete` anything
- **Clear ownership:** There's exactly one owner of each subsystem/command
- **Automatic cleanup:** When the program ends, everything is cleaned up automatically

**Why store commands even if they're not scheduled yet?**

```cpp
std::unique_ptr<CommandBase> pulseCommand = std::make_unique<Pulse>(exampleSub.get());
```

We create the command and hold onto it because:
1. The Scheduler take the original command and **clones** it when scheduling (it makes a copy)
2. The original must exist for the lifetime of the program so it can be reused
3. Button bindings store raw pointers to these commands (`pulseCommand.get()`)
4. If the unique_ptr was destroyed, the button binding would point to deleted memory (crash!)

**[PHOTO NEEDED: File explorer view showing organized example-code structure with annotations pointing out "Base classes", "Real implementations", and "Teaching examples"]**
**[PHOTO NEEDED: Screenshot of exampleCommand.h with comments visible, showing how each pattern is explained]**
**[PHOTO NEEDED: Diagram showing the flow: robot.cpp creates subsystems/commands → configureBindings() connects buttons → main.cpp calls lifecycle functions → Scheduler runs everything]**

---

## Week 10
### Scheduler Refinement and Development Planning

**Commits: "added TODO file" and scheduler refactoring (September 29)**

This week focused on two things: breaking the scheduler into more manageable pieces, and creating a roadmap for future development.

**Scheduler Modularization:**

The scheduler's `run()` method was becoming complicated - it was doing many different things all mixed together. We broke it into four distinct steps that happen in sequence. Think of it like organizing a morning routine: instead of "get ready for school" being one giant task, we break it into "shower," "get dressed," "eat breakfast," and "pack bag."

**The 4-Step Scheduler Process:**

```cpp
void Scheduler::run() {
    step1_runSubsystemPeriodicMethods();
    step2_pollCommandSchedulingTriggers();
    step3_runAndFinishScheduledCommands();
    step4_scheduleDefaultCommands();
}
```

This gets called every robot loop (every 20 milliseconds), and each step happens in order:

**Step 1: Run Subsystem Periodic Methods**
```cpp
void Scheduler::step1_runSubsystemPeriodicMethods() {
    for (auto* subsystem : registeredSubsystems) {
        subsystem->periodic();  // Update sensors, check limits, etc.
    }
}
```

**What's happening:** Every subsystem gets a chance to update itself before any commands run.

**Why this matters:** Imagine a lift subsystem that needs to check its limit switches. If we don't update sensors first, a command might make decisions based on old sensor data from 20ms ago. In a fast-moving robot, that's enough time to crash into something!

**Real example:** The drive subsystem updates its odometry (where it thinks it is on the field) based on encoder readings. Commands that need to know the robot's position get accurate, fresh data.

**Step 2: Poll Command Scheduling Triggers**
```cpp
void Scheduler::step2_pollCommandSchedulingTriggers() {
    controller->poll();  // Check button states and schedule new commands
}
```

**What's happening:** The controller checks all button and joystick states, detects edges (press/release events), and schedules new commands if buttons were pressed.

**Why this matters:** This is where user input turns into robot action. When the driver presses button A, this step is what notices and says "schedule the intake command."

**Why do this after updating subsystems?** If a button binding has a condition like "only run this command if the lift is at the bottom," we need the current lift position from Step 1.

**Step 3: Run and Finish Scheduled Commands**
```cpp
void Scheduler::step3_runAndFinishScheduledCommands() {
    for (auto* cmd : activeCommands) {
        if (cmd->isFirstRun) {
            cmd->initialize();          // First time? Run initialize
            cmd->isFirstRun = false;
        }
        
        cmd->execute();                 // Run the command's main logic
        
        if (cmd->isFinished()) {        // Check if it's done
            cmd->end();                 // Clean up
            removeCommand(cmd);         // Remove from active list
        }
    }
}
```

**What's happening:** This is the heart of the scheduler. Every active command gets to run its `execute()` method, and we check if it's finished.

**The command lifecycle in detail:**
1. **First loop:** `isFirstRun` is true → call `initialize()` once, then set `isFirstRun` to false
2. **Every loop:** Call `execute()` - this is the command doing its work
3. **Every loop:** Check `isFinished()` - is the command done?
4. **If done:** Call `end()` for cleanup, then remove command from active list

**Example with a "DriveForward2Feet" command:**
- Loop 1: `initialize()` records starting position. `execute()` starts driving. `isFinished()` returns false (only moved 0.1 feet)
- Loop 2-99: `execute()` keeps driving. `isFinished()` keeps returning false
- Loop 100: `execute()` still driving. `isFinished()` returns true (we've gone 2 feet!). `end()` stops motors. Command removed.

**Step 4: Schedule Default Commands**
```cpp
void Scheduler::step4_scheduleDefaultCommands() {
    for (auto& [subsystem, defaultCmd] : defaultCommands) {
        if (!subsystem->getCurrentCommand()) {
            scheduleCommand(defaultCmd);  // Start default if subsystem is idle
        }
    }
}
```

**What's happening:** For every subsystem that has a default command, if no other command is currently using it, start the default command.

**Why this matters:** This is what makes the robot responsive. The drive subsystem's default command is "respond to joysticks." So whenever no special driving command is running, the robot automatically responds to the driver's input.

**Example:** 
- Driver is controlling robot normally (default DriveCommand is running)
- Driver presses button to run "TurnToTarget" command
- TurnToTarget interrupts DriveCommand and takes over the drive subsystem
- TurnToTarget finishes turning
- Step 4 notices drive subsystem is now idle, restarts DriveCommand
- Robot instantly responds to joysticks again - the driver doesn't have to do anything

**Why Break It Into Steps?**

**Benefits:**

1. **Debuggable:** If something goes wrong, we can add debug prints at each step to see exactly where the problem is:
   ```cpp
   printf("Step 1 complete, updated 3 subsystems\n");
   printf("Step 2 complete, scheduled 1 new command\n");
   printf("Step 3 complete, running 4 active commands\n");
   printf("Step 4 complete, started 1 default command\n");
   ```

2. **Testable:** We can write unit tests for each step independently. We can test that Step 1 updates subsystems correctly without needing Steps 2-4 to work.

3. **Readable:** The main `run()` method now reads like a check list: "First update subsystems, then check buttons, then run commands, then fill idle time with defaults." Anyone can understand the flow instantly.

4. **Maintainable:** If we want to add a new step (like "Step 1.5: Check for emergency stops"), we know exactly where to add it and we know it won't break other steps.

**Development Roadmap (TODO.md):**

We created a structured development plan to track what features we still need to build:

```markdown
## Immediate Tasks:
- Test everything (subsystem periodic, controller binding, command lifecycle)
- Change how the main loop waits (currently uses pros::delay())
- Finalize drive command implementation

## Advanced Features to Add:

### Command Groups:
class SequentialCommandGroup : public CommandBase {
    // Runs commands one after another
    // Example: DriveForward → Intake → DriveBack → Score
};

class ParallelCommandGroup : public CommandBase {
    // Runs multiple commands simultaneously  
    // Example: Drive + RunIntake + UpdateLEDs (all at once)
};

### Command Decorators:
- withTimeout(seconds)  // Auto-end command after time
- repeatedly()          // Loop command forever
- andThen(nextCmd)      // Chain commands together
- alongWith(parallelCmd) // Run commands in parallel
```

**What are Command Groups?**

Command groups let you combine simple commands into complex behaviors:

**Sequential:** Do things in order
```cpp
// Autonomous: drive to goal, score, drive back
auto scoreSequence = new SequentialCommandGroup(
    new DriveToGoal(),
    new ScorePreload(),
    new DriveBack()
);
```

**Parallel:** Do multiple things at once
```cpp
// Drive while running intake and updating display
auto driveAndIntake = new ParallelCommandGroup(
    new DriveForward(),
    new RunIntake(),
    new UpdateLEDs()
);
```

**What are Command Decorators?**

Decorators modify commands to add extra behavior:

```cpp
// Run intake, but stop after 2 seconds no matter what
auto timedIntake = new RunIntake().withTimeout(2.0);

// Drive forward, then turn, then drive again
auto path = new DriveForward().andThen(new Turn90()).andThen(new DriveForward());

// Drive forward repeatedly until interrupted
auto keepDriving = new DriveForward1Foot().repeatedly();
```

These make it much easier to build complex autonomous routines from simple building blocks.

**Documentation Priorities:**
```markdown
### Add block comments for documentation
- controller.cpp/.h
- scheduler.cpp/.h  
- subsystemBase.cpp/.h
- commandBase.h

### Custom documentation for examples
- exampleCommand.h
- exampleSubsystem.h
```

We're tracking what needs detailed documentation after the code is in a reusable state.

**Files Changed:** 3 files
- `scheduler.h` - New method declarations for the 4 steps
- `scheduler.cpp` - Refactored implementation split into steps
- `TODO.md` - Development roadmap created

**Watchdog Performance Monitoring System:**

As our scheduler grew more complex with multiple subsystems, controllers, and commands all running in a tight 20ms loop, we needed a way to identify performance bottlenecks. If any part of the loop takes too long, the robot becomes unresponsive or jerky.

**The Problem:**

During testing, we occasionally noticed the robot feeling "sluggish" - controls would respond slower than expected, movements would stutter slightly. We suspected one of the scheduler steps was taking too long, but which one? Printing timestamps everywhere would clutter the code and slow things down even more.

**The Solution - Watchdog Class:**

We created a lightweight performance monitoring system (`watchdog.h`) that tracks timing for each part of the scheduler loop:

```cpp
class Watchdog {
public:
    struct Epoch {
        std::string name;        // "subsystems.periodic()", "controllers.poll()", etc.
        uint32_t timestamp;      // When this epoch started (ms since robot boot)
        uint32_t duration;       // How long this epoch took (ms)
    };
    
    // Start timing a new phase of the loop
    void addEpoch(const std::string& name) {
        uint32_t currentTime = pros::millis();
        
        if (m_epochs.empty()) {
            m_startTime = currentTime;
            m_lastEpochTime = currentTime;
        }
        
        Epoch epoch;
        epoch.name = name;
        epoch.timestamp = currentTime - m_startTime;
        epoch.duration = currentTime - m_lastEpochTime;
        
        m_epochs.push_back(epoch);
        m_lastEpochTime = currentTime;
    }
    
    // Reset for new scheduler loop
    void reset() {
        m_epochs.clear();
        m_startTime = 0;
        m_lastEpochTime = 0;
    }
    
    // Check if any phase took too long
    bool hasSlowEpochs(uint32_t thresholdMs = 5) const {
        for (const auto& epoch : m_epochs) {
            if (epoch.duration > thresholdMs) {
                return true;
            }
        }
        return getTotalTime() > 20;  // Total loop should be ~20ms
    }
    
    // Print detailed breakdown (only when slow!)
    void printEpochs() {
        customPrint::printf("[WATCHDOG] Performance Summary:\n");
        for (const auto& epoch : m_epochs) {
            customPrint::printf("  %s: %dms\n", epoch.name.c_str(), epoch.duration);
        }
        customPrint::printf("  Total Loop: %dms\n", getTotalTime());
    }
};
```

**Integration with Scheduler:**

```cpp
void Scheduler::run() {
    // Reset watchdog for new scheduler loop
    m_watchdog.reset();
    
    // Step 1: Update subsystems
    step1_runSubsystemPeriodicMethods();
    m_watchdog.addEpoch("subsystems.periodic()");
    
    // Step 2: Poll controller inputs
    step2_pollCommandSchedulingTriggers();
    m_watchdog.addEpoch("controllers.poll()");
    
    // Step 3: Execute active commands
    step3_runAndFinishScheduledCommands();
    m_watchdog.addEpoch("commands.execute()");
    
    // Step 4: Schedule default commands
    step4_scheduleDefaultCommands();
    m_watchdog.addEpoch("defaults.schedule()");
    
    // Check if loop was slow and print diagnostics
    if (m_watchdog.hasSlowEpochs()) {
        customPrint::printf("[SCHEDULER] WARNING: Slow loop detected!\n");
        m_watchdog.printEpochs();
    }
}
```

**How It Works:**

1. **Reset at loop start:** Clear all timing data from the previous loop
2. **Mark epochs:** After each scheduler step, record how long it took
3. **Detect problems:** If any step took >5ms or total >20ms, we have a problem
4. **Report details:** Print a breakdown showing exactly which step was slow

**Example Output (Normal Loop):**
```
(No output - everything fast enough, no warnings printed)
```

**Example Output (Slow Loop):**
```
[SCHEDULER] WARNING: Slow loop detected!
[WATCHDOG] Performance Summary:
  subsystems.periodic(): 2ms
  controllers.poll(): 1ms
  commands.execute(): 15ms    ← Problem found!
  defaults.schedule(): 1ms
  Total Loop: 19ms
```

**Interpreting Results:**

If we see "commands.execute(): 15ms" is slow, we know a command is doing too much work in its `execute()` method. Common causes:
- Blocking operations (waiting for sensor response)
- Heavy calculations (path planning, inverse kinematics)
- Too many print statements
- Busy-wait loops

**Why 20ms Target?**

FRC robots run at 50Hz (50 loops per second), which means each loop must complete in 20ms or less. VEX doesn't have a strict requirement, but matching FRC's timing ensures responsive controls. If loops take 50ms, that's only 20 updates per second - the robot will feel laggy.

**Performance Threshold Rationale:**

- **5ms per epoch:** With 4 main steps, if each took 5ms, total would be 20ms (our limit)
- **20ms total:** This is our hard deadline. Beyond this, we're dropping below 50Hz.

**Real-World Impact:**

After implementing the watchdog, we discovered that one of our commands was calling `pros::delay(10)` inside `execute()` - adding 10ms of unnecessary delay every loop! The watchdog immediately pointed us to the problem:

```
[WATCHDOG] Performance Summary:
  commands.execute(): 12ms    ← 10ms delay + 2ms actual work
```

We removed the delay, and the robot instantly felt more responsive.

**Smart Design Choices:**

1. **Silent when fast:** No output when everything's working - doesn't spam the console
2. **Detailed when slow:** When there's a problem, tells you exactly which step and how long
3. **Lightweight:** Only stores timing data, minimal overhead (<0.1ms)
4. **Easy to extend:** Want to add sub-timings within a step? Just add more addEpoch() calls

**Advanced Usage - Detecting Patterns:**

```cpp
// Track how often we get slow loops
int slowLoopCount = 0;
if (m_watchdog.hasSlowEpochs()) {
    slowLoopCount++;
    if (slowLoopCount > 50) {
        customPrint::printf("[WATCHDOG] ERROR: Persistent slow loops! Check your commands!\n");
        // Maybe even disable problematic commands automatically
    }
}
```

This pattern lets us detect not just occasional hiccups, but systemic performance problems that need immediate attention.

**Connection to FRC WPILib:**

FRC's WPILib has a similar watchdog system that monitors loop timing and throws warnings when loops are slow. By implementing our own, we:
- Maintain parity with FRC best practices
- Gain visibility into performance that would otherwise be invisible
- Build good habits for competition robotics

The watchdog is a perfect example of "defensive programming". We assume problems will happen and build tools to detect and diagnose them before they become competition failures.

**[PHOTO NEEDED: Flowchart showing 4-step scheduler process with arrows connecting each step, and annotations showing what happens in each step]**
**[PHOTO NEEDED: Screenshot of TODO.md with sections highlighted showing Immediate Tasks, Advanced Features, and Documentation Priorities]**
**[PHOTO NEEDED: Terminal screenshot showing watchdog output with one slow epoch highlighted]**
**[PHOTO NEEDED: Diagram showing scheduler loop with timing annotations: subsystems (2ms) → controllers (1ms) → commands (3ms) → defaults (1ms) = 7ms total ✓]**

---

## Week 11
### Code Quality Improvements

**Commits: "Removed prevButtonStates" (October 1) and "Move core source files" (October 21)**

This week we cleaned up redundant code and improved project organization. Code quality work like this might not add new features, but it makes the code more maintainable, reduces bugs, and makes it easier for others to understand.

**Removed Redundant State Tracking:**

We discovered that button state was being tracked in two different places, creating unnecessary duplication. This is a violation of the "Single Source of Truth" principle. Data should only be stored in one place, otherwise the copies can get out of sync.

**What we found:**
```cpp
// BEFORE - in controller.h
class Controller : public pros::Controller {
    std::array<bool, 12> prevButtonStates;  // ← Storing button states here!
    std::vector<ButtonBinder> buttonBinders; // ← But also storing them here!
};
```

An `std::array<bool, 12>` is a fixed-size list of 12 boolean (true/false) values - one for each button on the V5 controller. We were using this to remember "was this button pressed last loop?"

**The Problem:**

The `ButtonBinder` class was already tracking previous button state internally to detect edges (transitions from unpressed to pressed or vice versa). So we were storing the same information twice:
1. In the Controller's `prevButtonStates` array
2. In each ButtonBinder's internal `previousState` variable

**Why this is bad:**

- **Duplicating data unnecessarily:** We're using extra memory for no reason
- **Risk of desync:** What if we update one copy but forget to update the other? Now they disagree about whether a button was pressed
- **Violating Single Source of Truth:** If you want to know "was button A pressed last loop," which copy do you trust?
- **Confusing code:** New programmers looking at this would wonder "Why are button states tracked twice? Which one is the real one?"

**The Solution:**
```cpp
// AFTER - in controller.h
class Controller : public pros::Controller {
    std::vector<ButtonBinder> buttonBinders;  // Each binder tracks its own state
    std::vector<JoystickBinder> joystickBinders;
};
```

We removed the redundant array. Now state only lives in one place: inside each binder.

**How ButtonBinder Actually Works:**

```cpp
class ButtonBinder {
private:
    bool previousState = false;  // ← State lives HERE, nowhere else
    
public:
    void poll() {
        bool currentState = controller->get_digital(button);  // Read button NOW
        
        // Detect rising edge (button just pressed)
        if (currentState && !previousState) {
            if (edge == Edge::Rising) {
                scheduler->scheduleCommand(command);
            }
        }
        
        // Detect falling edge (button just released)
        if (!currentState && previousState) {
            if (edge == Edge::Falling) {
                scheduler->cancelCommand(command);
            }
        }
        
        previousState = currentState;  // Remember for next loop
    }
};
```

**Source Code Organization:**

We also reorganized source file locations to mirror the include directory structure. This makes the project more intuitive to navigate.

**Before (flat structure):**
```
src/
  ├── controller.cpp
  ├── scheduler.cpp
  ├── subsystemBase.cpp
  └── main.cpp
```

**After (organized structure):**
```
src/
  ├── custom/              # ← New folder matching include/custom/
  │   ├── controller.cpp
  │   ├── scheduler.cpp
  │   └── subsystemBase.cpp
  └── main.cpp            # Application entry point stays at top level
```

**Why This Organization is Better:**

1. **Mirrors include structure:** 
   - `include/custom/controller.h` corresponds to `src/custom/controller.cpp`
   - Makes it easy to find the implementation file for any header

2. **Clearer separation:**
   - `src/custom/` = framework code (our command system)
   - `src/` = application code (main.cpp, robot-specific code)
   - Anyone can instantly see what's framework vs what's application

3. **Professional structure:**
   - This matches how most C++ projects are organized
   - Makes our code look professional and mature
   - Easier for people from other C++ projects to understand

4. **Better for open source:**
   - If we release this framework, users can clearly see what files they need
   - The custom folder can be copied as-is to other projects

5. **IDE navigation:**
   - File trees in VS Code now group related files together
   - Autocomplete and "Go to Definition" work better with organized structure

**Files Changed:** 8 files total across both commits
- controller.h - Removed prevButtonStates array
- controller.cpp - Removed initialization code for that array
- exampleCommand.h - Fixed uninitialized count variable
- Various file moves to src/custom/
- TODO.md - Updated priorities

**Impact of Code Quality Work:**

While these changes don't add new features, they make the codebase:
- **More reliable:** Fewer bugs from uninitialized variables and state desync
- **More efficient:** Less memory used, fewer redundant operations
- **Easier to understand:** Clear organization and single source of truth
- **More maintainable:** Changes are easier when there's less redundancy

**Advanced Command Decorators:**

As we continued to refine the framework, we added several advanced decorators that provide more control over command behavior, particularly for competition scenarios.

**ignoringDisable() Decorator:**

This decorator allows a command to continue running even when the robot is disabled. By default, the Scheduler cancels all commands when the robot enters disabled state (between matches, during timeouts, etc.). However, some commands should keep running.

```cpp
// Command that keeps running even when robot is disabled
std::unique_ptr<CommandBase> holdCommand = std::make_unique<Hold>(subsystem.get())
    ->ignoringDisable();
```

**Why would you want this?**

1. **Holding mechanisms in position:** If your lift is holding a game object, you don't want it to drop when the robot is disabled between match states (autonomous  -> teleop). The "hold position" command should ignore disable.

2. **Logging and diagnostics:** Commands that record data or display information to the driver station don't control motors - they should keep working when disabled.

3. **Safety monitoring:** A command that watches for dangerous conditions (overheating, tipping) should never be disabled.

**How it works:**

```cpp
class CommandBase {
    bool m_runsWhenDisabled = false;
    
    CommandBase* ignoringDisable() {
        m_runsWhenDisabled = true;
        return this;  // Return 'this' to allow method chaining
    }
    
    bool runsWhenDisabled() const {
        return m_runsWhenDisabled;
    }
};
```

In the Scheduler:
```cpp
void Scheduler::run() {
    // ...execute commands...
    
    // When robot is disabled, cancel commands that don't ignore disable
    if (!m_enabled) {
        for (auto* cmd : activeCommands) {
            if (!cmd->runsWhenDisabled()) {
                interruptCommand(cmd);  // Stop this command
            }
        }
    }
}
```

**withInterruptBehavior() Decorator:**

This decorator controls what happens when a new command wants to use the same subsystem. There are two behaviors:

```cpp
enum class InterruptionBehavior {
    kCancelSelf,      // Default: This command gets interrupted by new commands
    kCancelIncoming   // Defensive: This command rejects new commands trying to interrupt
};
```

**kCancelSelf (Default):**
```cpp
// Normal behavior - new commands can interrupt this one
std::unique_ptr<CommandBase> normalCommand = std::make_unique<DriveForward>(drive.get());
// If driver presses button to turn, this command gets interrupted
```

**kCancelIncoming (Defensive):**
```cpp
// Critical command that must not be interrupted
std::unique_ptr<CommandBase> criticalCommand = std::make_unique<EmergencyStop>(drive.get())
    ->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);
// If driver tries to press buttons while emergency stopping, those commands are rejected
```

**When to use kCancelIncoming:**

1. **Safety-critical operations:**
   ```cpp
   // Emergency stop must complete - don't let driver override
   emergencyStop->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);
   ```

2. **Autonomous routines that must complete:**
   ```cpp
   // Don't let stray button presses interrupt carefully-tuned autonomous
   autonomousSequence->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);
   ```

3. **Calibration sequences:**
   ```cpp
   // Sensor calibration must complete fully - partial calibration is worse than none
   calibrateSensors->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);
   ```

**Implementation:**

```cpp
class CommandBase {
    InterruptionBehavior m_interruptBehavior = InterruptionBehavior::kCancelSelf;
    
    CommandBase* withInterruptBehavior(InterruptionBehavior behavior) {
        m_interruptBehavior = behavior;
        return this;  // Method chaining
    }
    
    InterruptionBehavior getInterruptionBehavior() const {
        return m_interruptBehavior;
    }
};
```

In the Scheduler, when scheduling a new command:
```cpp
void Scheduler::schedule(CommandBase* incoming) {
    for (auto* subsystem : incoming->getRequiredSubsystems()) {
        CommandBase* current = subsystem->getCurrentCommand();
        
        if (current != nullptr) {
            // Check if current command allows itself to be interrupted
            if (current->getInterruptionBehavior() == InterruptionBehavior::kCancelIncoming) {
                // Current command is defensive - reject the incoming command
                customPrint::printf("[SCHEDULER] Command rejected: %s (blocked by %s)\n",
                    incoming->getName().c_str(), current->getName().c_str());
                return;  // Don't schedule the incoming command
            } else {
                // Normal behavior - interrupt the current command
                interruptCommand(current);
            }
        }
    }
    
    // Schedule the incoming command
    addCommand(incoming);
}
```

**Combining Decorators:**

Decorators can be chained together for complex behavior:

```cpp
// A critical hold command that runs when disabled and can't be interrupted
std::unique_ptr<CommandBase> safeHold = std::make_unique<Hold>(lift.get())
    ->ignoringDisable()
    ->withInterruptBehavior(InterruptionBehavior::kCancelIncoming)
    ->withName("CriticalHold");
```

This creates a command that:
1. Keeps running when robot is disabled (holds lift in place)
2. Cannot be interrupted by other commands (stays active)
3. Has a descriptive name for debugging

**Connection to FRC WPILib:**

These patterns come directly from FRC's command framework:
- FRC's `RunsWhenDisabled()` → Our `ignoringDisable()`
- FRC's `InterruptionBehavior` enum → Same in our implementation

By maintaining this parity, team members with FRC experience can immediately understand our VEX code, and vice versa. It also means we can refer to FRC documentation and examples when learning advanced patterns.

**[PHOTO NEEDED: Before/after code comparison showing redundant prevButtonStates array vs clean single-location state tracking]**
**[PHOTO NEEDED: Side-by-side file explorer showing before structure (flat) vs after structure (src/custom/ folder)]**
**[PHOTO NEEDED: Flowchart showing scheduler.schedule() decision tree with InterruptionBehavior logic highlighted]**

---

## Week 12
### Controller-Scheduler Decoupling

**Commit: "Fixed controller setting a pointer to scheduler" (October 21)**

This week we made a major architectural improvement to properly decouple the Controller and Scheduler classes. "Decoupling" means reducing dependencies between components so they can work independently.

**Understanding the Problem - Tight Coupling:**

**What is coupling?**
Coupling is how much one class depends on another. Think of it like tools:
- **Tightly coupled:** A drill bit that only fits one specific drill (if you lose the drill, the bit is useless)
- **Loosely coupled:** A standard drill bit that fits any drill with the right chuck (much more flexible)

**Our tight coupling problem:**
```cpp
// OLD DESIGN - Tightly coupled
class Controller {
    Scheduler* m_scheduler;  // Controller "owns" a pointer to scheduler
    
    Controller(pros::controller_id_e_t id, Scheduler* sch) 
        : pros::Controller(id), m_scheduler(sch) {
        // Controller needs a scheduler to even be created!
    }
    
    void poll() {
        // Controller directly schedules commands on m_scheduler
        if (buttonPressed) {
            m_scheduler->scheduleCommand(cmd);
        }
    }
};
```

**Issues with this design:**

1. **Controller depends on Scheduler existing:** 
   - You can't create a Controller without providing a Scheduler
   - Makes testing hard - to test button detection, you need a full working scheduler

2. **Hard to use controller without scheduler:**
   - What if you just want to read button values? You still need a scheduler object
   - The controller and scheduler are paired - they can't exist independently

3. **Violates Dependency Inversion Principle:**
   - High-level components (Controller) shouldn't depend directly on other high-level components (Scheduler)
   - They should both depend on abstractions (interfaces)

4. **Makes it harder to have multiple controllers or schedulers:**
   - What if you want to have both a master and partner controller?
   - What if you want commands from different controllers to go to different schedulers?
   - The tight coupling makes this awkward

5. **Creates questions about ownership:**
   - Does Controller own the Scheduler? 
   - Who's responsible for deleting the Scheduler when we're done?
   - Unclear ownership leads to memory leaks or double-delete bugs

**The Solution - Global Auto-Registration:**

By using auto-registration rather than per-binder dependency injection. `Controller` maintains a static global scheduler pointer and a list of controllers created before a scheduler exists:

- `static Scheduler* Controller::m_globalScheduler = nullptr;` — a single global scheduler pointer shared by all controllers.
- `static std::vector<Controller*> Controller::m_pendingControllers;` — controllers created before the scheduler is set are stored here.
- `Controller::Controller(pros::controller_id_e_t id)` registers the controller with `m_globalScheduler` if it's already set; otherwise it pushes the controller onto `m_pendingControllers`.
- `Controller::setScheduler(Scheduler* sch)` sets `m_globalScheduler` and calls `registerPendingControllers()` to register any controllers created earlier.

Button/joystick binders do not hold their own `Scheduler*`. Instead, binder callbacks (`onTrue`, `onFalse`, `whileTrue`) store the command and add the binder to the controller's internal binder vectors. When `poll()` runs, the binder code consults `Controller::m_globalScheduler` and calls the scheduler's `schedule()`/`cancel()` APIs only if the global scheduler is set. Example behavior from `controller.cpp`:

```cpp
// In ButtonBinder::poll()
if (m_edge == Edge::Rising) {
    bool curr = m_controller->get_digital_new_press(m_button);
    if (curr && Controller::m_globalScheduler && km_command)
        Controller::m_globalScheduler->schedule(km_command);
}
// whileTrue() keeps a running instance via the global scheduler
```

This means the framework supports a simple global registration model: controllers and binders operate independently of scheduler construction order, and the scheduler is "injected" globally via `Controller::setScheduler(...)`.

**Controller Scheduler Wiring**

The repository uses a global auto-registration pattern rather than storing a per-binder `Scheduler*` everywhere. Controllers and binders work even if the scheduler is constructed later.

Key points:

- There is a single global pointer used by controllers: `static Scheduler* Controller::m_globalScheduler = nullptr;` and a list for controllers created early: `static std::vector<Controller*> Controller::m_pendingControllers`.
- When the system starts, code calls `Controller::setScheduler(...)` to provide the scheduler. That function sets `m_globalScheduler` and registers any pending controllers created earlier.
- Binders do not permanently own a `Scheduler*`. When `poll()` sees an event it checks `Controller::m_globalScheduler` and only calls the scheduler APIs if the global pointer is set.

Example usage (startup):

```cpp
// create or get scheduler
Controller::setScheduler(&Scheduler::getInstance()); // enable auto-registration
Controller master(E_CONTROLLER_MASTER);
master.A().onTrue(intakeCommand);
```

Example binder polling (simplified):

```cpp
// in ButtonBinder::poll()
bool pressed = m_controller->get_digital_new_press(m_button);
if (pressed && Controller::m_globalScheduler && m_command) {
    Controller::m_globalScheduler->schedule(m_command);
}
```

Why this matters:

- **Flexible construction order:** You can construct controllers before the scheduler exists; they'll be registered when `setScheduler()` runs.
- **Decoupled testing:** Controllers can be tested for input handling without a running scheduler; binders simply record events.
- **Clear startup wiring:** A single call to `Controller::setScheduler(...)` wires all binder scheduling behavior to the runtime scheduler.

Notes:

- The documentation concept of "dependency injection" still applies as a general pattern, but the repo uses a global injection point (the `setScheduler` call) plus deferred registration to keep construction simple and robust.
- There is no per-binder `Scheduler*` required in the current implementation; scheduling is guarded by the global pointer being non-null.


**Usage:**

There are two common patterns used in the repo:

1) Explicit global registration (recommended for clarity):

```cpp
Scheduler scheduler;                             // create scheduler
Controller::setScheduler(&scheduler);            // set global scheduler
Controller controller(E_CONTROLLER_MASTER);     // controller will auto-register

// Bind normally (binder uses Controller::m_globalScheduler internally)
controller.A().onTrue(new IntakeCommand());
```

2) Use the Scheduler singleton (already present in `src/custom/scheduler.cpp`):

```cpp
Controller::setScheduler(&Scheduler::getInstance()); // enable auto-registration to singleton
Controller controller(E_CONTROLLER_MASTER);
controller.A().onTrue(new IntakeCommand());
```

Notes:
- If you create `Controller` instances before calling `Controller::setScheduler(...)`, the constructor pushes them onto `m_pendingControllers`. Calling `setScheduler()` later registers those pending controllers with the scheduler (see `registerPendingControllers()`).
- Binders call into `Controller::m_globalScheduler` when `poll()` detects events; scheduling only happens if the global scheduler pointer is non-null.
- There is no per-binder `Scheduler*` stored in the current implementation; the code relies on the global/static scheduler pointer for scheduling.

**How It Works Internally:**

```cpp
class Controller {
    ButtonBinder& setButtonCommand(Scheduler* scheduler) {
        // Create a new binder with the injected scheduler
        m_buttonBinders.emplace_back(scheduler);
        
        // Return reference so we can chain .onTrue()
        return m_buttonBinders.back();
    }
};
```

When you call `setButtonCommand(&scheduler)`, it:
1. Creates a new ButtonBinder
2. Passes the scheduler pointer to that binder (injection!)
3. Returns the binder so you can call `.onTrue()` on it (method chaining from Week 4)

**Architecture Pattern - Dependency Injection:**

This follows a well-established software design pattern:
- **Dependencies** (things you need, like the scheduler) are **injected** (passed in) where needed
- Not stored in intermediate classes that don't use them
- More flexible and maintainable

This is one of the SOLID principles (the "D" - Dependency Inversion Principle). Good architecture makes code easier to understand, test, and modify.

**Real-World Analogy:**

Think of building a house:
- **Bad (tight coupling):** The house comes with built-in, non-removable appliances. If the fridge breaks, you might need to rebuild the kitchen.
- **Good (loose coupling):** The house has standard outlets and hookups. You can plug in any fridge you want. If one breaks, swap it out without touching the house.

Our new design is like having standard interfaces - binders plug into schedulers, but they're not permanently connected.

**Files Changed:** 4 files
- `controller.h` - Removed scheduler member variable and parameter
- `controller.cpp` - Updated constructor, binders now receive scheduler
- `exampleSubsystem.h` - Updated comments to reflect new pattern
- `main.cpp` - Updated controller initialization and binding syntax

**Impact:**

This architectural improvement makes our framework:
- **More professional:** Follows industry-standard design patterns
- **More flexible:** Can adapt to new use cases without redesign
- **More testable:** Can test components in isolation
- **More maintainable:** Clear responsibilities and dependencies
- **Ready for open source:** Other teams can use it in different ways

**[PHOTO NEEDED: UML diagram showing old design (Controller → Scheduler) vs new design (Controller → Binders → Scheduler) with arrows showing dependency flow]**
**[PHOTO NEEDED: Code snippet comparison showing before (passing scheduler to Controller constructor) vs after (injecting scheduler into binding)]**

---

## Week 13
### Controller-Scheduler Decoupling (Continued)

**Commit: "Made basic changes to Test2025, began adding LemLib support and other additional files"** (October 29)

This week we continued refining the framework architecture and began setting up our Test2025 project with the latest framework improvements.

**Setting Up Test2025 Project:**

After completing the major architectural improvements in Week 12 (Controller-Scheduler decoupling), we began applying these changes to our Test2025 competition robot project. This involved:

1. **Copying Framework Files:**
   - Transferred updated framework files from example-code to Test2025
   - Ensured all header and source files reflect the new dependency injection pattern
   - Updated include paths and dependencies

2. **LemLib Integration:**
   - Added LemLib support files to Test2025 project
   - Configured LemLib chassis parameters for our robot
   - Set up odometry tracking and PID tuning values
   - Prepared driveSubsystem for competition use

3. **Project Structure:**
   - Organized Test2025 with same include/src layout as example-code
   - Separated framework code (custom/) from robot-specific code
   - Configured Makefile for proper building and uploading

**Next Steps:**

With the framework architecture now stable and the Test2025 project set up, we're ready to:
- Build subsystems for our competition robot mechanisms
- Create commands for autonomous routines
- Tune motion control parameters
- Test framework performance on real hardware

**Files Changed:** Multiple files in Test2025 project
- Added/updated framework headers in `include/custom/`
- Added/updated framework implementations in `src/custom/`
- Configured LemLib integration
- Updated Makefile and project configuration

**[PHOTO NEEDED: Test2025 project structure showing framework integration]**
**[PHOTO NEEDED: Robot with Test2025 code running]**
