#ifndef DRIVECOMMAND_H_
#define DRIVECOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/driveSubsystem.h"
#include "custom/controller.h"
#include "custom/globals.h"

class SubsystemBase;

class DriveCommand: public CommandBase {
public:
    DriveCommand(DriveSubsystem* sub, Controller* ctrl) { 
        m_subsystem = sub;
        m_controller = ctrl;
        addRequirements(m_subsystem);
    }

    inline void execute() override {
        m_subsystem->tankDrive(m_controller, false);
    }

    inline void end(bool interrupted) override {}

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new DriveCommand(*this);
    }

private:
    DriveSubsystem* m_subsystem;
    Controller* m_controller;
};

#endif // DRIVECOMMAND_H_
