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
    ButtonBinder setButtonCommand();
    JoystickBinder setJoystickCommand();
    void poll();

    std::array<bool, 12> prevButtonStates;
    std::vector<ButtonBinder> buttonBinders;
    std::vector<JoystickBinder> joystickBinders;
    Scheduler* scheduler;
};

enum class Edge { None, Rising, Falling, WhileTrue };

class ButtonBinder {
public:
    ButtonBinder(Controller* ctrl);
    ButtonBinder& onTrue(pros::controller_digital_e_t btn, const CommandBase* cmd);
    ButtonBinder& onFalse(pros::controller_digital_e_t btn, const CommandBase* cmd);
    ButtonBinder& whileTrue(pros::controller_digital_e_t btn, const CommandBase* cmd);
    void poll();

private:
    Controller* controller;
    pros::controller_digital_e_t button;
    const CommandBase* command;
    Edge edge;
    CommandBase* runningCommand;
    void registerSubsystems(const CommandBase* cmd);
};


class JoystickBinder {
public:
    JoystickBinder(Controller* ctrl);
    JoystickBinder& onTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd);
    JoystickBinder& onFalse(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd);
    JoystickBinder& whileTrue(pros::controller_analog_e_t stick, int threshold, const CommandBase* cmd);
    void poll();

private:
    Controller* controller;
    pros::controller_analog_e_t stick;
    int threshold;
    const CommandBase* command;
    bool prev;
    Edge edge;
    CommandBase* runningCommand;
    void registerSubsystems(const CommandBase* cmd);
};



#include "custom/scheduler.h"
#endif //CONTROLLER_HPP_
