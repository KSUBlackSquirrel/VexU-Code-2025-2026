#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

class Controller;
#include "custom/command/commandBase.h"
#include "custom/subsystem/subsystemBase.h"
#include <vector>
#include <memory>

class Scheduler {
public:
    Scheduler();
    void registerController(Controller* ctrl);
    void pollControllers();
    CommandBase* addCommand(const CommandBase* cmd);
    void cancelCommand(CommandBase* commandInstance);
    void registerSubsystemForPeriodic(SubsystemBase* subsystem);
    void updateSubsystems();
    void tick();
    std::size_t size() const;
    bool empty() const;

private:
    std::vector<std::unique_ptr<CommandBase>> queue;
    std::vector<SubsystemBase*> periodicSubsystems;
    std::vector<Controller*> controllers;
};

#endif // SCHEDULER_HPP_
