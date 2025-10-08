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
- getName()
- setInterruptible()/getInterruptionBehavior()
- setRunsWhenDisabled()/runsWhenDisabled()
- setComposed()/isComposed() AFTER andThen() is created
- addRequirements()
- getRequiredSubsystems()
- DECORATORS
    - TimeoutCommand()
    - NamedCommand()
    - WaitCommand()
    - RunForCommand()
    - FunctionalCommand()
    - InstantCommand() done
- commands taking many subsystems
    - InstantCommand
    - FunctionalCommand
    - StartEndCommand
    - . . .


### subsystem class:
- setDefaultCommand()
- getCurrentCommand()
- setScheduler()
- registerPendingSubsystems()
- FACTORY METHODS
    - runOnce()
    - run()
    - runUntil()
    - runFor()


### scheduler:
- setDefaultCommand()
- cancel()
- cancelAll()
- enable() done
- disable() done
- isEnabled() done
- isScheduled()
- requiring()
- Command event callbacks
    - onCommandInitialize()
    - onCommandExecute()
    - onCommandFinish()
    - onCommandInterrupt()
- setRobotEnabled()
- isRobotEnabled() done
- run()
    - initCommand() done
    - interruptCommand() done
    - finishCommand() done
    - requirementsFree()
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