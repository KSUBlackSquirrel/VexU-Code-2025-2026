/**
 * @file commandBase.h
 * @brief Command-based framework for robot control
 * 
 * This file defines the command system used to control robot behavior.
 * Commands represent actions the robot can perform (drive forward, spin intake, etc.)
 * and are scheduled by the Scheduler to run at the appropriate times.
 * 
 * Key concepts:
 * - Commands control subsystems and define robot behaviors
 * - Multiple commands cannot use the same subsystem simultaneously
 * - Commands have lifecycle methods: initialize(), execute(), end(), isFinished()
 * - Commands can be composed into groups (sequential, parallel)
 * 
 * For creating custom commands, copy template_command.h.
 * 
 * DO NOT modify this file unless you know what you are doing.
 */
#ifndef COMMAND_H_
#define COMMAND_H_

#include "custom/globals.h"
#include "pros/rtos.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <typeinfo>
#include <cstdlib>
#include <initializer_list>
#include <unordered_set>
#include <cassert>
#ifdef __GNUG__
    #include <cxxabi.h>
#endif // __GNUG__

class SubsystemBase;

class TimeoutCommand;
class NamedCommand;
class WaitCommand;
class StartEndCommand;
class FunctionalCommand;
class InstantCommand;

/**
 * @brief Defines how commands respond to scheduling conflicts
 * 
 * When a new command is scheduled that requires a subsystem already in use,
 * the scheduler uses this behavior to resolve the conflict.
 */
enum class InterruptionBehavior {
    kCancelSelf,        ///< This command will be canceled when interrupted
    kCancelIncoming     ///< Incoming commands will be canceled instead of this one
};

/**
 * @brief Base class for all commands in the command-based framework
 * 
 * Commands represent actions the robot performs. They control subsystems and
 * are scheduled by the Scheduler. Commands have a lifecycle with four key methods:
 * 
 * Lifecycle:
 * 1. initialize() - Called once when command starts
 * 2. execute()    - Called repeatedly (~50Hz) while command runs
 * 3. isFinished() - Checked each cycle to determine if command should end
 * 4. end()        - Called once when command finishes or is interrupted
 * 
 * See template_command.h for examples of creating custom commands.
 */
class CommandBase {
    // Friend declarations for classes that need internal access
    friend class SequentialCommandGroup;
    friend class ParallelCommandGroup;
    friend class Scheduler;
    
private:
    std::vector<SubsystemBase*> m_requiredSubsystems;  // Subsystems required for this command
    InterruptionBehavior m_interruptionBehavior;
    bool m_runsWhenDisabled;
    bool m_isComposed = false;

public:
    CommandBase() :
        m_interruptionBehavior(InterruptionBehavior::kCancelSelf),
        m_runsWhenDisabled(false),
        m_isComposed(false)
    {}
    
    // ========================================================================
    // LIFECYCLE METHODS
    // ========================================================================
    
    /**
     * @brief Called once when the command is first scheduled
     * 
     * Use this to initialize variables, reset state, or prepare hardware.
     * Do NOT do this in the constructor - commands are reused and initialize()
     * runs each time the command starts.
     */
    virtual void initialize() {};
    
    /**
     * @brief Called repeatedly while the command is running (~50 times/second)
     * 
     * This is where the main command logic goes. Update motors, read sensors,
     * process controller input, or run control loops here.
     */
    virtual void execute() {};
    
    /**
     * @brief Called once when the command ends
     * @param interrupted True if command was interrupted, false if it finished normally
     * 
     * Use this to clean up resources, stop motors, or save final state.
     * Always stop motors here for safety!
     */
    virtual void end(bool interrupted) {};
    
    /**
     * @brief Determine if the command has completed
     * @return true if command should end, false if it should continue
     * 
     * Examples:
     * - return false; // Never ends (for default commands)
     * - return timer > 1000; // End after 1 second
     * - return sensor > threshold; // End when condition met
     */
    virtual bool isFinished() = 0;
    
    /**
     * @brief Create a copy of this command
     * @return Pointer to new command instance
     * 
     * Required for command groups and decorators. Usually just copy the
     * constructor parameters: return new MyCommand(m_subsystem, m_param);
     */
    virtual CommandBase* clone() const = 0;

    // ========================================================================
    // COMMAND FUNCTIONS
    // ========================================================================

