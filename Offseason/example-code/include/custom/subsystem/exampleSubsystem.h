#ifndef EXAMPLESUBSYSTEM_H_
#define EXAMPLESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"


class ExampleSubsystem : public SubsystemBase {
    public:
        inline ExampleSubsystem()
            : m_motor1(globalConst::example::kMotor1Id, globalConst::example::kMotorColor)
        {
            m_motor1.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);

            m_motor1.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
        }

        inline void forward() {
            m_motor1.move_velocity(globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
        }

        inline void backward() {
            m_motor1.move_velocity(-globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
        }

        inline void stop() {
            m_motor1.brake();
        }

        inline void setPosition(int val, int persent) {
            m_motor1.move_absolute(val, globalConst::MotorTools::percentToVelocity(persent, globalConst::example::kMotorColor));
        }

        inline double getPosition() {
            return m_motor1.get_position();
        }

        inline void periodic() override {

        }

        
    private:
        pros::Motor m_motor1;
        double m_position = 0;
        double m_percent = 100;

};

#endif // EXAMPLESUBSYSTEM_H_