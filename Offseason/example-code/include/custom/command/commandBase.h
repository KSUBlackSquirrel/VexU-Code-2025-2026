// CommandBase.h
// Abstract base class for all commands in the scheduler system.
// Commands encapsulate actions and can require or use subsystems.
#ifndef COMMANDBASE_H_
#define COMMANDBASE_H_

#include "custom/globals.h"
#include <vector>

class SubsystemBase;

// CommandBase: interface for all commands that can be scheduled and executed
class CommandBase {
public:
    CommandBase() {}

    // Called every tick while the command is scheduled
    virtual void execute() = 0;
    // Called when the command ends (naturally or interrupted)
    virtual void end() = 0;
    // Returns true if the command is finished and should be removed
    virtual bool isFinished() = 0;
    // Called if the command is interrupted by another command requiring the same subsystem
    virtual void interrupted() { end(); };

    // Clone the command (used for scheduling new instances)
    virtual CommandBase* clone() const = 0;
    
    // Add a subsystem requirement - interrupts any commands using the same subsystem
    inline void addRequirements(SubsystemBase* subsystem) {
        if (subsystem != nullptr) {
            requiredSubsystems.push_back(subsystem);
        }
    }
    
    // Add a subsystem that this command uses (for periodic updates) without requiring it
    inline void addUsedSubsystem(SubsystemBase* subsystem) {
        if (subsystem != nullptr) {
            usedSubsystems.push_back(subsystem);
        }
    }
    
    // Get all required subsystems (for interruption logic)
    inline const std::vector<SubsystemBase*>& getRequiredSubsystems() const {
        return requiredSubsystems;
    }
    
    // Get all used subsystems (for periodic updates)
    inline const std::vector<SubsystemBase*>& getUsedSubsystems() const {
        return usedSubsystems;
    }

private:
    std::vector<SubsystemBase*> requiredSubsystems;  // Subsystems required for this command
    std::vector<SubsystemBase*> usedSubsystems;      // Subsystems used for periodic updates
};

#endif