    /**
     * @brief Get the name of this command for debugging
     * @return Command name (auto-generated from class name)
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
        return name.empty() ? "UnknownCommand" : name;
    }

    /**
     * @brief Declare that this command requires a subsystem
     * @param subsystem Subsystem this command controls
     * 
     * Call this in your command constructor for any subsystems the command controls.
     * The scheduler ensures only one command can use a subsystem at a time.
     */
    inline void addRequirements(SubsystemBase* subsystem) { if (subsystem != nullptr) m_requiredSubsystems.push_back(subsystem); }
    
    /**
     * @brief Get all subsystems required by this command
     * @return Vector of subsystem pointers
     */
    inline const std::vector<SubsystemBase*>& getRequiredSubsystems() const { return m_requiredSubsystems; }

    /**
     * @brief Set whether this command can be interrupted
     * @param interruptible If true, command can be canceled; if false, it blocks other commands
     * 
     * Interruptible commands (default) will be canceled when a new command needs their subsystems.
     * Non-interruptible commands block other commands until they finish.
     */
    void setInterruptible(bool interruptible) { m_interruptionBehavior = interruptible ? InterruptionBehavior::kCancelSelf : InterruptionBehavior::kCancelIncoming; }
    
    /**
     * @brief Get the interruption behavior
     * @return Current interruption behavior setting
     */
    InterruptionBehavior getInterruptionBehavior() const { return m_interruptionBehavior; }

    /**
     * @brief Set whether command runs when robot is disabled
     * @param runsWhenDisabled If true, command runs even when disabled
     * 
     * Most commands should NOT run when disabled for safety.
     */
    void setRunsWhenDisabled(bool runsWhenDisabled) { m_runsWhenDisabled = runsWhenDisabled; }
    
    /**
     * @brief Check if command runs when disabled
     * @return True if command runs when disabled
     */
    bool runsWhenDisabled() const { return m_runsWhenDisabled; }

    // ========================================================================
    // COMMAND DECORATORS
    // ========================================================================
    // These methods create modified versions of commands with additional behavior

    /**
     * @brief Create version of command that ends after a timeout
     * @param timeoutSeconds Maximum time command can run (in seconds)
     * @return New command that ends when original finishes OR timeout expires
     * 
     * Usage: cmd->withTimeout(2.0) ends after 2 seconds
     */
    std::unique_ptr<CommandBase> withTimeout(double timeoutSeconds);
    
    /**
     * @brief Create version of command with custom name
     * @param name Custom name for debugging/logging
     * @return New command with specified name
     * 
     * Usage: cmd->withName("IntakeForward") for clearer logs
     */
    std::unique_ptr<CommandBase> withName(const std::string& name);
    
    /**
     * @brief Create sequential group with another command
     * @param next Command to run after this one finishes
     * @return New sequential command group
     * 
     * Usage: cmd1->andThen(cmd2) runs cmd1, then cmd2
     */
    std::unique_ptr<CommandBase> andThen(const CommandBase* next);

    /**
     * @brief Create version with specific interrupt behavior
     * @param behavior Interruption behavior to use
     * @return New command with specified behavior
     */
    std::unique_ptr<CommandBase> withInterruptBehavior(InterruptionBehavior behavior) {
        auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
        clonedCommand->m_interruptionBehavior = behavior;
        return clonedCommand;
    }

    /**
     * @brief Create version that runs even when robot is disabled
     * @param shouldIgnoreDisable If true, command runs when disabled
     * @return New command with specified disabled behavior
     * 
     * Use sparingly - most commands should stop when disabled for safety.
     */
    std::unique_ptr<CommandBase> ignoringDisable(bool shouldIgnoreDisable = true) {
        auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
        clonedCommand->setRunsWhenDisabled(shouldIgnoreDisable);
        return clonedCommand;
    }

protected:
    // ========================================================================
    // INTERNAL METHODS
    // ========================================================================
    // These methods are used internally by the framework. Do not call manually.
    
    /**
     * @brief Mark command as part of a command group
     * @param composed True if command is inside a group
     * 
     * Used internally by command groups to prevent double-scheduling.
     * Do not call manually - handled automatically by SequentialCommandGroup
     * and ParallelCommandGroup.
     */
    void setComposed(bool composed) { m_isComposed = composed; }
    
    /**
     * @brief Check if command is part of a group
     * @return True if command is composed in a group
     * 
     * Used internally by scheduler to prevent scheduling composed commands.
     */
    bool isComposed() const { return m_isComposed; }
};

