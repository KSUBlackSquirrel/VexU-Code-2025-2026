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
- getName() opcontrol print
- getInterruptionBehavior() opcontrol print
- ignoringDisable() ignoringDisableCommand should persist after comp mode
- setComposed()/isComposed() AFTER andThen() is created
- addRequirements() [DONE]
- getRequiredSubsystems() opcontrol print
- DECORATORS
    - TimeoutCommand() added in confBind
    - NamedCommand() added in confBind
    - WaitCommand() added in confBind
    - RunForCommand() added in confBind
    - FunctionalCommand() added in confBind
    - InstantCommand() [DONE]
- commands taking many subsystems
    - InstantCommand
    - FunctionalCommand commented out
    - StartEndCommand
    - . . .


### subsystem class:
- setDefaultCommand() / added in initialize and controller X should interrupt
- getCurrentCommand() opcontrol print
- setScheduler() [DONE]
- registerPendingSubsystems() [DONE]
- FACTORY METHODS
    - runOnce()
    - run()
    - runUntil()
    - runFor()


### scheduler:
- setDefaultCommand() it is in initialize -> holdCommand
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