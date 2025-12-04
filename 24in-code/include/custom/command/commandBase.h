// CommandBase.h
#ifndef COMMANDBASE_H_
#define COMMANDBASE_H_

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

// Interruption behavior enum
enum class InterruptionBehavior {
    kCancelSelf,        // This command will be canceled when interrupted
    kCancelIncoming     // Incoming commands will be canceled instead of this one
};

// CommandBase: interface for all commands that can be scheduled and executed
class CommandBase {
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
    
    virtual void initialize() {};
    virtual void execute() {};
    virtual void end(bool interrupted) {};
    virtual bool isFinished() = 0;
    virtual CommandBase* clone() const = 0;

    // ========================================================================
    // COMMAND FUNCTIONS
    // ========================================================================

    // Command properties
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

    // subsystem behavior
    inline void addRequirements(SubsystemBase* subsystem) { if (subsystem != nullptr) m_requiredSubsystems.push_back(subsystem); }
    inline const std::vector<SubsystemBase*>& getRequiredSubsystems() const { return m_requiredSubsystems; }

    // Interruption behavior
    void setInterruptible(bool interruptible) { m_interruptionBehavior = interruptible ? InterruptionBehavior::kCancelSelf : InterruptionBehavior::kCancelIncoming; }
    InterruptionBehavior getInterruptionBehavior() const { return m_interruptionBehavior; }

    // Robot state behavior
    void setRunsWhenDisabled(bool runsWhenDisabled) { m_runsWhenDisabled = runsWhenDisabled; }
    bool runsWhenDisabled() const { return m_runsWhenDisabled; }

    // Composition detection
    void setComposed(bool composed) { m_isComposed = composed; }
    bool isComposed() const { return m_isComposed; }

    // ========================================================================
    // COMMAND DECORATORS
    // ========================================================================

    // Defined later at bottom of code
    std::unique_ptr<CommandBase> withTimeout(double timeoutSeconds);
    std::unique_ptr<CommandBase> withName(const std::string& name);
    std::unique_ptr<CommandBase> andThen(const CommandBase* next);

    std::unique_ptr<CommandBase> withInterruptBehavior(InterruptionBehavior behavior) {
        auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
        clonedCommand->m_interruptionBehavior = behavior;
        return clonedCommand;
    }

    std::unique_ptr<CommandBase> ignoringDisable(bool shouldIgnoreDisable = true) {
        auto clonedCommand = std::unique_ptr<CommandBase>(this->clone());
        clonedCommand->setRunsWhenDisabled(shouldIgnoreDisable);
        return clonedCommand;
    }
};

// ============================================================================
// COMMAND UTILITY CLASSES
// ============================================================================

// Wrapper command that adds a timeout to any command
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

// Create a wrapper command that delegates to the original but has a custom name
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

// Command that waits for a specified time
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

// RunFor command - executes action for specified duration in seconds
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

// Command that accepts lambdas for all lifecycle methods
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

// Command that runs a single action and immediately finishes
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
// SequentialCommandGroup: runs commands one after another
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

// ParallelCommandGroup: runs all commands at once; finishes when all complete
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

#endif // COMMANDBASE_H_