// ============================================================================
// COMMAND UTILITY CLASSES
// ============================================================================
// These classes provide common command patterns and utilities.
// You typically don't instantiate these directly - use the decorator methods
// like withTimeout() instead.

/**
 * @brief Wrapper that adds a timeout to any command
 * 
 * Ends the wrapped command after a specified time, even if it hasn't finished.
 * Use via cmd->withTimeout(seconds) instead of directly constructing.
 */
class TimeoutCommand : public CommandBase {
private:
    std::unique_ptr<CommandBase> m_command;
    double m_timeout;
    double m_startTime;
    
public:
    TimeoutCommand(
        std::unique_ptr<CommandBase> command,
        double timeout
    ) :
        m_command(std::move(command)),
        m_timeout(timeout),
        m_startTime(0)
    { for (auto* subsystem : m_command->getRequiredSubsystems()) addRequirements(subsystem); } // Copy requirements from wrapped command

    void initialize() override {
        m_startTime = pros::millis();
        m_command->initialize();
    }
    void execute() override { m_command->execute(); }
    void end(bool interrupted) override { m_command->end(interrupted); }
    bool isFinished() override {
        // Check timeout first, then check wrapped command
        double currentTime = pros::millis();
        double elapsedTime = (currentTime - m_startTime) / 1000.0; // Convert to seconds
        
        return elapsedTime >= m_timeout || m_command->isFinished();
    }
    CommandBase* clone() const override { return new TimeoutCommand(std::unique_ptr<CommandBase>(m_command->clone()), m_timeout); }
    std::string getName() const override { return m_command->getName() + "_WithTimeout(" + std::to_string(m_timeout) + "s)"; }
};

/**
 * @brief Wrapper that gives a command a custom name
 * 
 * Delegates all behavior to wrapped command but reports a custom name for logging.
 * Use via cmd->withName("CustomName") instead of directly constructing.
 */
class NamedCommand : public CommandBase {
private:
    std::unique_ptr<CommandBase> m_command;
    std::string m_name;
    
public:
    NamedCommand(
        std::unique_ptr<CommandBase> command,
        const std::string& name
    ) :
        m_command(std::move(command)),
        m_name(name)
    { for (auto* subsystem : m_command->getRequiredSubsystems()) addRequirements(subsystem); } // Copy requirements from wrapped command

    void initialize() override { m_command->initialize(); }
    void execute() override { m_command->execute(); }
    void end(bool interrupted) override { m_command->end(interrupted); }
    bool isFinished() override { return m_command->isFinished(); }
    std::string getName() const override { return m_name; }
    CommandBase* clone() const override {
        return new NamedCommand(std::unique_ptr<CommandBase>(m_command->clone()), m_name);
    }
};

/**
 * @brief Command that waits for a specified duration
 * 
 * Does nothing except wait. Useful in sequential command groups to add delays.
 * Example: new WaitCommand(1.0) waits for 1 second
 */
class WaitCommand : public CommandBase {
private:
    double m_duration;
    double m_startTime;
    
public:
    WaitCommand(
        double seconds
    ) : 
        m_duration(seconds),
        m_startTime(0) 
    {}
    
    void initialize() override { m_startTime = pros::millis(); }
    bool isFinished() override {
        double currentTime = pros::millis();
        double elapsedTime = (currentTime - m_startTime) / 1000.0; // Convert to seconds
        return elapsedTime >= m_duration;
    }
    CommandBase* clone() const override { return new WaitCommand(m_duration); }
    std::string getName() const override { return "Wait(" + std::to_string(m_duration) + "s)"; }
};

/**
 * @brief Command that executes an action for a specified duration
 * 
 * Runs a lambda function repeatedly for a given number of seconds, then ends.
 * Example: RunForCommand(action, 2.0, subsystem) runs action for 2 seconds
 */
class RunForCommand : public CommandBase {
private:
    std::function<void()> m_action;
    double m_duration;
    double m_startTime;
    
public:
    RunForCommand(
        std::function<void()> action,
        double seconds,
        std::initializer_list<SubsystemBase*> subsystems = {}
    ) : 
        m_action(action),
        m_duration(seconds),
        m_startTime(0)
    {  for (auto* subsystem : subsystems) if (subsystem) addRequirements(subsystem); }
    RunForCommand(
        std::function<void()> action,
        double seconds,
        SubsystemBase* subsystem
    ) :
        RunForCommand(action, seconds, {subsystem})
    {}
    
