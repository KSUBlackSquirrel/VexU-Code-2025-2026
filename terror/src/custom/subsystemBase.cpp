#include "custom/subsystem/subsystemBase.h"
#include "custom/scheduler.h"

// Static members
Scheduler* SubsystemBase::m_globalScheduler = nullptr;
std::vector<SubsystemBase*> SubsystemBase::m_pendingSubsystems;

// Constructor automatically registers subsystem with scheduler if available
SubsystemBase::SubsystemBase() {
    if (m_globalScheduler != nullptr) {
        // Scheduler is available, register immediately
        m_globalScheduler->registerSubsystemForPeriodic(this);
    } else {
        // Scheduler not yet available, add to pending list
        m_pendingSubsystems.push_back(this);
    }
}

// Set a default command for this subsystem
void SubsystemBase::setDefaultCommand(CommandBase* cmd) {
    if (m_globalScheduler != nullptr) {
        m_globalScheduler->setDefaultCommand(this, cmd);
    }
}

// Get the command currently requiring this subsystem
CommandBase* SubsystemBase::getCurrentCommand() const {
    if (m_globalScheduler != nullptr) {
        return m_globalScheduler->requiring(const_cast<SubsystemBase*>(this));
    }
    return nullptr;
}

// Set the global scheduler reference for auto-registration
void SubsystemBase::setScheduler(Scheduler* sch) {
    m_globalScheduler = sch;
    registerPendingSubsystems();
}

// Register any subsystems created before scheduler was set
void SubsystemBase::registerPendingSubsystems() {
    if (m_globalScheduler != nullptr) {
        for (SubsystemBase* subsystem : m_pendingSubsystems) {
            m_globalScheduler->registerSubsystemForPeriodic(subsystem);
        }
        m_pendingSubsystems.clear();
    }
}