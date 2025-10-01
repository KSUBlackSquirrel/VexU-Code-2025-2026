change how the main loop waits

work on default command function

work on drive command

Add Command Groups:
class SequentialCommandGroup : public CommandBase {
    // Run commands one after another
};
class ParallelCommandGroup : public CommandBase {
    // Run commands simultaneously
};

Add Command Decorators:
CommandBase* withTimeout(double seconds);
CommandBase* repeatedly();
CommandBase* andThen(CommandBase* next);