#ifndef TESTCOMMAND_H_
#define TESTCOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/exampleSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class TestCommand: public CommandBase {
public:
    TestCommand(ExampleSubsystem* sub) { 
        m_subsystem = sub;
        addRequirements(m_subsystem);
    }

    inline void execute() override {
        m_subsystem -> forward();
    }

    inline void end(bool interrupted) override {}

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new TestCommand(*this);
    }

private:
    ExampleSubsystem* m_subsystem;
};

#endif // DRIVECOMMAND_H_
