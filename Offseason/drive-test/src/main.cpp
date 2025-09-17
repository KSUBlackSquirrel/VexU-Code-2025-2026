#include "main.h"

static Scheduler scheduler;

Controller controller(globalDrive::mainControllerID, &scheduler);

// Add Subsystems Here

// Add Any Button Bindings Here
void configureBindings() {
    controller.setButtonCommand().onTrue(pros::E_CONTROLLER_DIGITAL_A, new InstantCommand([] { int num=0; }));
    controller.setJoystickCommand().onFalse(pros::E_CONTROLLER_ANALOG_RIGHT_Y, -20, new InstantCommand([] { int num=0; }));
}


/**
 * This function runs once when the program starts. It sets up all controller bindings
 * and registers the controller with the scheduler. This is where you configure your
 * command-based system before any competition modes begin.
 *
 * - configureBindings(): Sets up all button/joystick bindings for the controller.
 * - scheduler.registerController(&controller): Registers the controller so its inputs
 *   are polled and commands can be scheduled during opcontrol.
 *
 * Keep this function fast—long operations here will block competition modes.
 */
void initialize() {
    configureBindings();
    scheduler.registerController(&controller);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

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
void autonomous() {}

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
    while (true) {
        pros::screen::print(pros::E_TEXT_MEDIUM, 3, "List Size: %3d", static_cast<int>(scheduler.size()));
        pros::screen::print(pros::E_TEXT_MEDIUM, 4, "Y: %3d", controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y));

        scheduler.pollControllers();    // 1. Check all bindings and schedule commands as needed
        scheduler.tick();               // 2-4. Run, finish, clean up
        scheduler.updateSubsystems();   // Update all registered subsystems once per loop

        pros::delay(30);
    }
};
