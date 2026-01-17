/**
 * @file controller.cpp
 * @brief Implementation of controller input binding system
 * 
 * Provides the implementation for Controller, ButtonBinder, and JoystickBinder
 * classes that enable mapping controller inputs to commands.
 */
#include "main.h"
#include "custom/scheduler.h"
#include <vector>

// Static members for deferred registration
Scheduler* Controller::m_globalScheduler = nullptr;
std::vector<Controller*> Controller::m_pendingControllers;

/**
 * @brief Construct a controller and register with scheduler
 * @param id Controller ID (MASTER or PARTNER)
 * 
 * Automatically registers with scheduler if available, otherwise queues for
 * later registration when scheduler is initialized.
 */
Controller::Controller(pros::controller_id_e_t id)
    : pros::Controller(id) {
    if (m_globalScheduler != nullptr) {
        m_globalScheduler->registerController(this);
    } else {
        m_pendingControllers.push_back(this);
    }
}

/**
 * @brief Set the global scheduler reference for auto-registration
 * @param sch Pointer to scheduler instance
 * 
 * Called once during initialization to enable controller auto-registration.
 * Registers any pending controllers that were created before scheduler was available.
 */
void Controller::setScheduler(Scheduler* sch) {
    m_globalScheduler = sch;
    registerPendingControllers();
}

/**
 * @brief Register pending controllers with scheduler
 * 
 * Processes all controllers that were created before the scheduler was available,
 * registering them so their inputs will be polled.
 */
void Controller::registerPendingControllers() {
    if (m_globalScheduler) {
        for (Controller* ctrl : m_pendingControllers) {
            m_globalScheduler->registerController(ctrl);
        }
        m_pendingControllers.clear();
    }
}

/**
 * @brief Create button binders for each controller button
 * @return ButtonBinder configured for the specified button
 * 
 * These methods create ButtonBinder objects that can be configured with
 * onTrue(), onFalse(), or whileTrue() to bind commands to button events.
 */
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

/**
 * @brief Create joystick binders for each axis
 * @param threshold Minimum absolute value to trigger (0-127)
 * @return JoystickBinder configured for the specified axis
 * 
 * These methods create JoystickBinder objects that can be configured to
 * bind commands to joystick movement with threshold detection.
 */
JoystickBinder Controller::LeftJoyY(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_LEFT_Y, threshold); }
JoystickBinder Controller::LeftJoyX(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_LEFT_X, threshold); }
JoystickBinder Controller::RightJoyY(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_RIGHT_Y, threshold); }
JoystickBinder Controller::RightJoyX(int threshold) { return JoystickBinder(this, pros::E_CONTROLLER_ANALOG_RIGHT_X, threshold); }


/**
 * @brief Poll all button and joystick bindings
 * 
 * Checks all bindings for state changes and schedules/cancels commands as needed.
 * Called by scheduler each cycle.
 */
void Controller::poll() {
    for (auto& binder : m_buttonBinders) binder.poll();
    for (auto& binder : m_joystickBinders) binder.poll();
}

/**
 * @brief Construct a button binder
 * @param ctrl Parent controller
 * @param btn Button identifier
 */
ButtonBinder::ButtonBinder(Controller* ctrl, pros::controller_digital_e_t btn)
    : m_controller(ctrl), m_button(btn), km_command(nullptr), m_edge(Edge::None), m_runningCommand(nullptr) {}

/**
 * @brief Bind command to button press (rising edge)
 * @param cmd Command to schedule when button is pressed
 * @return Reference to this binder (now stored in controller's list)
 */
ButtonBinder& ButtonBinder::onTrue(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::Rising;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}
/**
 * @brief Bind command to button release (falling edge)
 * @param cmd Command to schedule when button is released
 * @return Reference to this binder (now stored in controller's list)
 */
ButtonBinder& ButtonBinder::onFalse(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::Falling;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}

/**
 * @brief Bind command to run while button held
 * @param cmd Command to run continuously while button is pressed
 * @return Reference to this binder (now stored in controller's list)
 */
