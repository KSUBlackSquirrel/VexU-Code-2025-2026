// Scheduler.cpp
// Implements the Scheduler class for command scheduling, execution, and subsystem updates.
// Handles the main command queue, periodic subsystem management, and default commands.
#include "main.h"

// Scheduler constructor: initializes the scheduler
Scheduler::Scheduler() = default;

// Register a controller for polling (button/joystick events)
void Scheduler::registerController(Controller* ctrl) {
    controllers.push_back(ctrl);
}

// Add a command to the scheduler queue, handling subsystem conflicts
CommandBase* Scheduler::addCommand(const CommandBase* cmd) {
    const auto& requiredSubsystems = cmd->getRequiredSubsystems();
    if (!requiredSubsystems.empty()) {
        for (auto it = queue.begin(); it != queue.end();) {
            const auto& existingSubsystems = (*it)->getRequiredSubsystems();
            bool conflict = false;
            for (SubsystemBase* newSub : requiredSubsystems) {
                for (SubsystemBase* existingSub : existingSubsystems) {
                    if (newSub == existingSub) { conflict = true; break; }
                }
                if (conflict) break;
            }
            if (conflict) {
                (*it)->interrupted();
                it = queue.erase(it);
            } else {
                ++it;
            }
        }
    }
    auto clonedCmd = std::unique_ptr<CommandBase>(cmd->clone());
    CommandBase* runningInstance = clonedCmd.get();
    queue.push_back(std::move(clonedCmd));
    return runningInstance;
}

// Cancel a specific command instance in the queue
void Scheduler::cancelCommand(CommandBase* commandInstance) {
    if (!commandInstance) return;
    for (auto it = queue.begin(); it != queue.end(); ++it) {
        if (it->get() == commandInstance) {
            (*it)->end();
            queue.erase(it);
            break;
        }
    }
}

// Register a subsystem for periodic updates (called every loop)
void Scheduler::registerSubsystemForPeriodic(SubsystemBase* subsystem) {
    if (subsystem == nullptr) return;
    for (SubsystemBase* existing : periodicSubsystems) {
        if (existing == subsystem) return;
    }
    periodicSubsystems.push_back(subsystem);
}

// Set a default command for all subsystems affected by the command (required and used).
void Scheduler::setDefaultCommand(CommandBase* command) {
    for (SubsystemBase* subsystem : command->getRequiredSubsystems()) {
        defaultCommands[subsystem] = command;
    }
}

// Run one scheduler tick: 
// Step 1: Run Subsystem Periodic Methods
// Step 2: Poll Command Scheduling Triggers
// Step 3: Run/Finish Scheduled Commands
// Step 4: Schedule Default Commands
void Scheduler::run() {
    step1_runSubsystemPeriodicMethods();
    step2_pollCommandSchedulingTriggers();
    step3_runAndFinishScheduledCommands();
    step4_scheduleDefaultCommands();
}

// Step 1: Run Subsystem Periodic Methods
void Scheduler::step1_runSubsystemPeriodicMethods() {
    for (SubsystemBase* subsystem : periodicSubsystems) {
        subsystem->periodic();
    }
}

// Step 2: Poll Command Scheduling Triggers
void Scheduler::step2_pollCommandSchedulingTriggers() {
    for (Controller* ctrl : controllers) {
        ctrl->poll();
    }
}

// Step 3: Run/Finish Scheduled Commands
void Scheduler::step3_runAndFinishScheduledCommands() {
    // Track which subsystems are currently in use by scheduled commands
    std::unordered_set<SubsystemBase*> busySubsystems;
    for (const auto& cmdPtr : queue) {
        if (!cmdPtr) continue;
        for (SubsystemBase* sub : cmdPtr->getRequiredSubsystems()) {
            busySubsystems.insert(sub);
        }
    }

    // Remove default commands for busy subsystems and execute all commands
    for (auto it = queue.begin(); it != queue.end();) {
        if (!(*it)) {
            it = queue.erase(it);
            continue;
        }
        // If this is a default command and its subsystem is now busy, end and remove it
        for (auto& [subsystem, defaultCmd] : defaultCommands) {
            if ((*it).get() == defaultCmd && busySubsystems.count(subsystem)) {
                (*it)->end();
                it = queue.erase(it);
                goto next_cmd;
            }
        }
        (*it)->execute();
        if ((*it)->isFinished()) {
            (*it)->end();
            it = queue.erase(it);
        } else {
            ++it;
        }
    next_cmd:;
    }
}

// Step 4: Schedule Default Commands
void Scheduler::step4_scheduleDefaultCommands() {
    // Track which subsystems are currently in use by scheduled commands
    std::unordered_set<SubsystemBase*> busySubsystems;
    for (const auto& cmdPtr : queue) {
        if (!cmdPtr) continue;
        for (SubsystemBase* sub : cmdPtr->getRequiredSubsystems()) {
            busySubsystems.insert(sub);
        }
    }

    // Add default commands for subsystems that are not busy and not already scheduled
    for (auto& [subsystem, defaultCmd] : defaultCommands) {
        if (!busySubsystems.count(subsystem)) {
            bool alreadyScheduled = false;
            for (const auto& cmdPtr : queue) {
                if (cmdPtr && cmdPtr.get() == defaultCmd) {
                    alreadyScheduled = true;
                    break;
                }
            }
            if (!alreadyScheduled) {
                auto clonedCmd = std::unique_ptr<CommandBase>(defaultCmd->clone());
                queue.push_back(std::move(clonedCmd));
            }
        }
    }
}

std::size_t Scheduler::size() const { return queue.size(); }
std::size_t Scheduler::defaultSize() const { return defaultCommands.size(); }
bool Scheduler::empty() const { return queue.empty(); }
