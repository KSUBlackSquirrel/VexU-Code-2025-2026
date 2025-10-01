#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Scheduler;
#include "custom/command/commandBase.h"
#include "custom/subsystem/subsystemBase.h"
#include <array>
#include <vector>

class ButtonBinder;
class JoystickBinder;

class Controller : public pros::Controller {
public:
    Controller(pros::controller_id_e_t id, Scheduler* sch);
    ButtonBinder A();
    ButtonBinder B();
    ButtonBinder X();
    ButtonBinder Y();
    ButtonBinder Right();
    ButtonBinder Down();
    ButtonBinder Up();
    ButtonBinder Left();
    ButtonBinder L1();
    ButtonBinder L2();
    ButtonBinder R1();
    ButtonBinder R2();

    JoystickBinder LeftJoyY(int threshold);
    JoystickBinder LeftJoyX(int threshold);
    JoystickBinder RightJoyY(int threshold);
    JoystickBinder RightJoyX(int threshold);

    void poll();

    std::array<bool, 12> prevButtonStates;
    std::vector<ButtonBinder> buttonBinders;
    std::vector<JoystickBinder> joystickBinders;
    Scheduler* scheduler;
};

enum class Edge { None, Rising, Falling, WhileTrue };

class ButtonBinder {
public:
    ButtonBinder(Controller* ctrl, pros::controller_digital_e_t btn);
    ButtonBinder& onTrue(const CommandBase* cmd);
    ButtonBinder& onFalse(const CommandBase* cmd);
    ButtonBinder& whileTrue(const CommandBase* cmd);
    void poll();

private:
    Controller* controller;
    pros::controller_digital_e_t button;
    const CommandBase* command;
    Edge edge;
    CommandBase* runningCommand;
};


class JoystickBinder {
public:
    JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold);
    JoystickBinder& onTrue(const CommandBase* cmd);
    JoystickBinder& onFalse(const CommandBase* cmd);
    JoystickBinder& whileTrue(const CommandBase* cmd);
    void poll();

private:
    Controller* controller;
    pros::controller_analog_e_t stick;
    int threshold;
    const CommandBase* command;
    bool prev;
    Edge edge;
    CommandBase* runningCommand;
};



#include "custom/scheduler.h"
#endif //CONTROLLER_HPP_
