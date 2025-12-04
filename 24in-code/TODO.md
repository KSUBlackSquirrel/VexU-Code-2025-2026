Find a way to test robotDisabled()

## Note
### add into command:
- Command Groups:
    - SequentialCommandGroup
    - ParallelCommandGroup

- Command Decorators:
    - andThen(CommandBase* next);


### verify all watchdog functions [add_a_wait_in_loop]
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

## LAST
### add block comments for documentation
controller.cpp/.h
scheduler.cpp/.h
subsystemBase.cpp/.h
commandBase.h
watchdog.h

## AFTER LAST
### custom documentation for examples
exampleCommand.h
exampleSubsystem.h