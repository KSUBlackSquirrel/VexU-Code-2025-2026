#ifndef EXAMPLESUBSYSTEM_H_
#define EXAMPLESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"


class ExampleSubsystem : public SubsystemBase {
    public:
        inline ExampleSubsystem() : m_motor(globalConst::example::kMotorId, globalConst::example::kMotorColor) {
            m_motor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            m_motor.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
        }

        inline void forward() {
            m_motor.move_velocity(globalConst::MotorTools::percentToVelocity(30, globalConst::example::kMotorColor));
        }

        inline void backward() {
            m_motor.move_velocity(-globalConst::MotorTools::percentToVelocity(30, globalConst::example::kMotorColor));
        }

        inline void stop() {
            m_motor.brake();
        }

        inline void setPosition(int val, int persent) {
            m_motor.move_absolute(val, globalConst::MotorTools::percentToVelocity(persent, globalConst::example::kMotorColor));
        }

        inline double getPosition() {
            return m_motor.get_position();
        }

        inline void periodic() override {
            m_position = getPosition();

            pros::screen::print(pros::E_TEXT_MEDIUM, 6, "Pulse Position: %f", m_position);
            
            printf("%3d\n", m_position);

            std::cout << m_position << '\n';
        }

        
    private:
        pros::Motor m_motor;
        double m_position = 0;

};

#endif // EXAMPLESUBSYSTEM_H_