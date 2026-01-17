/**
 * @file scheduler.h
 * @brief Command scheduler for managing command execution
 * 
 * The scheduler is the central component of the command-based framework. It manages
 * all commands, subsystems, and controller inputs, ensuring commands run properly
 * and subsystems are never controlled by multiple commands simultaneously.
 * 
 * Key responsibilities:
 * - Execute scheduled commands each cycle (~50Hz)
 * - Manage subsystem requirements and prevent conflicts
 * - Handle command interruption and completion
 * - Run subsystem periodic() methods continuously
 * - Poll controller inputs for button/joystick events
 * - Track performance with watchdog monitoring
 * 
 * The scheduler is a singleton accessed via Scheduler::getInstance().
 * 
 * DO NOT modify this file unless you know what you are doing.
 */
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

/**
 * @brief Central scheduler for command-based robot control
 * 
 * The scheduler manages all commands and subsystems, ensuring proper execution
 * order and preventing conflicts. It runs continuously during all robot modes
 * (teleop, autonomous, disabled).
 * Example: Scheduler::getInstance().schedule(myCommand.get())
 */
class Scheduler {
    // Friend declarations for classes that need access to protected registration methods
    friend class Controller;
    friend class SubsystemBase;
    
public:
    /**
     * @brief Get the singleton scheduler instance
     * @return Reference to the global scheduler
     */
    static Scheduler& getInstance();
    
    // Core scheduling methods
    
    /**
     * @brief Schedule a command to run
     * @param cmd Command to schedule (must be const pointer - scheduler clones it)
     * @return Pointer to the scheduled command instance (owned by scheduler)
     * 
     * If the command's required subsystems are in use:
     * - Interruptible commands using those subsystems are canceled
     * - Non-interruptible commands block the new command
     * 
     * Usage: scheduler.schedule(myCommand.get());
     */
    CommandBase* schedule(const CommandBase* cmd);
    
    /**
     * @brief Set a default command for a subsystem
     * @param sub Subsystem to configure
     * @param cmd Command to run when subsystem is idle
     * 
     * Default commands automatically run when no other command requires the subsystem.
     * They're interrupted when a new command needs the subsystem.
     */
    void setDefaultCommand(SubsystemBase* sub, CommandBase* cmd);
    
    /**
     * @brief Cancel a specific running command
     * @param commandInstance Pointer to command instance to cancel
     * 
     * Calls end(true) on the command and removes it from the scheduler.
     */
    void cancel(CommandBase* commandInstance);
    
    /**
     * @brief Cancel all running commands
     * 
     * Calls end(true) on all commands and clears the scheduler queue.
     * Used when switching robot modes (autonomous to teleop, etc.).
     */
    void cancelAll();
    
    /**
     * @brief Register a controller for button/joystick polling
     * @param ctrl Controller to register
     * 
     * Controllers are automatically registered via Controller::setScheduler().
     * Do not call this manually - framework internal use only.
     */
    void registerController(Controller* ctrl);
    
    /**
     * @brief Register a subsystem for periodic updates
     * @param sub Subsystem to register
     * 
     * Subsystems are automatically registered via SubsystemBase::setScheduler().
     * Do not call this manually - framework internal use only.
     */
    void registerSubsystemForPeriodic(SubsystemBase* sub);
    
    /**
     * @brief Run one scheduler cycle
     * 
     * This is the main scheduler loop. It:
     * 1. Polls controller inputs
     * 2. Executes all running commands
     * 3. Runs subsystem periodic() methods
     * 4. Checks for finished commands
     * 5. Schedules default commands for idle subsystems
     * 
     * Called continuously in main loop (~50Hz).
     */
    void run();
    
    // Scheduler state management
    
    /**
     * @brief Enable the scheduler
     * 
     * When enabled, the scheduler processes commands and updates subsystems.
     * Called during initialization.
     */
    void enable();
    
    /**
     * @brief Disable the scheduler
     * 
     * When disabled, scheduler operations are paused.
     */
    void disable();
    
    /**
     * @brief Check if scheduler is enabled
     * @return True if scheduler is running
     */
    bool isEnabled() const;
    
    // Command state queries
    
    /**
     * @brief Check if a command is currently scheduled
     * @param cmd Command to check (const pointer to original)
     * @return True if any clone of this command is running
     */
    bool isScheduled(const CommandBase* cmd) const;
    
    /**
     * @brief Get the command currently requiring a subsystem
     * @param subsystem Subsystem to check
     * @return Pointer to command using the subsystem, or nullptr if idle
     */
    CommandBase* requiring(SubsystemBase* subsystem) const;
    
