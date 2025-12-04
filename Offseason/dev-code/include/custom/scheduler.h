#ifndef SCHEDULER_H_
#define SCHEDULER_H_

class Controller;
#include "custom/command/commandBase.h"
#include "custom/subsystem/subsystemBase.h"
#include "custom/watchdog.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>

class Scheduler {
public:
    // Singleton pattern
    static Scheduler& getInstance();
    
    // Core scheduling methods
    void registerController(Controller* ctrl);
    CommandBase* schedule(const CommandBase* cmd);
    void setDefaultCommand(SubsystemBase* sub, CommandBase* cmd);
    void cancel(CommandBase* commandInstance);
    void cancelAll();
    void registerSubsystemForPeriodic(SubsystemBase* sub);
    void run();
    
    // Scheduler state management
    void enable();
    void disable();
    bool isEnabled() const;
    
    // Command state queries
    bool isScheduled(const CommandBase* cmd) const;
    CommandBase* requiring(SubsystemBase* subsystem) const;
    
    // Command event callbacks
    void onCommandInitialize(std::function<void(CommandBase*)> action);
    void onCommandExecute(std::function<void(CommandBase*)> action);
    void onCommandFinish(std::function<void(CommandBase*)> action);
    void onCommandInterrupt(std::function<void(CommandBase*)> action);
    
    // Robot state awareness
    void setRobotEnabled(bool enabled);
    bool isRobotEnabled() const;
    
    // Utility methods
    std::size_t size() const;
    std::size_t defaultSize() const;
    bool empty() const;

private:
    // Private constructor for singleton
    Scheduler();
    
    // Internal helper methods
    void initCommand(CommandBase* command, const std::unordered_set<SubsystemBase*>& requirements);
    void interruptCommand(CommandBase* command);
    void finishCommand(CommandBase* command);
    bool requirementsFree(const std::unordered_set<SubsystemBase*>& requirements) const;
    bool areCommandsInterruptible(const std::unordered_set<SubsystemBase*>& requirements) const;

    // Scheduler state
    bool m_enabled = false;
    bool m_robotEnabled = false;
    bool m_inRunLoop = false;
    
    // Command storage
    std::vector<std::unique_ptr<CommandBase>> m_queue;
    std::unordered_map<SubsystemBase*, CommandBase*> m_defaultCommands;
    std::unordered_map<SubsystemBase*, CommandBase*> m_requirements;
    std::vector<SubsystemBase*> m_periodicSubsystems;
    std::vector<Controller*> m_controllers;
    
    // Commands to schedule during run loop
    std::vector<CommandBase*> m_toSchedule;
    
    // Event callbacks
    std::vector<std::function<void(CommandBase*)>> m_initActions;
    std::vector<std::function<void(CommandBase*)>> m_executeActions;
    std::vector<std::function<void(CommandBase*)>> m_finishActions;
    std::vector<std::function<void(CommandBase*)>> m_interruptActions;
    
    // Performance monitoring
    Watchdog m_watchdog;
};

#endif // SCHEDULER_H_
