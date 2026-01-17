/**
 * @file subsystemBase.h
 * @brief Base class for robot subsystems
 * 
 * Subsystems represent physical or logical components of the robot (drive, intake,
 * lift, etc.). They encapsulate hardware (motors, sensors) and provide methods
 * for commands to control them.
 * 
 * Key concepts:
 * - Subsystems are automatically registered with the scheduler
 * - periodic() runs continuously (~50Hz) for monitoring and telemetry
 * - Only one command can control a subsystem at a time
 * - Subsystems can have default commands that run when idle
 * 
 * For creating custom subsystems, copy template_subsystem.h.
 * 
 * DO NOT modify this file unless you know what you are doing.
 */
#ifndef SUBSYSTEM_H_
#define SUBSYSTEM_H_

#include "custom/globals.h"
#include "custom/command/commandBase.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <typeinfo>
#include <cstdlib>
#ifdef __GNUG__
    #include <cxxabi.h>
#endif // __GNUG__

class Scheduler;

/**
 * @brief Base class for all robot subsystems
 * 
 * Subsystems represent physical components (motors, pistons, sensors) or logical
 * units (vision processing, path following) of the robot. They provide methods
 * for commands to control hardware and handle periodic updates.
 * 
 * Lifecycle:
 * - Constructor: Initialize hardware (motors, sensors)
 * - periodic(): Called continuously (~50Hz) for monitoring/telemetry
 * 
 * See template_subsystem.h for examples of creating custom subsystems.
 */
class SubsystemBase {
public:
    /**
     * @brief Construct a subsystem and automatically register with scheduler
     * 
     * The constructor automatically registers the subsystem with the global scheduler
     * if it's available. If not, the subsystem is added to a pending list and
     * registered later when the scheduler becomes available.
     */
    SubsystemBase();
    virtual ~SubsystemBase() = default;
    
    // ========================================================================
    // LIFECYCLE METHODS
    // ========================================================================
    
    /**
     * @brief Called continuously by the scheduler (~50 times/second)
     * 
     * Use this for:
     * - Updating sensor readings
     * - Running background control loops
     * - Monitoring safety conditions (temperature, current, etc.)
     * - Updating telemetry/debugging displays
     * 
     * This runs even when no commands are using the subsystem!
     * Perfect for continuous monitoring and safety checks.
     */
    virtual void periodic() {};

    // ========================================================================
    // SUBSYSTEM FUNCTIONS
    // ========================================================================

    /**
     * @brief Get the name of this subsystem for debugging
     * @return Subsystem name (auto-generated from class name)
     * 
     * Used in scheduler logging and telemetry. Override to provide custom names.
     */
    virtual std::string getName() const {
        const char* typeName = typeid(*this).name();
        #ifdef __GNUG__
            int status = 0;
            char* demangled = abi::__cxa_demangle(typeName, 0, 0, &status);
            if (status == 0 && demangled) {
                std::string result(demangled);
                free(demangled);
                return result;
            }
        #endif
        std::string name(typeName);
        size_t classPos = name.find("class ");
        if (classPos != std::string::npos) {
            name = name.substr(classPos + 6);
        }
        size_t templatePos = name.find('<');
        if (templatePos != std::string::npos) {
            name = name.substr(0, templatePos);
        }
        return name.empty() ? "UnknownSubsystem" : name;
    }

    /**
     * @brief Set a command to run when no other command is using this subsystem
     * @param cmd Command to run as default (typically a stop or idle command)
     * 
     * Default commands run when the subsystem is not being used by other commands.
     * They're automatically interrupted when a new command requires the subsystem.
     * 
     * Example: A drive subsystem might have a default command that stops motors,
     * or an intake that idles at low speed.
     */
    void setDefaultCommand(CommandBase* cmd);
    
    /**
     * @brief Get the command currently requiring this subsystem
     * @return Pointer to current command, or nullptr if subsystem is idle
     */
    CommandBase* getCurrentCommand() const;
    
