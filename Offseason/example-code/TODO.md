## Note
### add into command:
- Command Groups:
    - SequentialCommandGroup
    - ParallelCommandGroup

- Command Decorators:
    - andThen(CommandBase* next);


## Updated
added suffix 'm_' for all member variables and 'k' for all constants


## Needs Testing
### command class:
- getName() opcontrol [DONE]
- getInterruptionBehavior() [DONE]
- ignoringDisable() ignoringDisableCommand should persist after comp mode
- setComposed()/isComposed() AFTER andThen() is created
- addRequirements() [DONE]
- getRequiredSubsystems() [DONE]
- DECORATORS
    - TimeoutCommand() [DONE]
    - NamedCommand() [DONE]
    - WaitCommand() [DONE]
    - RunForCommand() [DONE]
    - FunctionalCommand() [DONE]
    - InstantCommand() [DONE]
- commands taking many subsystems
    - InstantCommand [DONE]
    - FunctionalCommand [DONE]


### subsystem class:
- setDefaultCommand() [DONE]
- getCurrentCommand() [DONE]
- setScheduler() [DONE]
- registerPendingSubsystems() [DONE]
- FACTORY METHODS
    - runOnce()
    - run()
    - runUntil()
    - runFor()


### scheduler:
- setDefaultCommand() [DONE]
- cancel()
- cancelAll()
- enable() [DONE]
- disable() [DONE]
- isEnabled() [DONE]
- isScheduled() [DONE]
- requiring() [DONE]
- Command event callbacks
    - onCommandInitialize()
    - onCommandExecute()
    - onCommandFinish()
    - onCommandInterrupt()
- setRobotEnabled() [DONE]
- isRobotEnabled() [DONE]
- run()
    - initCommand() [DONE]
    - interruptCommand() [DONE]
    - finishCommand() [DONE]
    - requirementsFree() [DONE]
    - areCommandsInterruptible()


### verify all watchdog functions


## Continue
### map out vex control functions with Scheduler
- opcontrol()
- initialize()
- disabled()
- autonomous()


### continue working on:
- example commands
- drive command