#ifndef OUTTAKESUBSYSTEM_H_
#define OUTTAKESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"
#include "lemlib/api.hpp"


class OuttakeSubsystem : public SubsystemBase {
    public:
        OuttakeSubsystem() :
            m_outtakeMotor(globalConst::intake::kOuttakeMotorsID, globalConst::intake::kOuttakeMotorColor)
        {
            m_outtakeMotor.set_brake_mode_all(globalConst::drive::kBreakMode);
        }

        inline void run(bool reverse=false) {
            m_outtakeMotor.move(globalConst::MotorTools::percentToVelocity(reverse ? -100:100, globalConst::intake::kIntakeMotorColor));
        }
        inline void stop() {
            m_outtakeMotor.brake();
        }


    private:
        pros::MotorGroup m_outtakeMotor;
};

#endif // OUTTAKESUBSYSTEM_H_