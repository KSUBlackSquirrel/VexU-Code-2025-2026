#ifndef EXAMPLECOMMAND_H_
#define EXAMPLECOMMAND_H_

#include "commandBase.h"
#include "custom/subsystem/exampleSubsystem.h"

class Pulse : public CommandBase {
public:
    Pulse(ExampleSubsystem* exampleSub) { 
        exampleSubsystem = exampleSub;
        addRequirements(exampleSubsystem);
        count = 0;
    }

    inline void execute() override {
        uint32_t currentTime = pros::millis();
        
        if (exampleSubsystem->getPosition() >= pulseLen) {
            exampleSubsystem->backward();
            count++;
        } else if (exampleSubsystem->getPosition() <= -pulseLen) {
            exampleSubsystem->forward();
            count++;
        }
    }

    inline void end() override {
        exampleSubsystem->stop();
    }

    inline void interrupted() override {
        printf("pulse stopped\n");
        exampleSubsystem->stop();
    }

    inline bool isFinished() override {
        return count > 8;
    }

    inline CommandBase* clone() const override {
        return new Pulse(*this);
    }

private:
    ExampleSubsystem* exampleSubsystem;
    double pulseLen = 300.0;
    uint8_t count;
};

#endif
