#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class ClawSubsystem : public SubsystemBase {
    public:
        ClawSubsystem() :
            pneumaticClaw(globalConst::claw::clawID, LOW)
        {}

        inline void toggleClaw() {
            globalConst::claw::clawState = !globalConst::claw::clawState;
            pneumaticClaw.set_value(globalConst::claw::clawState);
        }

    private:
        pros::adi::DigitalOut pneumaticClaw;
};

