# Week 17

## Day 1 — Template documentation (Sat Jan 17, 2026)

- Activity: added documentation and comments to the template code to improve onboarding and clarify expected usage patterns.

- Files updated in `development/template-code` (documentation-focused changes):
  - `include/custom/command/commandBase.h`
  - `include/custom/controller.h`
  - `include/custom/globals.h`
  - `include/custom/print.h`
  - `include/custom/scheduler.h`
  - `include/custom/subsystem/subsystemBase.h`
  - `include/custom/subsystem/template_subsystem.h`
  - `include/custom/watchdog.h`
  - `src/custom/controller.cpp`
  - `src/custom/scheduler.cpp`
  - `src/custom/subsystemBase.cpp`
  - `src/main.cpp`
  - `src/robot.cpp`

- Purpose: make the template clearer for future development and help onboarding new coders to command-based programming. No code changes were made, only comments, usage notes, and small clarifying edits.



## Day 2 — Auto-pathing integration attempt (Sat Jan 17, 2026)

Goal: add an auto-pathing helper directly to the `DriveSubsystem` in `development/active-dev-code`.
```
ASSET(example_txt);

inline void exampleAutoPath() {
    // set chassis pose
    m_chassis.setPose(0, 0, 0);
    // lookahead distance: 15 inches
    // timeout: 2000 ms
    m_chassis.follow(example_txt, 15, 5000);
    // follow the next path, but with the robot going backwards
    // m_chassis.follow(example2_txt, 15, 2000, false);
}
```

What we tried: implemented a path helper `exampleAutoPath()` inside `DriveSubsystem` and referenced an external `example_txt` path resource.

Build failure observed (PROS CLI 3.5.6, Kernel 4.2.1):

  Compiled src/custom/controller.cpp [ERRORS]
  In file included from ./include/custom/command/driveCommand.h:4,
                   from ./include/main.h:54,
                   from src/custom/controller.cpp:1:
  ./include/custom/subsystem/driveSubsystem.h: In member function 'void DriveSubsystem::exampleAutoPath()':
  ./include/custom/subsystem/driveSubsystem.h:41:30: error: 'example_txt' was not declared in this scope
  make: *** [common.mk:284: bin/custom/controller.cpp.o] Error 1
  ERROR - pros.cli.build:make - Failed to make project: Exit Code 2

Root-cause: the path function referenced `example_txt` which is not declared/visible at compile time. The implementation was placed in a header file, causing missing symbol visibility and build errors when included from multiple translation units.

Temporary workaround applied: switched hot/cold linking to `cold` to bypass the immediate build/link failure (this allowed the project to make progress for testing). The hope is to find a better way to fix this but it will have to work for now.

Next steps: because the robot is currently being disassembled we have no drivetrain to test on. when we have a working drivetrain we can start testing how PROS autonomous() function will interact with the command framework and leblib's follow() function.