    /**
     * @brief Set the global scheduler reference for auto-registration
     * @param sch Pointer to scheduler instance
     * 
     * Called once during initialization to enable subsystem auto-registration.
     * Do not call this manually - it's handled in main.cpp.
     */
    static void setScheduler(Scheduler* sch);

    // ========================================================================
    // COMMAND FACTORY METHODS
    // ========================================================================
    // These methods create commands that automatically require this subsystem.
    // Useful for simple behaviors without defining a full command class.
    
    /**
     * @brief Create a command that runs the given function once, then finishes
     * @param action Lambda function to execute
     * @return Unique pointer to created command
     * 
     * Usage: subsystem->runOnce([]{ subsystem->reset(); })
     */
    std::unique_ptr<CommandBase> runOnce(std::function<void()> action);
    
    /**
     * @brief Create a command that runs the given function repeatedly until interrupted
     * @param action Lambda function to execute each cycle
     * @return Unique pointer to created command
     * 
     * Usage: subsystem->run([]{ subsystem->setSpeed(50); })
     */
    std::unique_ptr<CommandBase> run(std::function<void()> action);
    
    /**
     * @brief Create a command that runs the given function until condition becomes true
     * @param action Lambda function to execute each cycle
     * @param condition Lambda function that returns true when command should end
     * @return Unique pointer to created command
     * 
     * Usage: subsystem->runUntil([]{ subsystem->move(); }, []{ return subsystem->atTarget(); })
     */
    std::unique_ptr<CommandBase> runUntil(std::function<void()> action, std::function<bool()> condition);
    
    /**
     * @brief Create a command that runs the given function for specified duration
     * @param action Lambda function to execute each cycle
     * @param cycles Duration in seconds
     * @return Unique pointer to created command
     * 
     * Usage: subsystem->runFor([]{ subsystem->spin(); }, 2.0)
     */
    std::unique_ptr<CommandBase> runFor(std::function<void()> action, int cycles);

protected:
    // ========================================================================
    // INTERNAL METHODS
    // ========================================================================
    // These methods are used internally by the framework. Do not call manually.
    
    /**
     * @brief Register any subsystems created before scheduler was set
     * 
     * Called internally after scheduler is initialized. Do not call manually.
     */
    static void registerPendingSubsystems();

private:
    static Scheduler* m_globalScheduler;
    static std::vector<SubsystemBase*> m_pendingSubsystems;
};

// ============================================================================
// COMMAND FACTORY METHOD IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Create a command that runs the given function once, then finishes
 * @return InstantCommand that executes action immediately
 */
inline std::unique_ptr<CommandBase> SubsystemBase::runOnce(std::function<void()> action) {
    return std::unique_ptr<CommandBase>(new InstantCommand(action, this));
}

/**
 * @brief Create a command that runs the given function repeatedly until interrupted
 * @return FunctionalCommand that executes action each cycle
 */
inline std::unique_ptr<CommandBase> SubsystemBase::run(std::function<void()> action) {
    return std::unique_ptr<CommandBase>(new FunctionalCommand(
        nullptr,    // onInit
        action,     // onExecute (runs every cycle)
        nullptr,    // onEnd
        nullptr,    // isFinished (defaults to false - runs until interrupted)
        this,       // subsystem
        "Run"       // name
    ));
}

/**
 * @brief Create a command that runs the given function until condition becomes true
 * @return FunctionalCommand that executes action until condition returns true
 */
inline std::unique_ptr<CommandBase> SubsystemBase::runUntil(std::function<void()> action, std::function<bool()> condition) {
    return std::unique_ptr<CommandBase>(new FunctionalCommand(
        nullptr,    // onInit
        action,     // onExecute (runs every cycle)
        nullptr,    // onEnd
        condition,  // isFinished (stops when condition is true)
        this,       // subsystem
        "RunUntil"  // name
    ));
}

/**
 * @brief Create a command that runs the given function for specified duration
 * @return RunForCommand that executes action for specified number of seconds
 */
inline std::unique_ptr<CommandBase> SubsystemBase::runFor(std::function<void()> action, int cycles) {
    return std::unique_ptr<CommandBase>(new RunForCommand(action, cycles, this));
}

#endif // SUBSYSTEM_H_
