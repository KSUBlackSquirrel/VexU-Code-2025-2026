#include "main.h"


// Controller constructor: initializes the controller and binds it to a scheduler.
Controller::Controller(pros::controller_id_e_t id, Scheduler* sch)
    : pros::Controller(id), scheduler(sch) {
    prevButtonStates.fill(false);
}

// Create a new ButtonBinder for every available button on this controller
ButtonBinder Controller::A() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_A); }
ButtonBinder Controller::B() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_B); }
ButtonBinder Controller::X() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_X); }
ButtonBinder Controller::Y() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_Y); }
ButtonBinder Controller::Right() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_RIGHT); }
ButtonBinder Controller::Down() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_DOWN); }
ButtonBinder Controller::Up() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_UP); }
ButtonBinder Controller::Left() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_LEFT); }
ButtonBinder Controller::L1() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_L1); }
ButtonBinder Controller::L2() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_L2); }
ButtonBinder Controller::R1() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_R1); }
ButtonBinder Controller::R2() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_R2); }

// Create a new JoystickBinder for each joystick on this controller
JoystickBinder Controller::LeftJoyY(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_LEFT_Y, threshold); }
JoystickBinder Controller::LeftJoyX(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_LEFT_X, threshold); }
JoystickBinder Controller::RightJoyY(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_RIGHT_Y, threshold); }
JoystickBinder Controller::RightJoyX(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_RIGHT_X, threshold); }


// Poll all binders to check for input events and schedule/cancel commands
void Controller::poll() {
    for (auto& binder : buttonBinders) binder.poll();
    for (auto& binder : joystickBinders) binder.poll();
}

// ButtonBinder constructor: binds to a controller
ButtonBinder::ButtonBinder(Controller* ctrl, pros::controller_digital_e_t btn)
    : controller(ctrl), button(btn), command(nullptr), edge(Edge::None), runningCommand(nullptr) {}

// Bind a command to button movement (rising edge, falling edge, or while held)
ButtonBinder& ButtonBinder::onTrue(const CommandBase* cmd) {
    command = cmd;
    edge = Edge::Rising;
    controller->buttonBinders.emplace_back(*this);
    return controller->buttonBinders.back();
}
ButtonBinder& ButtonBinder::onFalse(const CommandBase* cmd) {
    command = cmd;
    edge = Edge::Falling;
    controller->buttonBinders.emplace_back(*this);
    return controller->buttonBinders.back();
}
ButtonBinder& ButtonBinder::whileTrue(const CommandBase* cmd) {
    command = cmd;
    edge = Edge::WhileTrue;
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

// JoystickBinder constructor: binds to a controller
JoystickBinder::JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold)
    : controller(ctrl), stick(stick), threshold(threshold), command(nullptr), edge(Edge::None), prev(false), runningCommand(nullptr) {}

// Bind a command to joystick movement (rising edge, falling edge, or while held)
JoystickBinder& JoystickBinder::onTrue(const CommandBase* cmd) {
    this->command = cmd;
    edge = Edge::Rising;
    controller->joystickBinders.emplace_back(*this);
    return controller->joystickBinders.back();
}
JoystickBinder& JoystickBinder::onFalse(const CommandBase* cmd) {
    this->command = cmd;
    edge = Edge::Falling;
    controller->joystickBinders.emplace_back(*this);
    return controller->joystickBinders.back();
}
JoystickBinder& JoystickBinder::whileTrue(const CommandBase* cmd) {
    this->command = cmd;
    edge = Edge::WhileTrue;
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
