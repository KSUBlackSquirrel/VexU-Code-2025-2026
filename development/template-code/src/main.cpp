/**
 * @file main.cpp
 * @brief Main program entry points for robot lifecycle
 * 
 * This file contains the PROS framework entry points that are called during
 * different robot states (initialize, disabled, autonomous, opcontrol).
 * Each entry point calls user-defined functions from robot.cpp through the
 * scheduler's run loop.
 */
#include "main.h"

// Forward declarations for robot functions
extern void configureBindings();
extern void robotInit();
extern void robotDisabled();
extern void robotCompInit();
extern void robotAuto();
extern void robotTeleop();

const static uint32_t kMinLoopMs = 20;

/**
 * @brief Helper function to run robot state with scheduler loop
 * @param robotState Whether robot is enabled (true) or disabled (false)
 * @param funcState User function to call each cycle (robotDisabled, robotAuto, etc.)
 * 
 * This helper manages the scheduler loop for each robot state, ensuring:
 * - Scheduler knows robot enabled/disabled state
 * - All previous commands are canceled when state changes
 * - Loop runs at consistent 20ms intervals (~50Hz)
 * - User function is called each cycle for custom logic
 */
void run_helper(bool robotState, void (*funcState)()) {
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

    }
}

/**
 * @brief Called once when the program starts
 * 
 * This function runs once when the robot powers on. It sets up the command-based
 * framework before any competition modes begin.
 * 
 * Initialization order:
 * 1. SubsystemBase::setScheduler() - Enables auto-registration for subsystems
 * 2. Controller::setScheduler() - Enables auto-registration for controllers
 * 3. Scheduler is disabled initially (robot not enabled yet)
 * 4. Scheduler is enabled for processing
 * 5. robotInit() - User initialization code
 * 6. configureBindings() - Set up button/joystick bindings
 * 
 * Keep this function fast - long operations here will block competition modes.
 */
void initialize() {
    SubsystemBase::setScheduler(&Scheduler::getInstance());
    Controller::setScheduler(&Scheduler::getInstance());
    
    Scheduler::getInstance().setRobotEnabled(false);
    Scheduler::getInstance().enable();
    
    robotInit();
    
    configureBindings();
}

/**
 * @brief Called when robot is disabled
 * 
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol.
 * When the robot is enabled, this task will exit.
 */
void disabled() { 
    run_helper(false, robotDisabled);
}

/**
 * @brief Called before autonomous when connected to competition control
 * 
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol starts.
 */
void competition_initialize() {
    run_helper(true, robotCompInit);
}

/**
 * @brief Called during autonomous period
 * 
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
 * @brief Called during teleoperated period
 * 
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the operator control task
 * will be stopped. Re-enabling the robot will restart the task, not resume it
 * from where it left off.
 */
void opcontrol() {
    run_helper(true, robotTeleop);
};
