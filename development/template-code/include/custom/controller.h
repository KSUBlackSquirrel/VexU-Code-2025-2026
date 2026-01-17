/**
 * @file controller.h
 * @brief Controller input binding system for command scheduling
 * 
 * This file provides the Controller class and binding interfaces for mapping
 * controller buttons and joysticks to commands. When buttons are pressed or
 * joysticks moved, bound commands are automatically scheduled.
 * 
 * Key concepts:
 * - Controller extends pros::Controller with command binding capabilities
 * - ButtonBinder maps button presses to commands (onTrue, onFalse, whileTrue)
 * - JoystickBinder maps joystick movement to commands with threshold detection
 * - All bindings are configured in configureBindings() in robot.cpp
 * 
 * DO NOT modify this file unless you know what you are doing.
 */
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

class Scheduler;
#include "custom/command/commandBase.h"
#include "custom/subsystem/subsystemBase.h"
#include <array>
#include <vector>

class ButtonBinder;
class JoystickBinder;

/**
 * @brief Extended controller with command binding support
 * 
 * Extends pros::Controller to add command scheduling capabilities. Provides
 * methods to bind buttons and joysticks to commands, which are then automatically
 * scheduled when inputs change.
 * Example: controller.A().onTrue(myCommand.get()) schedules command when A pressed
 */
class Controller : public pros::Controller {
    // Friend declarations for classes that need access to protected methods
    friend class Scheduler;
    
public:
    /**
     * @brief Construct a controller and register with scheduler
     * @param id Controller ID (MASTER or PARTNER)
     */
    Controller(pros::controller_id_e_t id);
    
    // Button binding methods - return ButtonBinder for fluent configuration
    ButtonBinder A();       ///< Bind to A button
    ButtonBinder B();       ///< Bind to B button
    ButtonBinder X();       ///< Bind to X button
    ButtonBinder Y();       ///< Bind to Y button
    ButtonBinder RIGHT();   ///< Bind to RIGHT D-pad
    ButtonBinder DOWN();    ///< Bind to DOWN D-pad
    ButtonBinder UP();      ///< Bind to UP D-pad
    ButtonBinder LEFT();    ///< Bind to LEFT D-pad
    ButtonBinder L1();      ///< Bind to L1 shoulder button
    ButtonBinder L2();      ///< Bind to L2 shoulder button
    ButtonBinder R1();      ///< Bind to R1 shoulder button
    ButtonBinder R2();      ///< Bind to R2 shoulder button

    /**
     * @brief Bind to left joystick Y-axis (forward/back)
     * @param threshold Minimum absolute value to trigger (0-127)
     * @return JoystickBinder for configuration
     */
    JoystickBinder LeftJoyY(int threshold);
    
    /**
     * @brief Bind to left joystick X-axis (left/right)
     * @param threshold Minimum absolute value to trigger (0-127)
     * @return JoystickBinder for configuration
     */
    JoystickBinder LeftJoyX(int threshold);
    
    /**
     * @brief Bind to right joystick Y-axis (forward/back)
     * @param threshold Minimum absolute value to trigger (0-127)
     * @return JoystickBinder for configuration
     */
    JoystickBinder RightJoyY(int threshold);
    
    /**
     * @brief Bind to right joystick X-axis (left/right)
     * @param threshold Minimum absolute value to trigger (0-127)
     * @return JoystickBinder for configuration
     */
    JoystickBinder RightJoyX(int threshold);

    /**
     * @brief Set the global scheduler reference for auto-registration
     * @param sch Pointer to scheduler instance
     * 
     * Called during initialization. Do not call manually - it's handled in main.cpp.
     */
    static void setScheduler(Scheduler* sch);

    std::vector<ButtonBinder> m_buttonBinders;      ///< All button bindings
    std::vector<JoystickBinder> m_joystickBinders;  ///< All joystick bindings

    static Scheduler* m_globalScheduler;                ///< Global scheduler reference
    static std::vector<Controller*> m_pendingControllers;  ///< Controllers awaiting registration

protected:
    // ========================================================================
    // INTERNAL METHODS
    // ========================================================================
    // These methods are used internally by the framework. Do not call manually.
    
    /**
     * @brief Check all bindings and schedule/cancel commands as needed
     */
    void poll();
    
    /**
     * @brief Register pending controllers with scheduler
     */
    static void registerPendingControllers();

};

