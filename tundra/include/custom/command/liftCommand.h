#pragma once

#include "commandBase.h"
#include "custom/subsystem/outtakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class LiftCommand: public CommandBase {
public:
    LiftCommand(OuttakeSubsystem* sub) { 
        m_subsystem = sub;
    }

    inline void execute() override {
        m_subsystem->toggleLift();
    }

    inline void end(bool interrupted) override {
    }

    inline bool isFinished() override { return true; }

    inline CommandBase* clone() const override {
        return new LiftCommand(*this);
    }

private:
    OuttakeSubsystem* m_subsystem;
};
