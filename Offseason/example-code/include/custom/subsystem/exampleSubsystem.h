#ifndef EXAMPLESUBSYSTEM_H_
#define EXAMPLESUBSYSTEM_H_

#include "custom/subsystem/subsystemBase.h"


class ExampleSubsystem : public SubsystemBase {
    public:
        inline ExampleSubsystem() : motor(globalConst::example::motorId, globalConst::example::motorColor) {
            motor.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
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
            globalVar::pulse::count += 1;
            globalVar::pulse::position = motor.get_position();
        }

    private:
        pros::Motor motor;

};

#endif