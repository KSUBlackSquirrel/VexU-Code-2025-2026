// InstantCommand.h
// Utility command for running a single action instantly (one tick).
// Useful for button bindings that trigger a lambda or function.
#ifndef INSTANTCOMMAND_H_
#define INSTANTCOMMAND_H_

#include "commandBase.h"
#include <functional>

class InstantCommand : public CommandBase  {
public:
    // Construct with a function to run and optional subsystem requirement
    InstantCommand(std::function<void()> func, SubsystemBase* subsystem = nullptr) : action(func) {
        if (subsystem) addRequirements(subsystem);
    }

    // Run the action once when scheduled
    inline void execute() override { if (action) action(); }

    // InstantCommand always finishes after one tick
    inline bool isFinished() override { return true; }

    // No cleanup needed
    inline void end() override {}

    // Clone the command, copying requirements
    inline CommandBase* clone() const override { 
        auto clone = new InstantCommand(action);
        // Copy requirements from original command
        for (auto* subsystem : getRequiredSubsystems()) {
            clone->addRequirements(subsystem);
        }
        return clone;
    }

private:
    std::function<void()> action;
};

#endif
