#ifndef SUBSYSTEMBASE_H_
#define SUBSYSTEMBASE_H_

#include "custom/globals.h"

class SubsystemBase {
public:
    virtual void periodic() {};

    virtual ~SubsystemBase() = default;
};

#endif
