#ifndef OUTTAKECOMMAND_H_
#define OUTTAKECOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/intakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class OuttakeCommand: public CommandBase {
public:
    OuttakeCommand(IntakeSubsystem* sub) { 
        m_subsystem = sub;
        addRequirements(m_subsystem);
    }

    inline void execute() override {
        m_subsystem->runOut();
    }

    inline void end(bool interrupted) override {
        m_subsystem->stop();
    }

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new OuttakeCommand(*this);
    }

private:
    IntakeSubsystem* m_subsystem;
};

#endif // OUTTAKECOMMAND_H_
