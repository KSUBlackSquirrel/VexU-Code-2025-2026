// SubsystemBase.h
// Abstract base class for all subsystems in the command scheduler system.
// Subsystems represent hardware or logical units that can be updated periodically.
#ifndef SUBSYSTEMBASE_H_
#define SUBSYSTEMBASE_H_

#include "custom/globals.h"

class SubsystemBase {
public:
    // Called every loop if registered with the scheduler
    virtual void periodic() {};

    // Ensure proper cleanup for derived subsystems
    virtual ~SubsystemBase() = default;
};

#endif
