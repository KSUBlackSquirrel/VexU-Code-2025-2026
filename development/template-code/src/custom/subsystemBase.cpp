/**
 * @file subsystemBase.cpp
 * @brief Implementation of subsystem base class
 * 
 * Provides the implementation for SubsystemBase, handling automatic registration
 * with the scheduler and default command management.
 */
#include "custom/subsystem/subsystemBase.h"
#include "custom/scheduler.h"

// Static members
Scheduler* SubsystemBase::m_globalScheduler = nullptr;
std::vector<SubsystemBase*> SubsystemBase::m_pendingSubsystems;

/**
 * @brief Construct subsystem and register with scheduler
 * 
 * Automatically registers with scheduler if available, otherwise queues for
 * later registration when scheduler is initialized.
 */
SubsystemBase::SubsystemBase() {
    if (m_globalScheduler != nullptr) {
        // Scheduler is available, register immediately
        m_globalScheduler->registerSubsystemForPeriodic(this);
    } else {
        // Scheduler not yet available, add to pending list
        m_pendingSubsystems.push_back(this);
    }
}

/**
 * @brief Set a default command for this subsystem
 * @param cmd Command to run when subsystem is idle
 * 
 * Default commands run automatically when no other command requires the subsystem.
 * They're interrupted when a new command needs the subsystem.
 */
void SubsystemBase::setDefaultCommand(CommandBase* cmd) {
    if (m_globalScheduler != nullptr) {
        m_globalScheduler->setDefaultCommand(this, cmd);
    }
}

/**
 * @brief Get the command currently requiring this subsystem
 * @return Pointer to current command, or nullptr if subsystem is idle
 */
CommandBase* SubsystemBase::getCurrentCommand() const {
    if (m_globalScheduler != nullptr) {
        return m_globalScheduler->requiring(const_cast<SubsystemBase*>(this));
    }
    return nullptr;
}

/**
 * @brief Set the global scheduler reference for auto-registration
 * @param sch Pointer to scheduler instance
 * 
 * Called once during initialization to enable subsystem auto-registration.
 * Registers any pending subsystems that were created before scheduler was available.
 */
void SubsystemBase::setScheduler(Scheduler* sch) {
    m_globalScheduler = sch;
    registerPendingSubsystems();
}

/**
 * @brief Register pending subsystems with scheduler
 * 
 * Processes all subsystems that were created before the scheduler was available,
 * registering them so their periodic() methods will be called.
 */
void SubsystemBase::registerPendingSubsystems() {
    if (m_globalScheduler != nullptr) {
        for (SubsystemBase* subsystem : m_pendingSubsystems) {
            m_globalScheduler->registerSubsystemForPeriodic(subsystem);
        }
        m_pendingSubsystems.clear();
    }
}