#ifndef EXAMPLESUBSYSTEM_H_
#define EXAMPLESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"


class ExampleSubsystem : public SubsystemBase {
    public:
        inline ExampleSubsystem() : motor(globalConst::example::motorId, globalConst::example::motorColor) {
            motor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            motor.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
        }

        inline void forward() {
            motor.move_velocity(globalConst::MotorTools::percentToVelocity(30, globalConst::example::motorColor));
        }

        inline void backward() {
            motor.move_velocity(-globalConst::MotorTools::percentToVelocity(30, globalConst::example::motorColor));
        }

        inline void stop() {
            motor.brake();
        }

        inline double getPosition() {
            return motor.get_position();
        }

        inline void periodic() override {
            position = getPosition();
            pros::screen::print(pros::E_TEXT_MEDIUM, 5, "Pulse Count: %3d", count);

            std::cout << '\r' << count << '\n';
            std::cout << '\r' << position << '\n';
        }

        int count = 0;
        double position = 0;

    private:
        pros::Motor motor;

};

#endif