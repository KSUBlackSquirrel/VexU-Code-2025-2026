## Note
### add into command:
- Command Groups:
    - SequentialCommandGroup
    - ParallelCommandGroup

- Command Decorators:
    - andThen(CommandBase* next);

## Needs Testing
### command class:
- getName() opcontrol [DONE]
- getInterruptionBehavior() [DONE]
- ignoringDisable() *added test first*
- setComposed()/isComposed() AFTER andThen() is created
- addRequirements() [DONE]
- getRequiredSubsystems() [DONE]
- DECORATORS [DONE]
    - TimeoutCommand() [DONE]
    - NamedCommand() [DONE]
    - WaitCommand() [DONE]
    - RunForCommand() [DONE]
    - FunctionalCommand() [DONE]
    - InstantCommand() [DONE]
- commands taking many subsystems [DONE]
    - InstantCommand [DONE]
    - FunctionalCommand [DONE]


### subsystem class:
- setDefaultCommand() [DONE]
- getCurrentCommand() [DONE]
- setScheduler() [DONE]
- registerPendingSubsystems() [DONE]
- FACTORY METHODS *added test second*
    - runOnce() *added*
    - run() *added*
    - runUntil() *added*
    - runFor() *added*


### scheduler:
- setDefaultCommand() [DONE]
- cancel() [DONE]
- cancelAll() *added test first*
- enable() [DONE]
- disable() [DONE]
- isEnabled() [DONE]
- isScheduled() [DONE]
- requiring() [DONE]
- Command event callbacks [DONE]
    - onCommandInitialize() [DONE]
    - onCommandExecute() [DONE]
    - onCommandFinish() [DONE]
    - onCommandInterrupt() [DONE]
- setRobotEnabled() [DONE]
- isRobotEnabled() [DONE]
- run() [DONE]
    - initCommand() [DONE]
    - interruptCommand() [DONE]
    - finishCommand() [DONE]
    - requirementsFree() [DONE]
    - areCommandsInterruptible() [DONE]


### verify all watchdog functions
addEpoch() *added*
reset() *added*
printEpochs() *added*
getTotalTime() *added*
hasSlowEpochs() *added*


## Continue
### map out vex control functions with Scheduler
- opcontrol()
- initialize()
- disabled()
- autonomous()


### continue working on:
- example commands
- drive command
- comments for all functions with examples