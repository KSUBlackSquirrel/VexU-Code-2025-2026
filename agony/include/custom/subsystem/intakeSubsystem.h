#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class IntakeSubsystem : public SubsystemBase {
    public:
        IntakeSubsystem() :
            m_intakeMotor(globalConst::intake::kIntakeMotorsID, globalConst::intake::kIntakeMotorColor),
            pneumaticChute(globalConst::intake::intakeChuteID, globalConst::intake::intakeChute)
        {
            m_intakeMotor.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
            pneumaticChute.set_value(globalConst::intake::intakeChute);
        }

        inline void run(bool reverse=false) {
            m_intakeMotor.move(globalConst::MotorTools::percentToVelocity(reverse ? -100:100, globalConst::intake::kIntakeMotorColor));
        }
        inline void stop() {
            m_intakeMotor.brake();
        }

        inline void toggleChute() {
            globalConst::intake::intakeChute = !globalConst::intake::intakeChute;
            pneumaticChute.set_value(globalConst::intake::intakeChute);
        }


    private:
        pros::MotorGroup m_intakeMotor;
        pros::adi::DigitalOut pneumaticChute;
};

