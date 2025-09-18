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
        subsystem = sub;
        controller = ctrl;
        addRequirements(subsystem);
    }

    inline void execute() override {
        subsystem->tankDrive(controller, false);
    }

    inline void end() override {}

    inline void interrupted() override {}

    inline bool isFinished() override { return false; }

    inline CommandBase* clone() const override {
        return new DriveCommand(*this);
    }

private:
    DriveSubsystem* subsystem;
    Controller* controller;
};

#endif
