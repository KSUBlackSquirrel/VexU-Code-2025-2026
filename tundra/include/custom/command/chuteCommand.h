#pragma once

#include "commandBase.h"
#include "custom/subsystem/intakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class ChuteCommand: public CommandBase {
public:
    ChuteCommand(IntakeSubsystem* sub) { 
        m_subsystem = sub;
    }

    inline void execute() override {
        m_subsystem->toggleChute();
    }

    inline void end(bool interrupted) override {
    }

    inline bool isFinished() override { return true; }

    inline CommandBase* clone() const override {
        return new ChuteCommand(*this);
    }

private:
    IntakeSubsystem* m_subsystem;
};
