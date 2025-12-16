#ifndef INTAKECOMMAND_H_
#define INTAKECOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/intakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class IntakeCommand: public CommandBase {
public:
    IntakeCommand(IntakeSubsystem* sub, bool reverse=false) { 
        m_subsystem = sub;
        m_reverse = reverse;
        addRequirements(m_subsystem);
    }

    inline void execute() override {
        m_subsystem->run(m_reverse);
    }

    inline void end(bool interrupted) override {
        m_subsystem->stop();
    }

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new IntakeCommand(*this);
    }

private:
    IntakeSubsystem* m_subsystem;
    bool m_reverse;
};

#endif // INTAKECOMMAND_H_
