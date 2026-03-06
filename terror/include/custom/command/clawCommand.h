#pragma once

#include "commandBase.h"
#include "custom/subsystem/clawSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class ClawCommand: public CommandBase {
public:
    ClawCommand(ClawSubsystem* sub) { 
        m_subsystem = sub;
    }

    inline void execute() override {
        m_subsystem->toggleClaw();
    }

    inline void end(bool interrupted) override {
    }

    inline bool isFinished() override { return true; }

    inline CommandBase* clone() const override {
        return new ClawCommand(*this);
    }

private:
    ClawSubsystem* m_subsystem;
};