ButtonBinder& ButtonBinder::whileTrue(const CommandBase* cmd) {
    km_command = cmd;
    m_edge = Edge::WhileTrue;
    m_controller->m_buttonBinders.emplace_back(*this);
    return m_controller->m_buttonBinders.back();
}

/**
 * @brief Poll button state and schedule/cancel commands as needed
 * 
 * Checks button state each cycle and handles command scheduling based on
 * configured edge type (onTrue, onFalse, whileTrue).
 */
void ButtonBinder::poll() {
    if (m_edge == Edge::Rising) {
        bool curr = m_controller->get_digital_new_press(m_button);
        if (curr && Controller::m_globalScheduler && km_command) Controller::m_globalScheduler->schedule(km_command);
    }
    if (m_edge == Edge::Falling) {
        bool curr = m_controller->get_digital_new_release(m_button);
        if (curr && Controller::m_globalScheduler && km_command) Controller::m_globalScheduler->schedule(km_command);
    }
    if (m_edge == Edge::WhileTrue) {
        bool curr = m_controller->get_digital(m_button);
        if (curr && !m_runningCommand && Controller::m_globalScheduler && km_command) {
            m_runningCommand = Controller::m_globalScheduler->schedule(km_command);
        } else if (!curr && m_runningCommand && Controller::m_globalScheduler) {
            Controller::m_globalScheduler->cancel(m_runningCommand);
            m_runningCommand = nullptr;
        }
    }
}

/**
 * @brief Construct a joystick binder
 * @param ctrl Parent controller
 * @param stick Joystick axis identifier
 * @param threshold Minimum absolute value to trigger (0-127)
 */
JoystickBinder::JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold)
    : m_controller(ctrl), m_stick(stick), m_threshold(threshold), km_command(nullptr), m_edge(Edge::None), m_prev(false), m_runningCommand(nullptr) {}

/**
 * @brief Bind command to joystick crossing threshold (rising edge)
 * @param cmd Command to schedule when joystick crosses threshold
 * @return Reference to this binder (now stored in controller's list)
 */
JoystickBinder& JoystickBinder::onTrue(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::Rising;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}
/**
 * @brief Bind command to joystick falling below threshold (falling edge)
 * @param cmd Command to schedule when joystick falls below threshold
 * @return Reference to this binder (now stored in controller's list)
 */
JoystickBinder& JoystickBinder::onFalse(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::Falling;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}

/**
 * @brief Bind command to run while joystick above threshold
 * @param cmd Command to run continuously while joystick exceeds threshold
 * @return Reference to this binder (now stored in controller's list)
 */
JoystickBinder& JoystickBinder::whileTrue(const CommandBase* cmd) {
    this->km_command = cmd;
    m_edge = Edge::WhileTrue;
    m_controller->m_joystickBinders.emplace_back(*this);
    return m_controller->m_joystickBinders.back();
}

/**
 * @brief Poll joystick position and schedule/cancel commands as needed
 * 
 * Checks joystick position each cycle against threshold and handles command
 * scheduling based on configured edge type (onTrue, onFalse, whileTrue).
 */
void JoystickBinder::poll() {
    int curr = m_controller->get_analog(m_stick);
    bool above = (m_threshold >= 0) ? (curr >= m_threshold) : (curr <= m_threshold);
    if (m_edge == Edge::Rising && above && !m_prev && Controller::m_globalScheduler && km_command) Controller::m_globalScheduler->schedule(km_command);
    if (m_edge == Edge::Falling && !above && m_prev && Controller::m_globalScheduler && km_command) Controller::m_globalScheduler->schedule(km_command);
    if (m_edge == Edge::WhileTrue) {
        if (above && !m_runningCommand && Controller::m_globalScheduler && km_command) {
            m_runningCommand = Controller::m_globalScheduler->schedule(km_command);
        } else if (!above && m_runningCommand && Controller::m_globalScheduler) {
            Controller::m_globalScheduler->cancel(m_runningCommand);
            m_runningCommand = nullptr;
        }
    }
    m_prev = above;
}
