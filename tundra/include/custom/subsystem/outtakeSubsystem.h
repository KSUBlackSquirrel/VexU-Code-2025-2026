#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class OuttakeSubsystem : public SubsystemBase {
    public:
        OuttakeSubsystem() :
            m_outtakeMotor(globalConst::outtake::kOuttakeMotorsID, globalConst::outtake::kOuttakeMotorColor),
            pneumaticLift(globalConst::outtake::outtakeLiftID, LOW), 
            pneumaticGate(globalConst::outtake::outtakeGateID, LOW)
        {
            m_outtakeMotor.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
        }

        inline void run(bool reverse=false) {
            m_outtakeMotor.move(globalConst::MotorTools::percentToVelocity(reverse ? -100:100, globalConst::outtake::kOuttakeMotorColor));
        }
        inline void stop() {
            m_outtakeMotor.brake();
        }

        inline void toggleGate() {
            globalConst::outtake::outtakeGate = !globalConst::outtake::outtakeGate;
            pneumaticGate.set_value(globalConst::outtake::outtakeGate);
        }

        inline void toggleLift() {
            globalConst::outtake::outtakeLift = !globalConst::outtake::outtakeLift;
            pneumaticLift.set_value(globalConst::outtake::outtakeLift);
        }

    private:
        pros::MotorGroup m_outtakeMotor;
        pros::adi::DigitalOut pneumaticLift;
        pros::adi::DigitalOut pneumaticGate;
};

