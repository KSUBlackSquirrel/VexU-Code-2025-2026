#pragma once

#include "commandBase.h"
#include "custom/subsystem/driveSubsystem.h"
#include "custom/globals.h"

class SubsystemBase;

class TestAutoCommand: public CommandBase {
public:
    TestAutoCommand(DriveSubsystem* sub) { 
        m_subsystem = sub;
        addRequirements(m_subsystem);
    }

    inline void initialize() override {
        m_subsystem->resetPos();
    }

    inline void execute() override {
        m_subsystem->LinearPID();
    }

    inline void end(bool interrupted) override {}

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new TestAutoCommand(*this);
    }

private:
    DriveSubsystem* m_subsystem;
};
