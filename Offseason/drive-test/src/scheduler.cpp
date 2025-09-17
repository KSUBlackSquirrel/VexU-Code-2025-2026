// Scheduler.cpp
// Implements the Scheduler class for command scheduling, execution, and subsystem updates.
// Handles the main command queue and periodic subsystem management.
#include "main.h"

// Scheduler constructor: initializes the scheduler
Scheduler::Scheduler() = default;

// Register a controller for polling (button/joystick events)
void Scheduler::registerController(Controller* ctrl) {
    controllers.push_back(ctrl);
}

// Poll all registered controllers to process input events
void Scheduler::pollControllers() {
    for (Controller* ctrl : controllers) {
        ctrl->poll();
    }
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

// Call periodic() on all registered subsystems
void Scheduler::updateSubsystems() {
    for (SubsystemBase* subsystem : periodicSubsystems) {
        subsystem->periodic();
    }
}

// Run one scheduler tick: execute, finish, cleanup commands
void Scheduler::tick() {
    for (auto it = queue.begin(); it != queue.end();) {
        (*it)->execute();
        if ((*it)->isFinished()) {
            (*it)->end();
            it = queue.erase(it);
        } else {
            ++it;
        }
    }
}

// Utility helpers for queue size and empty check
std::size_t Scheduler::size() const { return queue.size(); }
bool Scheduler::empty() const { return queue.empty(); }