/**
 * @brief Edge detection types for button/joystick triggers
 * 
 * Determines when bound commands are scheduled based on input state changes.
 */
enum class Edge { 
    None,       ///< No binding configured
    Rising,     ///< Trigger when input goes from off to on
    Falling,    ///< Trigger when input goes from on to off
    WhileTrue   ///< Continuously run while input is on
};

/**
 * @brief Button binding interface for mapping buttons to commands
 * 
 * Provides methods to bind commands to button press events. Created by
 * Controller button methods (A(), B(), etc.) and configured with onTrue(),
 * onFalse(), or whileTrue().
 * Examples: controller.A().onTrue(cmd), controller.B().whileTrue(cmd)
 */
class ButtonBinder {
public:
    /**
     * @brief Construct a button binder
     * @param ctrl Parent controller
     * @param btn Button identifier
     */
    ButtonBinder(Controller* ctrl, pros::controller_digital_e_t btn);
    
    /**
     * @brief Schedule command when button is pressed (rising edge)
     * @param cmd Command to schedule
     * @return Reference to this binder for chaining
     */
    ButtonBinder& onTrue(const CommandBase* cmd);
    
    /**
     * @brief Schedule command when button is released (falling edge)
     * @param cmd Command to schedule
     * @return Reference to this binder for chaining
     */
    ButtonBinder& onFalse(const CommandBase* cmd);
    
    /**
     * @brief Run command while button is held, cancel when released
     * @param cmd Command to run (should return false from isFinished())
     * @return Reference to this binder for chaining
     */
    ButtonBinder& whileTrue(const CommandBase* cmd);

protected:
    /**
     * @brief Check button state and schedule/cancel commands as needed
     * 
     * Called by controller's poll() method. Do not call manually.
     */
    void poll();
    
    friend class Controller;  ///< Controller needs access to poll()

private:
    Controller* m_controller;                   ///< Parent controller
    pros::controller_digital_e_t m_button;      ///< Button identifier
    const CommandBase* km_command;              ///< Bound command (owned by caller)
    Edge m_edge;                                ///< Edge detection mode
    CommandBase* m_runningCommand;              ///< Currently running command instance
};


/**
 * @brief Joystick binding interface for mapping joystick axes to commands
 * 
 * Provides methods to bind commands to joystick movement with threshold detection.
 * Created by Controller joystick methods (LeftJoyY(), etc.) and configured with
 * onTrue(), onFalse(), or whileTrue().
 * Example: controller.LeftJoyY(20).whileTrue(driveCmd.get()) drives when joystick > 20
 */
class JoystickBinder {
public:
    /**
     * @brief Construct a joystick binder
     * @param ctrl Parent controller
     * @param stick Joystick axis identifier
     * @param threshold Minimum absolute value to trigger (0-127)
     */
    JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold);
    
    /**
     * @brief Schedule command when joystick crosses threshold (rising edge)
     * @param cmd Command to schedule
     * @return Reference to this binder for chaining
     */
    JoystickBinder& onTrue(const CommandBase* cmd);
    
    /**
     * @brief Schedule command when joystick falls below threshold (falling edge)
     * @param cmd Command to schedule
     * @return Reference to this binder for chaining
     */
    JoystickBinder& onFalse(const CommandBase* cmd);
    
    /**
     * @brief Run command while joystick above threshold, cancel when below
     * @param cmd Command to run (should return false from isFinished())
     * @return Reference to this binder for chaining
     */
    JoystickBinder& whileTrue(const CommandBase* cmd);

protected:
    /**
     * @brief Check joystick position and schedule/cancel commands as needed
     * 
     * Called by controller's poll() method. Do not call manually.
     */
    void poll();
    
    friend class Controller;  ///< Controller needs access to poll()

private:
    Controller* m_controller;               ///< Parent controller
    pros::controller_analog_e_t m_stick;    ///< Joystick axis identifier
    int m_threshold;                        ///< Minimum value to trigger (0-127)
    const CommandBase* km_command;          ///< Bound command (owned by caller)
    bool m_prev;                            ///< Previous trigger state
    Edge m_edge;                            ///< Edge detection mode
    CommandBase* m_runningCommand;          ///< Currently running command instance
};



#include "custom/scheduler.h"
#endif // CONTROLLER_H_
