#pragma once

#include "commandBase.h"
#include "custom/subsystem/outtakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class GateCommand: public CommandBase {
public:
    GateCommand(OuttakeSubsystem* sub, bool open) { 
        m_subsystem = sub;
        m_open = open;
    }

    inline void execute() override {
        m_subsystem->setGate(m_open);
    }

    inline void end(bool interrupted) override {
    }

    inline bool isFinished() override { return true; }

    inline CommandBase* clone() const override {
        return new GateCommand(*this);
    }

private:
    OuttakeSubsystem* m_subsystem;
    bool m_open;
};