    void initialize() override { m_startTime = pros::millis(); }
    void execute() override { m_action(); }
    bool isFinished() override {
        double currentTime = pros::millis();
        double elapsedTime = (currentTime - m_startTime) / 1000.0; // Convert to seconds
        return elapsedTime >= m_duration;
    }
    CommandBase* clone() const override { 
        auto clone = new RunForCommand(m_action, m_duration);
        // Copy requirements
        for (auto* subsystem : getRequiredSubsystems()) {
            clone->addRequirements(subsystem);
        }
        return clone;
    }
    std::string getName() const override { return "RunFor(" + std::to_string(m_duration) + "s)"; }
};

/**
 * @brief Command that uses lambda functions for all lifecycle methods
 * 
 * Allows quick creation of commands without defining a new class.
 * Useful for prototyping or simple one-off behaviors.
 * 
 * Parameters: onInit, onExecute, onEnd, isFinished, subsystems, name
 */
class FunctionalCommand : public CommandBase {
private:
    std::function<void()> m_onInit = nullptr;
    std::function<void()> m_onExecute = nullptr;
    std::function<void(bool)> m_onEnd = nullptr;
    std::function<bool()> m_isFinished = nullptr;
    std::string m_name = "Functional";
    
public:
    FunctionalCommand(
        std::function<void()> onInit,
        std::function<void()> onExecute,
        std::function<void(bool)> onEnd,
        std::function<bool()> isFinished,
        std::initializer_list<SubsystemBase*> subsystems = {},
        std::string name = "Functional"
    ) : 
        m_onInit(onInit),
        m_onExecute(onExecute),
        m_onEnd(onEnd),
        m_isFinished(isFinished),
        m_name(name)
    {  for (auto* subsystem : subsystems) if (subsystem) addRequirements(subsystem); }
    FunctionalCommand(
        std::function<void()> onInit,
        std::function<void()> onExecute,
        std::function<void(bool)> onEnd,
        std::function<bool()> isFinished,
        SubsystemBase* subsystem,
        std::string name = "Functional"
    ) :
        FunctionalCommand(onInit, onExecute, onEnd, isFinished, {subsystem}, name)
    {}
    
    void initialize() override { if (m_onInit) m_onInit(); }
    void execute() override { if (m_onExecute) m_onExecute(); }
    void end(bool interrupted) override { if (m_onEnd) m_onEnd(interrupted); }
    bool isFinished() override {
        if (m_isFinished) return m_isFinished();
        return false; // Default to never finishing
    }
    CommandBase* clone() const override {
        auto clone = new FunctionalCommand(m_onInit, m_onExecute, m_onEnd, m_isFinished, {}, m_name);
        for (auto* subsystem : getRequiredSubsystems()) {
            clone->addRequirements(subsystem);
        }
        return clone;
    }
    std::string getName() const override { return m_name; }
};

/**
 * @brief Command that runs a single action and immediately finishes
 * 
 * Executes a lambda once in initialize(), then ends. Perfect for one-shot
 * actions like toggling states or resetting positions.
 * Example: InstantCommand(action, subsystem) runs action once
 */
class InstantCommand : public CommandBase {
private:
    std::function<void()> m_action;
    
public:
    InstantCommand(
        std::function<void()> action,
        std::initializer_list<SubsystemBase*> subsystems = {}
    ) :
        m_action(action)
    { for (auto* subsystem : subsystems) if (subsystem) addRequirements(subsystem); }
    InstantCommand(
        std::function<void()> action,
        SubsystemBase* subsystem
    ) :
        InstantCommand(action, {subsystem})
    {}
    
    void initialize() override { if (m_action) m_action(); }
    bool isFinished() override { return true; } // Always finished immediately
    CommandBase* clone() const override {
        auto clone = new InstantCommand(m_action);
        // Copy requirements
        for (auto* subsystem : getRequiredSubsystems()) clone->addRequirements(subsystem);
        return clone;
    }
    std::string getName() const override { return "Instant"; }
};

// ============================================================================
// COMMAND GROUPS
// ============================================================================
// Command groups compose multiple commands into complex behaviors

/**
 * @brief Runs commands one after another in sequence
 * 
 * Each command runs to completion before the next starts. The group finishes
 * when all commands have finished. If interrupted, the current command is
 * interrupted and remaining commands are skipped.
 * Example: SequentialCommandGroup(cmd1, cmd2, cmd3) runs cmd1, then cmd2, then cmd3
 */
