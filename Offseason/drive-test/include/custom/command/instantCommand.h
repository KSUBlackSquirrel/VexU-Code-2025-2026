#ifndef INSTANTCOMMAND_H_
#define INSTANTCOMMAND_H_

#include "commandBase.h"
#include <functional>

class InstantCommand : public CommandBase  {
public:
    InstantCommand(std::function<void()> func, SubsystemBase* subsystem = nullptr) : action(func) {
        if (subsystem) addRequirements(subsystem);
    }

    inline void execute() override { if (action) action(); }

    inline bool isFinished() override { return true; }

    inline void end() override {}

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
