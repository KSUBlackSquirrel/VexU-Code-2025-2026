#include "main.h"
#include "custom/scheduler.h"
#include <vector>

// Static members for backup registration
Scheduler* Controller::m_globalScheduler = nullptr;
std::vector<Controller*> Controller::m_pendingControllers;

// Controller constructor: initializes the controller, binds it to a scheduler, and auto-registers with the scheduler.
Controller::Controller(pros::controller_id_e_t id)
    : pros::Controller(id) {
    if (m_scheduler) {
        m_scheduler->registerController(this);
    } else if (m_globalScheduler) {
        m_globalScheduler->registerController(this);
    } else {
        m_pendingControllers.push_back(this);
    }
}

// Set the global scheduler reference for auto-registration
void Controller::setScheduler(Scheduler* sch) {
    m_globalScheduler = sch;
    registerPendingControllers();
}

// Register any controllers created before scheduler was set
void Controller::registerPendingControllers() {
    if (m_globalScheduler) {
        for (Controller* ctrl : m_pendingControllers) {
            m_globalScheduler->registerController(ctrl);
        }
        m_pendingControllers.clear();
    }
}

// Create a new ButtonBinder for every available button on this controller
ButtonBinder Controller::A() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_A); }
ButtonBinder Controller::B() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_B); }
ButtonBinder Controller::X() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_X); }
ButtonBinder Controller::Y() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_Y); }
ButtonBinder Controller::RIGHT() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_RIGHT); }
ButtonBinder Controller::DOWN() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_DOWN); }
ButtonBinder Controller::UP() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_UP); }
ButtonBinder Controller::LEFT() { return ButtonBinder(this, pros::E_CONTROLLER_DIGITAL_LEFT); }
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
    for (auto& binder : m_buttonBinders) binder.poll();
    for (auto& binder : m_joystickBinders) binder.poll();
}

// ButtonBinder constructor: binds to a controller
ButtonBinder::ButtonBinder(Controller* ctrl, pros::controller_digital_e_t btn)
    : m_controller(ctrl), m_button(btn), km_command(nullptr), m_edge(Edge::None), m_runningCommand(nullptr) {}

// Bind a command to button movement (rising edge, falling edge, or while held)
ButtonBinder& ButtonBinder::onTrue(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::Rising;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}
ButtonBinder& ButtonBinder::onFalse(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::Falling;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}
ButtonBinder& ButtonBinder::whileTrue(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::WhileTrue;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}

// Poll the button for events and schedule/cancel commands as needed
void ButtonBinder::poll() {
    if (m_edge == Edge::Rising) {
        bool curr = m_controller->get_digital_new_press(m_button);
        if (curr) m_controller->m_scheduler->schedule(km_command);
    }
    if (m_edge == Edge::Falling) {
        bool curr = m_controller->get_digital_new_release(m_button);
        if (curr) m_controller->m_scheduler->schedule(km_command);
    }
    if (m_edge == Edge::WhileTrue) {
        bool curr = m_controller->get_digital(m_button);
        if (curr && !m_runningCommand) {
            m_runningCommand = m_controller->m_scheduler->schedule(km_command);
        } else if (!curr && m_runningCommand) {
            m_controller->m_scheduler->cancel(m_runningCommand);
            m_runningCommand = nullptr;
        }
    }
}

// JoystickBinder constructor: binds to a controller
JoystickBinder::JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold)
    : m_controller(ctrl), m_stick(stick), m_threshold(threshold), km_command(nullptr), m_edge(Edge::None), m_prev(false), m_runningCommand(nullptr) {}

// Bind a command to joystick movement (rising edge, falling edge, or while held)
JoystickBinder& JoystickBinder::onTrue(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::Rising;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}
JoystickBinder& JoystickBinder::onFalse(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::Falling;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}
JoystickBinder& JoystickBinder::whileTrue(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::WhileTrue;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}

// Poll the joystick for events and schedule/cancel commands as needed
void JoystickBinder::poll() {
    int curr = m_controller->get_analog(m_stick);
    bool above = (m_threshold >= 0) ? (curr >= m_threshold) : (curr <= m_threshold);
    if (m_edge == Edge::Rising && above && !m_prev) m_controller->m_scheduler->schedule(km_command);
    if (m_edge == Edge::Falling && !above && m_prev) m_controller->m_scheduler->schedule(km_command);
    if (m_edge == Edge::WhileTrue) {
        if (above && !m_runningCommand) {
            m_runningCommand = m_controller->m_scheduler->schedule(km_command);
        } else if (!above && m_runningCommand) {
            m_controller->m_scheduler->cancel(m_runningCommand);
            m_runningCommand = nullptr;
        }
    }
    m_prev = above;
}
