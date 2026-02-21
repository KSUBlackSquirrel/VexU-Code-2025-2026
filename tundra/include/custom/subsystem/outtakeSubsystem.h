#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class OuttakeSubsystem : public SubsystemBase {
    public:
        OuttakeSubsystem() :
            m_outtakeMotor(globalConst::intake::kOuttakeMotorsID, globalConst::intake::kOuttakeMotorColor), 
            pneumaticLift(globalConst::outtake::outtakeLiftID, LOW), 
            pneumaticGate(globalConst::outtake::outtakeGateID, LOW)
        {
            m_outtakeMotor.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
        }

        inline void run(bool reverse=false) {
            m_outtakeMotor.move(globalConst::MotorTools::percentToVelocity(reverse ? -100:100, globalConst::intake::kIntakeMotorColor));
        }
        inline void stop() {
            m_outtakeMotor.brake();
        }

        inline void setGate(bool open) {
            pneumaticGate.set_value(open);
        }

        inline void setLift(bool open) {
            pneumaticLift.set_value(open);
        }


    private:
        pros::MotorGroup m_outtakeMotor;
        pros::adi::DigitalOut pneumaticLift;
        pros::adi::DigitalOut pneumaticGate;

        bool outtakeGate;
        bool outtakeLift;
};

