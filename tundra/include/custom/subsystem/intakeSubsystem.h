#ifndef INTAKESUBSYSTEM_H_
#define INTAKESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class IntakeSubsystem : public SubsystemBase {
    public:
        IntakeSubsystem() :
            m_intakeMotor(globalConst::intake::kIntakeMotorsID, globalConst::intake::kIntakeMotorColor)
        {
            m_intakeMotor.set_brake_mode_all(globalConst::drive::kBreakMode);
        }

        inline void run(bool reverse=false) {
            m_intakeMotor.move(globalConst::MotorTools::percentToVelocity(reverse ? -100:100, globalConst::intake::kIntakeMotorColor));
        }
        inline void stop() {
            m_intakeMotor.brake();
        }

    private:
        pros::MotorGroup m_intakeMotor;
};

#endif // INTAKESUBSYSTEM_H_