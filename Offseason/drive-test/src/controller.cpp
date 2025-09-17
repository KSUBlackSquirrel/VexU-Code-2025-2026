// Controller.cpp
// Implements the Controller class and its binder helpers for button/joystick command scheduling.
// This file handles polling controller inputs and scheduling commands via the Scheduler.
#include "main.h"


// Controller constructor: initializes the controller and binds it to a scheduler.
Controller::Controller(pros::controller_id_e_t id, Scheduler* sch)
    : pros::Controller(id), scheduler(sch) {
    prevButtonStates.fill(false);
}


// Create a new ButtonBinder for this controller
ButtonBinder Controller::setButtonCommand() {
    return ButtonBinder(this);
}

// Create a new JoystickBinder for this controller
JoystickBinder Controller::setJoystickCommand() {
    return JoystickBinder(this);
}

// Poll all binders to check for input events and schedule/cancel commands
void Controller::poll() {
    for (auto& binder : buttonBinders) binder.poll();
    for (auto& binder : joystickBinders) binder.poll();
}

// ButtonBinder constructor: binds to a controller
ButtonBinder::ButtonBinder(Controller* ctrl)
    : controller(ctrl), edge(Edge::None), runningCommand(nullptr) {}

// Bind a command to a button press (rising edge)
ButtonBinder& ButtonBinder::onTrue(pros::controller_digital_e_t btn, const CommandBase* cmd) {
    button = btn;
    command = cmd;
    edge = Edge::Rising;
    registerSubsystems(cmd);
    controller->buttonBinders.emplace_back(*this);
    return controller->buttonBinders.back();
}

// Bind a command to a button release (falling edge)
ButtonBinder& ButtonBinder::onFalse(pros::controller_digital_e_t btn, const CommandBase* cmd) {
    button = btn;
    command = cmd;
    edge = Edge::Falling;
    registerSubsystems(cmd);
    controller->buttonBinders.emplace_back(*this);
    return controller->buttonBinders.back();
}

// Bind a command to while a button is held
ButtonBinder& ButtonBinder::whileTrue(pros::controller_digital_e_t btn, const CommandBase* cmd) {
    button = btn;
    command = cmd;
    edge = Edge::WhileTrue;
    registerSubsystems(cmd);
    controller->buttonBinders.emplace_back(*this);
    return controller->buttonBinders.back();
}

// Poll the button for events and schedule/cancel commands as needed
void ButtonBinder::poll() {
    if (edge == Edge::Rising) {
        bool curr = controller->get_digital_new_press(button);
        if (curr) controller->scheduler->addCommand(command);
    }
    if (edge == Edge::Falling) {
        bool curr = controller->get_digital_new_release(button);
        if (curr) controller->scheduler->addCommand(command);
    }
    if (edge == Edge::WhileTrue) {
        bool curr = controller->get_digital(button);
        if (curr && !runningCommand) {
            runningCommand = controller->scheduler->addCommand(command);
        } else if (!curr && runningCommand) {
            controller->scheduler->cancelCommand(runningCommand);
            runningCommand = nullptr;
        }
    }
}

// Register all subsystems required/used by this command for periodic updates
void ButtonBinder::registerSubsystems(const CommandBase* cmd) {
    const auto& requiredSubsystems = cmd->getRequiredSubsystems();
    for (SubsystemBase* subsystem : requiredSubsystems) {
        controller->scheduler->registerSubsystemForPeriodic(subsystem);
    }
    const auto& usedSubsystems = cmd->getUsedSubsystems();
    for (SubsystemBase* subsystem : usedSubsystems) {
        controller->scheduler->registerSubsystemForPeriodic(subsystem);
    }
}

// JoystickBinder constructor: binds to a controller
JoystickBinder::JoystickBinder(Controller* ctrl)
    : controller(ctrl), edge(Edge::None), prev(false), runningCommand(nullptr) {}

// Bind a command to joystick movement (rising edge, falling edge, or while held)
JoystickBinder& JoystickBinder::onTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
    this->stick = stick;
    this->threshold = threshold;
    this->command = cmd;
    edge = Edge::Rising;
    registerSubsystems(cmd);
    controller->joystickBinders.emplace_back(*this);
    return controller->joystickBinders.back();
}

JoystickBinder& JoystickBinder::onFalse(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
    this->stick = stick;
    this->threshold = threshold;
    this->command = cmd;
    edge = Edge::Falling;
    registerSubsystems(cmd);
    controller->joystickBinders.emplace_back(*this);
    return controller->joystickBinders.back();
}

JoystickBinder& JoystickBinder::whileTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd) {
    this->stick = stick;
    this->threshold = threshold;
    this->command = cmd;
    edge = Edge::WhileTrue;
    registerSubsystems(cmd);
    controller->joystickBinders.emplace_back(*this);
    return controller->joystickBinders.back();
}

// Poll the joystick for events and schedule/cancel commands as needed
void JoystickBinder::poll() {
    int curr = controller->get_analog(stick);
    bool above = (threshold >= 0) ? (curr >= threshold) : (curr <= threshold);
    if (edge == Edge::Rising && above && !prev) controller->scheduler->addCommand(command);
    if (edge == Edge::Falling && !above && prev) controller->scheduler->addCommand(command);
    if (edge == Edge::WhileTrue) {
        if (above && !runningCommand) {
            runningCommand = controller->scheduler->addCommand(command);
        } else if (!above && runningCommand) {
            controller->scheduler->cancelCommand(runningCommand);
            runningCommand = nullptr;
        }
    }
    prev = above;
}

// Register all subsystems required/used by this command for periodic updates
void JoystickBinder::registerSubsystems(const CommandBase* cmd) {
    const auto& requiredSubsystems = cmd->getRequiredSubsystems();
    for (SubsystemBase* subsystem : requiredSubsystems) {
        controller->scheduler->registerSubsystemForPeriodic(subsystem);
    }
    const auto& usedSubsystems = cmd->getUsedSubsystems();
    for (SubsystemBase* subsystem : usedSubsystems) {
        controller->scheduler->registerSubsystemForPeriodic(subsystem);
    }
}