    // Command event callbacks
    
    /**
     * @brief Register callback for command initialization
     * @param action Function to call when any command is initialized
     * 
     * Useful for logging or telemetry. Called when command.initialize() runs.
     */
    void onCommandInitialize(std::function<void(CommandBase*)> action);
    
    /**
     * @brief Register callback for command execution
     * @param action Function to call when any command executes
     * 
     * Called each cycle that command.execute() runs.
     */
    void onCommandExecute(std::function<void(CommandBase*)> action);
    
    /**
     * @brief Register callback for command completion
     * @param action Function to call when any command finishes normally
     * 
     * Called when command.isFinished() returns true and command.end(false) runs.
     */
    void onCommandFinish(std::function<void(CommandBase*)> action);
    
    /**
     * @brief Register callback for command interruption
     * @param action Function to call when any command is interrupted
     * 
     * Called when command is canceled and command.end(true) runs.
     */
    void onCommandInterrupt(std::function<void(CommandBase*)> action);
    
    // Robot state awareness
    
    /**
     * @brief Set whether robot is enabled
     * @param enabled True if robot is enabled (teleop/auto), false if disabled
     * 
     * Commands with runsWhenDisabled=false are canceled when robot is disabled.
     */
    void setRobotEnabled(bool enabled);
    
    /**
     * @brief Check if robot is enabled
     * @return True if robot is enabled
     */
    bool isRobotEnabled() const;
    
    // Utility methods
    
    /**
     * @brief Get number of running commands
     * @return Count of active commands
     */
    std::size_t size() const;
    
    /**
     * @brief Get number of default commands configured
     * @return Count of default commands
     */
    std::size_t defaultSize() const;
    
    /**
     * @brief Check if scheduler has any running commands
     * @return True if no commands are running
     */
    bool empty() const;

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Scheduler();
    
    // Internal helper methods
    
    /**
     * @brief Initialize a command and track its subsystem requirements
     * @param command Command to initialize
     * @param requirements Subsystems the command requires
     */
    void initCommand(CommandBase* command, const std::unordered_set<SubsystemBase*>& requirements);
    
    /**
     * @brief Interrupt and remove a command
     * @param command Command to interrupt
     */
    void interruptCommand(CommandBase* command);
    
    /**
     * @brief Finish a command normally and remove it
     * @param command Command that finished
     */
    void finishCommand(CommandBase* command);
    
    /**
     * @brief Check if all required subsystems are free
     * @param requirements Subsystems to check
     * @return True if all subsystems are available
     */
    bool requirementsFree(const std::unordered_set<SubsystemBase*>& requirements) const;
    
    /**
     * @brief Check if commands using subsystems are interruptible
     * @param requirements Subsystems to check
     * @return True if all commands using those subsystems can be interrupted
     */
    bool areCommandsInterruptible(const std::unordered_set<SubsystemBase*>& requirements) const;

    // Scheduler state
    bool m_enabled = false;         ///< Whether scheduler is processing commands
    bool m_robotEnabled = false;    ///< Whether robot is enabled (vs disabled mode)
    bool m_inRunLoop = false;       ///< Whether currently in run() method
    
    // Command storage
    std::vector<std::unique_ptr<CommandBase>> m_queue;                      ///< Active commands
    std::unordered_map<SubsystemBase*, CommandBase*> m_defaultCommands;     ///< Default commands per subsystem
    std::unordered_map<SubsystemBase*, CommandBase*> m_requirements;        ///< Current command per subsystem
    std::vector<SubsystemBase*> m_periodicSubsystems;                       ///< Subsystems to update each cycle
    std::vector<Controller*> m_controllers;                                  ///< Controllers to poll each cycle
    
    // Commands to schedule during run loop
    std::vector<CommandBase*> m_toSchedule;  ///< Commands queued for scheduling
    
    // Event callbacks
    std::vector<std::function<void(CommandBase*)>> m_initActions;       ///< Initialize callbacks
    std::vector<std::function<void(CommandBase*)>> m_executeActions;    ///< Execute callbacks
    std::vector<std::function<void(CommandBase*)>> m_finishActions;     ///< Finish callbacks
    std::vector<std::function<void(CommandBase*)>> m_interruptActions;  ///< Interrupt callbacks
    
    // Performance monitoring
    Watchdog m_watchdog;  ///< Performance tracking for slow loops
};

#endif // SCHEDULER_H_
