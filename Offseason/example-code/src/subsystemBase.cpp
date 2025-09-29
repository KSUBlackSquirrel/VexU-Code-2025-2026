// SubsystemBase.cpp
// Implementation of SubsystemBase class with direct scheduler registration
#include "custom/subsystem/subsystemBase.h"
#include "custom/scheduler.h"

// Static members
Scheduler* SubsystemBase::globalScheduler = nullptr;
std::vector<SubsystemBase*> SubsystemBase::pendingSubsystems;

// Constructor automatically registers subsystem with scheduler if available
SubsystemBase::SubsystemBase() {
    if (globalScheduler != nullptr) {
        // Scheduler is available, register immediately
        globalScheduler->registerSubsystemForPeriodic(this);
    } else {
        // Scheduler not yet available, add to pending list
        pendingSubsystems.push_back(this);
    }
}

// Set the global scheduler reference for auto-registration
void SubsystemBase::setScheduler(Scheduler* sched) {
    globalScheduler = sched;
    registerPendingSubsystems();
}

// Register any subsystems created before scheduler was set
void SubsystemBase::registerPendingSubsystems() {
    if (globalScheduler != nullptr) {
        for (SubsystemBase* subsystem : pendingSubsystems) {
            globalScheduler->registerSubsystemForPeriodic(subsystem);
        }
        pendingSubsystems.clear();
    }
}