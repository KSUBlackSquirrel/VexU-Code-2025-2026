#pragma once

#include "custom/subsystem/subsystemBase.h"


class ExampleSubsystem : public SubsystemBase {
    public:
        inline ExampleSubsystem()
            : m_motor1(globalConst::example::kMotor1Id, globalConst::example::kMotorColor),
              m_motor2(globalConst::example::kMotor2Id, globalConst::example::kMotorColor),
              m_motor3(globalConst::example::kMotor3Id, globalConst::example::kMotorColor)
        {
            m_motor3.set_reversed(true);
            
            m_motor1.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            m_motor2.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            m_motor3.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);

            m_motor1.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
            m_motor2.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
            m_motor3.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
        }

        inline void forward() {
            m_motor1.move_velocity(globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
            m_motor2.move_velocity(globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
            m_motor3.move_velocity(globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
        }

        inline void backward() {
            m_motor1.move_velocity(-globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
            m_motor2.move_velocity(-globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
            m_motor3.move_velocity(-globalConst::MotorTools::percentToVelocity(m_percent, globalConst::example::kMotorColor));
        }

        inline void stop() {
            m_motor1.brake();
            m_motor2.brake();
            m_motor3.brake();
        }

        inline void setPosition(int val, int persent) {
            m_motor1.move_absolute(val, globalConst::MotorTools::percentToVelocity(persent, globalConst::example::kMotorColor));
            m_motor2.move_absolute(val, globalConst::MotorTools::percentToVelocity(persent, globalConst::example::kMotorColor));
            m_motor3.move_absolute(val, globalConst::MotorTools::percentToVelocity(persent, globalConst::example::kMotorColor));
        }

        inline double getPosition() {
            return m_motor1.get_position();
        }

        inline void periodic() override {
            m_position = getPosition();

            customPrint::screenPrint(1, "Pulse Position: %0.2f", m_position);
            customPrint::printf("%0.2f\n", m_position);
        }

        
    private:
        pros::Motor m_motor1;
        pros::Motor m_motor2;
        pros::Motor m_motor3;
        double m_position = 0;
        double m_percent = 100;

};

