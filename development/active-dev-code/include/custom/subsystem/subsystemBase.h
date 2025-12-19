// SubsystemBase.h
// Abstract base class for all subsystems in the command scheduler system.
// Subsystems represent hardware or logical units that can be updated periodically.
#ifndef SUBSYSTEMBASE_H_
#define SUBSYSTEMBASE_H_

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

class SubsystemBase {
public:
    // Constructor automatically registers subsystem with scheduler if available
    SubsystemBase();
    virtual ~SubsystemBase() = default;
    
    // ========================================================================
    // LIFECYCLE METHODS
    // ========================================================================
    
    virtual void periodic() {};

    // ========================================================================
    // SUBSYSTEM FUNCTIONS
    // ========================================================================

    // Gets the name of the subsystem (auto-generated from class name)
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

    // Sets a command to run when no other command is using the subsystem
    void setDefaultCommand(CommandBase* cmd);
    // Get the command currently requiring this subsystem (returns nullptr if none)
    CommandBase* getCurrentCommand() const;
    // Set the global scheduler reference for auto-registration
    static void setScheduler(Scheduler* sch);
    // Register any subsystems created before scheduler was set
    static void registerPendingSubsystems();

    // ========================================================================
    // COMMAND FACTORY METHODS
    // ========================================================================
    // These methods create commands that automatically require this subsystem
    
    // Create a command that runs the given function once, then finishes
    std::unique_ptr<CommandBase> runOnce(std::function<void()> action);
    // Create a command that runs the given function repeatedly until interrupted
    std::unique_ptr<CommandBase> run(std::function<void()> action);
    // Create a command that runs the given function until the condition becomes true
    std::unique_ptr<CommandBase> runUntil(std::function<void()> action, std::function<bool()> condition);
    // Create a command that runs the given function for a specified number of scheduler cycles
    std::unique_ptr<CommandBase> runFor(std::function<void()> action, int cycles);

private:
    static Scheduler* m_globalScheduler;
    static std::vector<SubsystemBase*> m_pendingSubsystems;
};

// ============================================================================
// COMMAND FACTORY METHOD IMPLEMENTATIONS
// ============================================================================

// Create a command that runs the given function once, then finishes
inline std::unique_ptr<CommandBase> SubsystemBase::runOnce(std::function<void()> action) {
    return std::unique_ptr<CommandBase>(new InstantCommand(action, this));
}

// Create a command that runs the given function repeatedly until interrupted
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

// Create a command that runs the given function until the condition becomes true
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

// Create a command that runs the given function for a specified number of scheduler cycles
inline std::unique_ptr<CommandBase> SubsystemBase::runFor(std::function<void()> action, int cycles) {
    return std::unique_ptr<CommandBase>(new RunForCommand(action, cycles, this));
}

#endif // SUBSYSTEMBASE_H_
