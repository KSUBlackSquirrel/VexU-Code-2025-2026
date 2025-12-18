#ifndef OUTTAKECOMMAND_H_
#define OUTTAKECOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/outtakeSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class OuttakeCommand: public CommandBase {
public:
    OuttakeCommand(OuttakeSubsystem* sub, bool reverse=false) { 
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
        return new OuttakeCommand(*this);
    }

private:
    OuttakeSubsystem* m_subsystem;
    bool m_reverse;
};

#endif // OUTTAKECOMMAND_H_