class SequentialCommandGroup : public CommandBase {
private:
    std::vector<std::unique_ptr<CommandBase>> m_commands;
    size_t m_index = 0;

    // Helper to add a single command
    void addCommand(const CommandBase* cmd) {
        if (!cmd) return;
        auto cloned = std::unique_ptr<CommandBase>(cmd->clone());
        cloned->setComposed(true);
        for (auto* subsys : cloned->getRequiredSubsystems()) if (subsys) addRequirements(subsys);
        m_commands.push_back(std::move(cloned));
    }

public:
    // Variadic constructor - accepts any number of command pointers
    template<typename... Commands>
    SequentialCommandGroup(Commands*... commands) {
        (addCommand(commands), ...);
    }

    // Construct from initializer list
    SequentialCommandGroup(std::initializer_list<const CommandBase*> commands) {
        for (const CommandBase* cmd : commands) addCommand(cmd);
    }

    // Construct from already-owned command clones
    SequentialCommandGroup(std::vector<std::unique_ptr<CommandBase>>&& clones)
        : m_commands(std::move(clones)) {
        for (auto& c : m_commands) {
            if (c) {
                c->setComposed(true);
                for (auto* subsys : c->getRequiredSubsystems()) if (subsys) addRequirements(subsys);
            }
        }
    }

    void initialize() override {
        m_index = 0;
        if (!m_commands.empty() && m_commands[0]) m_commands[0]->initialize();
    }

    void execute() override {
        if (m_index >= m_commands.size()) return;
        auto& cur = m_commands[m_index];
        if (!cur) { m_index++; return; }
        cur->execute();
        if (cur->isFinished()) {
            cur->end(false);
            m_index++;
            if (m_index < m_commands.size() && m_commands[m_index]) m_commands[m_index]->initialize();
        }
    }

    void end(bool interrupted) override {
        if (m_index < m_commands.size() && m_commands[m_index]) {
            m_commands[m_index]->end(interrupted);
        }
        // for interrupted: call end(true) on remaining commands if desired
        if (interrupted) {
            for (size_t i = m_index + 1; i < m_commands.size(); ++i) {
                if (m_commands[i]) m_commands[i]->end(true);
            }
        }
    }

    bool isFinished() override {
        return m_index >= m_commands.size();
    }

    CommandBase* clone() const override {
        std::vector<std::unique_ptr<CommandBase>> clones;
        clones.reserve(m_commands.size());
        for (const auto& c : m_commands) {
            if (c) clones.emplace_back(c->clone());
            else clones.emplace_back(nullptr);
        }
        return new SequentialCommandGroup(std::move(clones));
    }

    std::string getName() const override {
        std::string name = "Sequential(";
        for (size_t i = 0; i < m_commands.size(); ++i) {
            if (i) name += ", ";
            name += (m_commands[i] ? m_commands[i]->getName() : "<null>");
        }
        name += ")";
        return name;
    }
};

/**
 * @brief Runs multiple commands simultaneously
 * 
 * All commands start together and run in parallel. The group finishes when
 * ALL commands have finished. Commands must NOT share subsystems or the
 * program will crash immediately.
 * 
 * CRITICAL: Commands in parallel groups CANNOT share subsystems!
 * Example: ParallelCommandGroup(driveCmd, intakeCmd) runs both at once (different subsystems)
 */
class ParallelCommandGroup : public CommandBase {
private:
    std::vector<std::unique_ptr<CommandBase>> m_commands;

    // Helper to add a single command
    void addCommand(const CommandBase* cmd) {
        if (!cmd) return;
        auto cloned = std::unique_ptr<CommandBase>(cmd->clone());
        cloned->setComposed(true);
        
        // Check for subsystem conflicts - multiple commands cannot require the same subsystem in parallel
        for (auto* subsys : cloned->getRequiredSubsystems()) {
            if (subsys) {
                // Check if this subsystem is already required by another command in the group
                for (const auto& existingCmd : m_commands) {
                    if (existingCmd) {
                        const auto& existingReqs = existingCmd->getRequiredSubsystems();
                        for (auto* existingSubsys : existingReqs) {
                            if (existingSubsys == subsys) {
                                // FATAL: Commands share subsystem - will crash immediately!
                                volatile int* crash = nullptr;
                                *crash = 0;
                            }
                        }
                    }
                }
                addRequirements(subsys);
            }
        }
        m_commands.push_back(std::move(cloned));
    }

public:
    // Variadic constructor - accepts any number of command pointers
    template<typename... Commands>
    ParallelCommandGroup(Commands*... commands) {
        // Immediate check before any construction
        const CommandBase* cmdArray[] = {commands...};
        std::unordered_set<SubsystemBase*> allSubsystems;
        
        for (size_t i = 0; i < sizeof...(commands); ++i) {
            if (cmdArray[i]) {
                for (auto* subsys : cmdArray[i]->getRequiredSubsystems()) {
                    if (subsys && !allSubsystems.insert(subsys).second) {
                        // FATAL: Duplicate subsystem found - will crash immediately!
                        volatile int* crash = nullptr;
                        *crash = 0;
                    }
                }
            }
        }
        
        // Now add the commands
        (addCommand(commands), ...);
    }

