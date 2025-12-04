#include "main.h"

// Forward declarations for robot functions
extern void configureBindings();
extern void robotInit();
extern void robotDisabled();
extern void robotCompInit();
extern void robotAuto();
extern void robotTeleop();

const static uint32_t kMinLoopMs = 20;

void run_helper(bool robotState, void (*funcState)(), std::string name="") {
    std::string functionName = typeid(funcState).name();
    Scheduler::getInstance().setRobotEnabled(robotState);
    Scheduler::getInstance().cancelAll();

    while (true) {
        // Mark the start of this iteration
        static uint32_t lastTick = pros::millis();
        uint32_t iterationStart = pros::millis();
        lastTick = iterationStart;

        funcState();
        Scheduler::getInstance().run();

        // Enforce minimum loop time (20 ms). If work took less than 20ms,
        // sleep the remainder. Otherwise continue immediately.
        uint32_t elapsed = pros::millis() - iterationStart;
        if (elapsed < kMinLoopMs) {
            pros::delay(kMinLoopMs - elapsed);
        }

        // testing \/ \/ \/
        // Print command name and number of commands in scheduler
        size_t commandCount = Scheduler::getInstance().size();
        customPrint::printf("[%s] Commands in scheduler: %zu\n", !name.empty() ? name.c_str() : functionName.c_str(), commandCount);
        customPrint::screenPrint(8, "[%s] Commands in scheduler: %zu", !name.empty() ? name.c_str() : functionName.c_str(), commandCount);
        // testing ^ ^ ^
    }
}

/**
 * This function runs once when the program starts. It sets up all controller bindings
 * and registers the controller with the scheduler. This is where you configure your
 * command-based system before any competition modes begin.
 *
 * - SubsystemBase::setScheduler(&scheduler): Enables auto-registration for subsystems
 * - configureBindings(): Sets up all button/joystick bindings for the controller.
 * - scheduler.registerController(&controller): Registers the controller so its inputs
 *   are polled and commands can be scheduled during opcontrol.
 *
 * Keep this function fast—long operations here will block competition modes.
 */
void initialize() {
    SubsystemBase::setScheduler(&Scheduler::getInstance()); // Enable auto-registration
    Controller::setScheduler(&Scheduler::getInstance()); // Enable auto-registration
    
    Scheduler::getInstance().setRobotEnabled(false);
    Scheduler::getInstance().enable();
    
    configureBindings();

    robotInit();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() { 
    run_helper(false, robotDisabled);
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
    run_helper(true, robotCompInit);
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
    run_helper(true, robotAuto);
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
    run_helper(true, robotTeleop);
};
