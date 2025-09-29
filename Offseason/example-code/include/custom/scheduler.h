#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

class Controller;
#include "custom/command/commandBase.h"
#include "custom/subsystem/subsystemBase.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

class Scheduler {
public:
    Scheduler();
    void registerController(Controller* ctrl);
    CommandBase* addCommand(const CommandBase* cmd);
    void setDefaultCommand(CommandBase* command);
    void cancelCommand(CommandBase* commandInstance);
    void registerSubsystemForPeriodic(SubsystemBase* subsystem);
    void run();

    
    std::size_t size() const;
    std::size_t defaultSize() const;
    bool empty() const;

private:
    // Individual scheduler tick steps
    void step1_runSubsystemPeriodicMethods();
    void step2_pollCommandSchedulingTriggers();
    void step3_runAndFinishScheduledCommands();
    void step4_scheduleDefaultCommands();
    
    std::vector<std::unique_ptr<CommandBase>> queue;
    std::unordered_map<SubsystemBase*, CommandBase*> defaultCommands;
    std::vector<SubsystemBase*> periodicSubsystems;
    std::vector<Controller*> controllers;
};

#endif // SCHEDULER_HPP_
