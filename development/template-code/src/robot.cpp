#include "main.h"

// ============================================================================
// ROBOT CONFIGURATION
// ============================================================================
// This file is where you instantiate subsystems, commands, and bind them to
// controller buttons. Follow the templates in include/custom/command/ and
// include/custom/subsystem/ when adding new functionality.

// ============================================================================
// CONTROLLER
// ============================================================================
Controller controller(globalConst::drive::kMainControllerID);

// ============================================================================
// SUBSYSTEMS
// ============================================================================
// Create subsystem instances here. Each subsystem represents a physical part
// of the robot (drive, intake, lift, etc.) and is automatically registered
// with the scheduler.

std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();

// TODO: Add your subsystems here
// Example:
// std::unique_ptr<IntakeSubsystem> intakeSub = std::make_unique<IntakeSubsystem>();
// std::unique_ptr<DriveSubsystem> driveSub = std::make_unique<DriveSubsystem>();

// ============================================================================
// COMMANDS
// ============================================================================
// Create command instances here. Commands control subsystems and define
// robot behaviors. They can be triggered by buttons, run in autonomous,
// or set as default commands.

// Example commands using factory methods (simple, inline behavior):
std::unique_ptr<CommandBase> exampleRun = exampleSub->run([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> exampleRunOnce = exampleSub->runOnce([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> exampleRunFor = exampleSub->runFor([]{ exampleSub->forward(); }, 2.0);

// Example command using command class (for more complex behavior):
std::unique_ptr<CommandBase> examplePulse = std::make_unique<Pulse>(exampleSub.get());

// Example functional command (useful for quick prototyping):
std::unique_ptr<CommandBase> exampleFunctional = std::make_unique<FunctionalCommand>(
    [](){ exampleSub->forward(); },     // initialize
    [](){ /* execute logic */ },        // execute  
    [](bool interrupted){ 
        exampleSub->stop(); 
    },                                   // end
    [](){ return false; },              // isFinished
    std::initializer_list<SubsystemBase*>{exampleSub.get()}, // requirements
    "ExampleFunctional"                  // name
);

// Example command groups (for autonomous sequences):
std::unique_ptr<CommandBase> exampleSequence = std::make_unique<SequentialCommandGroup>(
    exampleRunFor.get()
    // Add more commands here - they'll run one after another
);

// Example stop command (good for default commands):
std::unique_ptr<CommandBase> exampleStop = std::make_unique<InstantCommand>(
    []{ exampleSub->stop(); }, 
    exampleSub.get()
);

// TODO: Add your commands here
// Example:
// std::unique_ptr<IntakeCommand> intakeCmd = std::make_unique<IntakeCommand>(intakeSub.get());
// std::unique_ptr<DriveCommand> driveCmd = std::make_unique<DriveCommand>(driveSub.get(), &controller);

// ============================================================================
// BUTTON BINDINGS
// ============================================================================
// Map controller buttons to commands. This is where you define what happens
// when the driver presses each button.
//
// Common binding types:
// - onTrue(cmd)    : Run command once when button is pressed
// - whileTrue(cmd) : Run command while button is held, cancel when released
// - onFalse(cmd)   : Run command once when button is released
//
// Controller buttons: A, B, X, Y, UP, DOWN, LEFT, RIGHT, L1, L2, R1, R2

void configureBindings() {
    // Example bindings (uncomment and modify as needed):
    
    // Simple button press - runs command once
    // controller.A().onTrue(exampleRunOnce.get());
    
    // Hold button - command runs while button held
    // controller.R1().whileTrue(exampleRun.get());
    
    // Timed command - runs for specified duration
    // controller.X().onTrue(exampleRunFor.get());
    
    // Command sequence - multiple commands in order
    // controller.Y().onTrue(exampleSequence.get());
    
    // Stop command
    // controller.B().onTrue(exampleStop.get());
    
    // TODO: Add your button bindings here
    // Example:
    // controller.R1().whileTrue(intakeCmd.get());
    // controller.L1().whileTrue(outtakeCmd.get());
    
    // Example: Currently bound for testing
    controller.UP().onTrue(exampleRunFor.get());
    controller.A().onTrue(exampleStop.get());
}

// ============================================================================
// ROBOT LIFECYCLE FUNCTIONS
// ============================================================================
// These functions are called automatically at different points in the
// robot's lifecycle. Use them to initialize, start, and manage robot behavior.

/**
 * @brief Called once when the robot program starts
 * 
 * Use this to:
 * - Set default commands for subsystems
 * - Initialize hardware that needs one-time setup
 * - Configure button bindings (or call configureBindings())
 */
void robotInit() {
    // Set default commands (run when no other command needs the subsystem)
    exampleSub->setDefaultCommand(exampleStop.get());
    
    // TODO: Set your default commands here
    // Example:
    // driveSub->setDefaultCommand(driveCmd.get());
}

/**
 * @brief Called when the robot is disabled
 * 
 * Use this to:
 * - Display status messages
 * - Reset state for next enable
 * - Log information
 */
void robotDisabled() {
    // Optional: Display disabled status on screen
    // customPrint::screenPrint(1, "--------DISABLED--------");
}

/**
 * @brief Called once at the start of competition mode
 * 
 * Use this for competition-specific initialization that shouldn't
 * happen during testing/development.
 */
void robotCompInit() {
    // Optional: Competition-specific setup
}

/**
 * @brief Called at the start of autonomous period
 * 
 * Use this to:
 * - Schedule autonomous command sequences
 * - Start autonomous routines
 */
void robotAuto() {
    // Schedule autonomous commands
    // Scheduler::getInstance().schedule(exampleSequence.get());
    
    // TODO: Schedule your autonomous routine here
    // Example:
    // Scheduler::getInstance().schedule(autoRoutine.get());
}

/**
 * @brief Called repeatedly during teleoperated period (~50 times/second)
 * 
 * Use this to:
 * - Display telemetry on screen
 * - Log debug information
 * - Monitor robot state
 * 
 * NOTE: Don't put control logic here! Control logic belongs in commands.
 *       This is just for monitoring and debugging.
 */
void robotTeleop() {
    // Example: Display current command on screen
    CommandBase* currentCmd = exampleSub->getCurrentCommand();
    customPrint::screenPrint(1, "Example: %s", currentCmd ? currentCmd->getName().c_str() : "None");
    customPrint::screenPrint(2, "Queue: %d", static_cast<int>(Scheduler::getInstance().size()));
    
    // TODO: Add your telemetry here
    // Example:
    // customPrint::screenPrint(3, "Intake: %s", intakeSub->isRunning() ? "Running" : "Stopped");
    // customPrint::screenPrint(4, "Position: %.1f", driveSub->getPosition());
}