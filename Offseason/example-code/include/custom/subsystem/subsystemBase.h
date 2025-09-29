// SubsystemBase.h
// Abstract base class for all subsystems in the command scheduler system.
// Subsystems represent hardware or logical units that can be updated periodically.
#ifndef SUBSYSTEMBASE_H_
#define SUBSYSTEMBASE_H_

#include "custom/globals.h"
#include <vector>

// Forward declaration
class Scheduler;

class SubsystemBase {
public:
    // Constructor automatically registers subsystem with scheduler if available
    SubsystemBase();
    
    // Called every loop if registered with the scheduler
    virtual void periodic() {};

    // Ensure proper cleanup for derived subsystems
    virtual ~SubsystemBase() = default;
    
    // Set the global scheduler reference for auto-registration
    static void setScheduler(Scheduler* sched);
    
    // Register any subsystems created before scheduler was set
    static void registerPendingSubsystems();

private:
    static Scheduler* globalScheduler;
    static std::vector<SubsystemBase*> pendingSubsystems;
};

#endif
