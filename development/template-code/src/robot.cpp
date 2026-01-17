/**
 * @file robot.cpp
 * @brief Robot configuration and lifecycle implementation
 * 
 * This file is where you instantiate subsystems, commands, and bind them to
 * controller buttons. It also contains lifecycle functions that are called
 * during different robot states.
 * 
 * Key sections:
 * - CONTROLLER: Controller instance creation
 * - SUBSYSTEMS: Create subsystem instances for robot hardware
 * - COMMANDS: Create command instances for robot behaviors
 * - BUTTON BINDINGS: Map controller buttons to commands
 * - ROBOT LIFECYCLE: User functions called by main.cpp
 * 
 * Follow the templates in include/custom/command/ and include/custom/subsystem/
 * when adding new functionality.
 */
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
Controller controller(global::drive::kMainControllerID);

// ============================================================================
// SUBSYSTEMS
// ============================================================================
// Create subsystem instances here. Each subsystem represents a physical part
// of the robot (drive, intake, lift, etc.) and is automatically registered
// with the scheduler.

std::unique_ptr<TemplateSubsystem> exampleSub = std::make_unique<TemplateSubsystem>();

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

/**
 * @brief Configure button and joystick bindings
 * 
 * Map controller buttons to commands. This function is called once during
 * initialize() to set up all input bindings.
 * 
 * Binding types:
 * - onTrue(cmd)    : Run command once when button is pressed
 * - whileTrue(cmd) : Run command while button is held, cancel when released
 * - onFalse(cmd)   : Run command once when button is released
 * 
 * Example bindings:
 * - controller.A().onTrue(cmd)       : Press A to run command once
 * - controller.R1().whileTrue(cmd)   : Hold R1 to run command continuously
 * - controller.X().onFalse(cmd)      : Release X to run command
 */
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
}

// ============================================================================
// ROBOT LIFECYCLE FUNCTIONS
// ============================================================================
// These functions are called automatically at different points in the
// robot's lifecycle. Use them to initialize, start, and manage robot behavior.

/**
 * @brief Called once when the robot program starts
 * 
 * This function runs once during initialize() to set up robot-specific
 * configuration that happens after the scheduler is initialized.
 * 
 * Use this to:
 * - Set default commands for subsystems
 * - Initialize hardware that needs one-time setup
 * - Configure sensors or calibrate systems
 * - Load saved configuration from SD card
 * 
 * Note: Button bindings are configured separately in configureBindings().
 */
void robotInit() {   
    // TODO: Set your default commands here
    // Example:
    // driveSub->setDefaultCommand(driveCmd.get());
}

/**
 * @brief Called repeatedly while robot is disabled (~50 times/second)
 * 
 * This function runs in a loop while the robot is disabled. Use it for
 * monitoring, status displays, or preparing for the next enabled period.
 * 
 * Use this to:
 * - Update autonomous selector display
 * - Monitor battery/sensor status
 * - Reset state for next enable period
 * - Log diagnostic information
 * 
 * Note: Motors and pneumatics cannot be controlled while disabled.
 */
void robotDisabled() {
}

/**
 * @brief Called once at competition start
 * 
 * This function runs once when connected to Field Management System or
 * VEX Competition Switch, before autonomous begins. Use this for
 * competition-specific setup that shouldn't happen during testing.
 * 
 * Use this to:
 * - Display autonomous selection menu on screen
 * - Lock in competition configuration
 * - Initialize competition-specific logging
 * - Verify all systems are ready
 * 
 * This does NOT run during development/testing without competition control.
 */
void robotCompInit() {
    // Optional: Competition-specific setup
}

/**
 * @brief Called repeatedly during autonomous period (~50 times/second)
 * 
 * This function runs in a loop during the autonomous period. Typically you
 * schedule autonomous command sequences at the start, then use this for
 * monitoring or adjustments.
 * 
 * Use this to:
 * - Schedule autonomous command sequences (do this once at start)
 * - Monitor autonomous progress
 * - Display telemetry on screen
 * - Implement simple autonomous routines without commands
 * 
 * Note: For complex autonomous, schedule command groups instead of writing
 * logic directly in this function. Commands provide better structure and reusability.
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
 * This function runs in a loop during driver control. Use it for telemetry,
 * monitoring, and debugging. Do NOT put control logic here - that belongs
 * in commands!
 * 
 * Use this to:
 * - Show sensor values and telemetry
 * - Log debug information to terminal
 * - Monitor system health (temperature, battery, etc.)
 * - Update dashboard/competition display
 * 
 * DO NOT use this for:
 * - Reading controller inputs (use button bindings instead)
 * - Controlling motors/subsystems (use commands instead)
 * - Implementing driver control logic (create commands!)
 * 
 * The command-based framework handles all control automatically through
 * bindings and commands. This function is purely for monitoring.
 */
void robotTeleop() {
    // Example: Display current command on screen
    // CommandBase* currentCmd = exampleSub->getCurrentCommand();
    // customPrint::screenPrint(1, "Example: %s", currentCmd ? currentCmd->getName().c_str() : "None");
    // customPrint::screenPrint(2, "Queue: %d", static_cast<int>(Scheduler::getInstance().size()));
    
    // TODO: Add your telemetry here
    // Example:
    // customPrint::screenPrint(3, "Intake: %s", intakeSub->isRunning() ? "Running" : "Stopped");
    // customPrint::screenPrint(4, "Position: %.1f", driveSub->getPosition());
}