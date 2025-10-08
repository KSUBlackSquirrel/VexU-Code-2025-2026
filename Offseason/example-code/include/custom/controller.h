#ifndef CONTROLLER_H_
#define CONTROLLER_H_

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
    ButtonBinder RIGHT();
    ButtonBinder DOWN();
    ButtonBinder UP();
    ButtonBinder LEFT();
    ButtonBinder L1();
    ButtonBinder L2();
    ButtonBinder R1();
    ButtonBinder R2();

    JoystickBinder LeftJoyY(int threshold);
    JoystickBinder LeftJoyX(int threshold);
    JoystickBinder RightJoyY(int threshold);
    JoystickBinder RightJoyX(int threshold);

    void poll();

    std::vector<ButtonBinder> m_buttonBinders;
    std::vector<JoystickBinder> m_joystickBinders;
    Scheduler* m_scheduler;
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
    Controller* m_controller;
    pros::controller_digital_e_t m_button;
    const CommandBase* km_command;
    Edge m_edge;
    CommandBase* m_runningCommand;
};


class JoystickBinder {
public:
    JoystickBinder(Controller* ctrl, pros::controller_analog_e_t stick, int threshold);
    JoystickBinder& onTrue(const CommandBase* cmd);
    JoystickBinder& onFalse(const CommandBase* cmd);
    JoystickBinder& whileTrue(const CommandBase* cmd);
    void poll();

private:
    Controller* m_controller;
    pros::controller_analog_e_t m_stick;
    int m_threshold;
    const CommandBase* km_command;
    bool m_prev;
    Edge m_edge;
    CommandBase* m_runningCommand;
};



#include "custom/scheduler.h"
#endif // CONTROLLER_H_
