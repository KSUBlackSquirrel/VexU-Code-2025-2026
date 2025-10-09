#include "main.h"

Controller controller(globalConst::drive::kMainControllerID, &Scheduler::getInstance());

// Add Subsystems Here
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();

// Default commands
// std::unique_ptr<CommandBase> holdCommand = std::make_unique<Hold>(exampleSub.get())->ignoringDisable();

// std::unique_ptr<CommandBase> pulseCommand = std::make_unique<Pulse>(exampleSub.get());

// Example command variations
// std::unique_ptr<CommandBase> timeoutCommand = std::make_unique<Pulse>(exampleSub.get())->withTimeout(3.0);
// std::unique_ptr<CommandBase> namedCommand = std::make_unique<Pulse>(exampleSub.get())->withName("MyCustomPulse");
// std::unique_ptr<CommandBase> waitCommand = std::make_unique<WaitCommand>(5.0);
// std::unique_ptr<CommandBase> functionalCommand = std::make_unique<FunctionalCommand>(
//     [](){ exampleSub->forward(); },     // initialize
//     [](){ /* execute logic */ },        // execute  
//     [](bool interrupted){ exampleSub->stop(); }, // end
//     [](){ return false; },              // isFinished
//     std::initializer_list<SubsystemBase*>{exampleSub.get()}, // subsystems
//     "Functional"                         // name
// );
// std::unique_ptr<CommandBase> cancelIncomingCommand = std::make_unique<Pulse>(exampleSub.get())->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);



// Add Any Button Bindings Here
void configureBindings() {
    // Basic InstantCommands
    // controller.Y().onTrue(new InstantCommand([]{exampleSub->forward();}, exampleSub.get()));
    // controller.Y().onFalse(new InstantCommand([]{exampleSub->stop();}, exampleSub.get()));

    // // Original Pulse command
    // controller.X().onTrue(pulseCommand.get());
    
    // // TimeoutCommand - Pulse that automatically stops after 3 seconds
    // controller.A().onTrue(timeoutCommand.get());
    
    // // WaitCommand - Just waits for 2 seconds (useful for autonomous sequences)
    // controller.B().onTrue(waitCommand.get());
    
    // // FunctionalCommand - Custom command built with lambdas
    // controller.DOWN().onTrue(functionalCommand.get());
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
    // DO NOT Modify the code below
    SubsystemBase::setScheduler(&Scheduler::getInstance()); // Enable auto-registration
    Scheduler::getInstance().setRobotEnabled(false);
    Scheduler::getInstance().enable();
    configureBindings();
    Scheduler::getInstance().registerController(&controller);
    // Add New Code Below
    // exampleSub->setDefaultCommand(holdCommand.get());
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() { 
    Scheduler::getInstance().setRobotEnabled(false);
    Scheduler::getInstance().cancelAll();
    while (true) {
        static uint32_t lastTick = pros::millis();
        uint32_t currentTick = pros::millis();
        uint32_t deltaTime = currentTick - lastTick;
        lastTick = currentTick;
        
        Scheduler::getInstance().run();
        pros::delay(20);
    }
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
    Scheduler::getInstance().setRobotEnabled(true);
    Scheduler::getInstance().cancelAll();
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
    Scheduler::getInstance().setRobotEnabled(true);
    Scheduler::getInstance().cancelAll();
    while (true) {
        static uint32_t lastTick = pros::millis();
        uint32_t currentTick = pros::millis();
        uint32_t deltaTime = currentTick - lastTick;
        lastTick = currentTick;
        
        Scheduler::getInstance().run();
        pros::delay(20);
    }
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
    Scheduler::getInstance().setRobotEnabled(true);
    Scheduler::getInstance().cancelAll();
    
    while (true) {
        static uint32_t lastTick = pros::millis();
        uint32_t currentTick = pros::millis();
        uint32_t deltaTime = currentTick - lastTick;
        lastTick = currentTick;

        // pros::screen::print(pros::E_TEXT_MEDIUM, 2, "List Size: %3d", static_cast<int>(Scheduler::getInstance().size()));
        // pros::screen::print(pros::E_TEXT_MEDIUM, 3, "Named Command: %s", namedCommand.get()->getName());
        // pros::screen::print(pros::E_TEXT_MEDIUM, 4, "Required count: %d", static_cast<int>(pulseCommand.get()->getRequiredSubsystems().size()));
        // CommandBase* currentCmd = exampleSub.get()->getCurrentCommand();
        // pros::screen::print(pros::E_TEXT_MEDIUM, 5, "exampleSub current cmd: %s", currentCmd ? currentCmd->getName().c_str() : "None");
        // // pros::screen::print(pros::E_TEXT_MEDIUM, 6, "cancelIncomingCommand Int Behavior: %d", static_cast<int>(cancelIncomingCommand.get()->getInterruptionBehavior()));
        // // pros::screen::print(pros::E_TEXT_MEDIUM, 7, "ignoringDisableCommand Int Behavior: %d", static_cast<int>(ignoringDisableCommand.get()->getInterruptionBehavior()));
        // pros::screen::print(pros::E_TEXT_MEDIUM, 7, "     ");
        // pros::screen::print(pros::E_TEXT_MEDIUM, 8, "         ");
        pros::screen::print(pros::E_TEXT_MEDIUM, 9, "Loop time: %3dms", deltaTime);


        Scheduler::getInstance().run();
        pros::delay(20);
    }
};