    // Construct from initializer list
    ParallelCommandGroup(std::initializer_list<const CommandBase*> commands) {
        // Check for subsystem conflicts before adding
        std::unordered_set<SubsystemBase*> allSubsystems;
        for (const CommandBase* cmd : commands) {
            if (cmd) {
                for (auto* subsys : cmd->getRequiredSubsystems()) {
                    if (subsys && !allSubsystems.insert(subsys).second) {
                        // FATAL: Duplicate subsystem found - will crash immediately!
                        volatile int* crash = nullptr;
                        *crash = 0;
                    }
                }
            }
        }
        
        // Now add the commands
        for (const CommandBase* cmd : commands) addCommand(cmd);
    }

    // Construct from already-owned command clones
    ParallelCommandGroup(std::vector<std::unique_ptr<CommandBase>>&& clones)
        : m_commands(std::move(clones)) {
        // Check for subsystem conflicts in pre-cloned commands
        std::unordered_set<SubsystemBase*> seenSubsystems;
        for (auto& c : m_commands) {
            if (c) {
                c->setComposed(true);
                for (auto* subsys : c->getRequiredSubsystems()) {
                    if (subsys) {
                        if (!seenSubsystems.insert(subsys).second) {
                            // FATAL: Duplicate subsystem found - will crash immediately!
                            volatile int* crash = nullptr;
                            *crash = 0;
                        }
                        addRequirements(subsys);
                    }
                }
            }
        }
    }

    void initialize() override {
        for (auto& c : m_commands) if (c) c->initialize();
    }

    void execute() override {
        for (auto& c : m_commands) {
            if (!c) continue;
            if (!c->isFinished()) {
                c->execute();
                if (c->isFinished()) {
                    c->end(false);
                }
            }
        }
    }

    void end(bool interrupted) override {
        for (auto& c : m_commands) if (c) c->end(interrupted);
    }

    bool isFinished() override {
        for (auto& c : m_commands) if (c && !c->isFinished()) return false;
        return true;
    }

    CommandBase* clone() const override {
        std::vector<std::unique_ptr<CommandBase>> clones;
        clones.reserve(m_commands.size());
        for (const auto& c : m_commands) {
            if (c) clones.emplace_back(c->clone()); else clones.emplace_back(nullptr);
        }
        return new ParallelCommandGroup(std::move(clones));
    }

    std::string getName() const override {
        std::string name = "Parallel(";
        for (size_t i = 0; i < m_commands.size(); ++i) {
            if (i) name += ", ";
            name += (m_commands[i] ? m_commands[i]->getName() : "<null>");
        }
        name += ")";
        return name;
    }
};


// ============================================================================
// COMMAND DECORATOR IMPLEMENTATIONS (after class definitions)
// ============================================================================

inline std::unique_ptr<CommandBase> CommandBase::withTimeout(double timeoutSeconds) {
    auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
    return std::unique_ptr<CommandBase>(new TimeoutCommand(std::move(clonedCommand), timeoutSeconds));
}

inline std::unique_ptr<CommandBase> CommandBase::withName(const std::string& name) {
    auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
    return std::unique_ptr<CommandBase>(new NamedCommand(std::move(clonedCommand), name));
}

inline std::unique_ptr<CommandBase> CommandBase::andThen(const CommandBase* next) {
    // Create clones of both commands and wrap into a SequentialCommandGroup
    std::vector<std::unique_ptr<CommandBase>> clones;
    clones.emplace_back(this->clone());
    if (next) clones.emplace_back(next->clone());
    auto group = std::unique_ptr<CommandBase>(new SequentialCommandGroup(std::move(clones)));
    return group;
}

#endif // COMMAND_H_
