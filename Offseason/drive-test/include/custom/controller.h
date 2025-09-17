#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include "main.h"
#include <array>
#include <typeinfo>
#include <vector>

class ButtonBinder;
class JoystickBinder;

class Controller : public pros::Controller {
public:
    Controller(pros::controller_id_e_t id, std::vector<std::unique_ptr<CommandBase>>* sch)
        : pros::Controller(id), scheduler(sch) {
        prevButtonStates.fill(false);
    }

    inline CommandBase* addCommand(const CommandBase* cmd) { 
        // Check for conflicts with existing commands
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        if (!requiredSubsystems.empty()) {
            // Check for existing commands using any of the same subsystems
            for (auto it = scheduler->begin(); it != scheduler->end(); ) {
                const auto& existingSubsystems = (*it)->getRequiredSubsystems();
                bool conflict = false;
                
                // Check if there's any overlap in subsystems
                for (SubsystemBase* newSub : requiredSubsystems) {
                    for (SubsystemBase* existingSub : existingSubsystems) {
                        if (newSub == existingSub) {
                            conflict = true;
                            break;
                        }
                    }
                    if (conflict) break;
                }
                
                if (conflict) {
                    // Found conflict - interrupt the existing command
                    (*it)->interrupted();
                    it = scheduler->erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        // Add the new command and return pointer to the running instance
        auto clonedCmd = std::unique_ptr<CommandBase>(cmd->clone());
        CommandBase* runningInstance = clonedCmd.get();
        scheduler->push_back(std::move(clonedCmd));
        return runningInstance;
    }

    void cancelCommand(CommandBase* commandInstance) {
        if (!commandInstance) return;
        
        // Find and cancel the specific command instance
        for (auto it = scheduler->begin(); it != scheduler->end(); ++it) {
            if (it->get() == commandInstance) {
                (*it)->end();  // Call end() for normal whileTrue termination
                scheduler->erase(it);
                break;
            }
        }
    }

    // Register a subsystem for periodic updates
    inline void registerSubsystemForPeriodic(SubsystemBase* subsystem) {
        if (subsystem != nullptr) {
            // Check if already in the list
            for (SubsystemBase* existing : periodicSubsystems) {
                if (existing == subsystem) {
                    return; // Already registered
                }
            }
            periodicSubsystems.push_back(subsystem);
        }
    }

    // Update all registered subsystems
    inline void updateSubsystems() {
        for (SubsystemBase* subsystem : periodicSubsystems) {
            subsystem->periodic();
        }
    }

    inline ButtonBinder setButtonCommand();

    inline JoystickBinder setJoystickCommand();

    inline void poll();

    std::array<bool, 12> prevButtonStates;
    std::vector<ButtonBinder> buttonBinders;
    std::vector<JoystickBinder> joystickBinders;

private:
    std::vector<std::unique_ptr<CommandBase>>* scheduler;
    std::vector<SubsystemBase*> periodicSubsystems;
};

class ButtonBinder {
public:
    enum class Edge { None, Rising, Falling, WhileTrue };
    ButtonBinder(Controller* ctrl) : controller(ctrl), edge(Edge::None), runningCommand(nullptr) {}

    ButtonBinder& onTrue(pros::controller_digital_e_t btn, const CommandBase* cmd) {
        button = btn;
        command = cmd;
        edge = Edge::Rising;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->buttonBinders.emplace_back(*this);
        return controller->buttonBinders.back();
    }
    ButtonBinder& onFalse(pros::controller_digital_e_t btn, const CommandBase* cmd) {
        button = btn;
        command = cmd;
        edge = Edge::Falling;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->buttonBinders.emplace_back(*this);
        return controller->buttonBinders.back();
    }
    
    ButtonBinder& whileTrue(pros::controller_digital_e_t btn, const CommandBase* cmd) {
        button = btn;
        command = cmd;
        edge = Edge::WhileTrue;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->buttonBinders.emplace_back(*this);
        return controller->buttonBinders.back();
    }

    void poll() {
        if (edge == Edge::Rising)  { bool curr = controller->get_digital_new_press(button);    if (curr) controller->addCommand(command); }
        if (edge == Edge::Falling) { bool curr = controller->get_digital_new_release(button);  if (curr) controller->addCommand(command); }
        if (edge == Edge::WhileTrue) { 
            bool curr = controller->get_digital(button); 
            if (curr && !runningCommand) {
                // Button pressed - start the command and store reference
                runningCommand = controller->addCommand(command);
            } else if (!curr && runningCommand) {
                // Button released - cancel the specific command instance
                controller->cancelCommand(runningCommand);
                runningCommand = nullptr;
            }
        }
    }

private:
    Controller* controller;
    pros::controller_digital_e_t button;
    const CommandBase* command;
    Edge edge;
    CommandBase* runningCommand;  // Track the specific running command instance
};

class JoystickBinder {
public:
    enum class Edge { None, Rising, Falling, WhileTrue };
    JoystickBinder(Controller* ctrl) : controller(ctrl), edge(Edge::None), prev(false), runningCommand(nullptr) {}

    JoystickBinder& onTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
        this->stick = stick;
        this->threshold = threshold;
        this->command = cmd;
        edge = Edge::Rising;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->joystickBinders.emplace_back(*this);
        return controller->joystickBinders.back();
    }
    JoystickBinder& onFalse(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
        this->stick = stick;
        this->threshold = threshold;
        this->command = cmd;
        edge = Edge::Falling;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->joystickBinders.emplace_back(*this);
        return controller->joystickBinders.back();
    }
    
    JoystickBinder& whileTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
        this->stick = stick;
        this->threshold = threshold;
        this->command = cmd;
        edge = Edge::WhileTrue;
        
        // Register all subsystems from this command immediately
        const auto& requiredSubsystems = cmd->getRequiredSubsystems();
        for (SubsystemBase* subsystem : requiredSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        const auto& usedSubsystems = cmd->getUsedSubsystems();
        for (SubsystemBase* subsystem : usedSubsystems) {
            controller->registerSubsystemForPeriodic(subsystem);
        }
        
        controller->joystickBinders.emplace_back(*this);
        return controller->joystickBinders.back();
    }

    void poll() {
        int curr = controller->get_analog(stick);
        bool above = (threshold >= 0) ? (curr >= threshold) : (curr <= threshold);
        if (edge == Edge::Rising && above && !prev) controller->addCommand(command);
        if (edge == Edge::Falling && !above && prev) controller->addCommand(command);
        if (edge == Edge::WhileTrue) {
            if (above && !runningCommand) {
                // Threshold exceeded - start the command and store reference
                runningCommand = controller->addCommand(command);
            } else if (!above && runningCommand) {
                // Below threshold - cancel the specific command instance
                controller->cancelCommand(runningCommand);
                runningCommand = nullptr;
            }
        }
        prev = above;
    }

private:
    Controller* controller;
    pros::controller_analog_e_t stick;
    int threshold;
    const CommandBase* command;
    bool prev;
    Edge edge;
    CommandBase* runningCommand;  // Track the specific running command instance
};

inline ButtonBinder Controller::setButtonCommand() {
    return ButtonBinder(this);
}

inline JoystickBinder Controller::setJoystickCommand() {
    return JoystickBinder(this);
}

inline void Controller::poll() {
    for (auto& binder : buttonBinders) {
        binder.poll();
    }
    for (auto& binder : joystickBinders) {
        binder.poll();
    }
}

#endif
