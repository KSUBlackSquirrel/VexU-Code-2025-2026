#pragma once

#include "commandBase.h"
#include "custom/subsystem/exampleSubsystem.h"

class Hold : public CommandBase {
public:
    Hold(ExampleSubsystem* exampleSub) { 
        exampleSubsystem = exampleSub;
        addRequirements(exampleSubsystem);
    }

    inline void initialize() override {}
    inline void execute() override { exampleSubsystem->setPosition(0, 100); }
    inline void end(bool interrupted) override {}
    inline bool isFinished() override { return false; }
    inline CommandBase* clone() const override { return new Hold(*this); }

private:
    ExampleSubsystem* exampleSubsystem;
};
