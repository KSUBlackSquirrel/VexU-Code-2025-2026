#ifndef INTAKESUBSYSTEM_H_
#define INTAKESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class IntakeSubsystem : public SubsystemBase {
    public:
        IntakeSubsystem() :
            // m_motorOne(globalConst::intake::kMotorsID[0], globalConst::intake::kMotorColor),
            // m_motorTwo(globalConst::intake::kMotorsID[1], globalConst::intake::kMotorColor)
            m_motor(globalConst::intake::kMotorsID, globalConst::intake::kMotorColor)
        {
            m_motor.set_brake_mode_all(globalConst::drive::kBreakMode);
        }

        inline void runIn() {
            m_motor.move(globalConst::MotorTools::percentToVelocity(100, globalConst::intake::kMotorColor));
            // m_motorOne.move(globalConst::MotorTools::percentToVelocity(100, globalConst::intake::kMotorColor));
            // m_motorTwo.move(globalConst::MotorTools::percentToVelocity(100, globalConst::intake::kMotorColor));
        }
        inline void runOut() {
            m_motor.move(globalConst::MotorTools::percentToVelocity(-100, globalConst::intake::kMotorColor));
            // m_motorOne.move(globalConst::MotorTools::percentToVelocity(-100, globalConst::intake::kMotorColor));
            // m_motorTwo.move(globalConst::MotorTools::percentToVelocity(-100, globalConst::intake::kMotorColor));
        }
        inline void stop() {
            m_motor.brake();
            // m_motorOne.brake();
            // m_motorTwo.brake();
        }

    private:
        pros::MotorGroup m_motor;
        // pros::Motor m_motorOne;
        // pros::Motor m_motorTwo;
};

#endif // INTAKESUBSYSTEM_H_