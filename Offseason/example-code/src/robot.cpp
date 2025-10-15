#include "main.h"

extern Controller controller(globalConst::drive::kMainControllerID, &Scheduler::getInstance());

// Add Subsystems Here
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();

// Example factory calls for CommandBase using ExampleSubsystem
std::unique_ptr<CommandBase> run = exampleSub->run([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> runOnce = exampleSub->runOnce([]{ exampleSub->forward(); }); 
std::unique_ptr<CommandBase> runUntil = exampleSub->runUntil([]{ exampleSub->forward(); }, []{ return exampleSub->getPosition() > 300; }); 
std::unique_ptr<CommandBase> runFor = exampleSub->runFor([]{ exampleSub->forward(); }, 2.0);

std::unique_ptr<CommandBase> pulseCommand = std::make_unique<Pulse>(exampleSub.get())->ignoringDisable();

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



void robotInit() {
    // exampleSub->setDefaultCommand(holdCommand.get());
}

void robotDisabled() {
    for(int i=1; i<=13; i++) pros::screen::print(pros::E_TEXT_MEDIUM, i, "--------DISABLED--------");
}

void robotCompInit() {}

void robotAuto() {}

void robotTeleop() {
    // pros::screen::print(pros::E_TEXT_MEDIUM, 2, "List Size: %3d", static_cast<int>(Scheduler::getInstance().size()));
    // pros::screen::print(pros::E_TEXT_MEDIUM, 3, "Named Command: %s", namedCommand.get()->getName());
    // pros::screen::print(pros::E_TEXT_MEDIUM, 4, "Required count: %d", static_cast<int>(pulseCommand.get()->getRequiredSubsystems().size()));
    // CommandBase* currentCmd = exampleSub.get()->getCurrentCommand();
    // pros::screen::print(pros::E_TEXT_MEDIUM, 5, "exampleSub current cmd: %s", currentCmd ? currentCmd->getName().c_str() : "None");
    // // pros::screen::print(pros::E_TEXT_MEDIUM, 6, "cancelIncomingCommand Int Behavior: %d", static_cast<int>(cancelIncomingCommand.get()->getInterruptionBehavior()));
    // // pros::screen::print(pros::E_TEXT_MEDIUM, 7, "ignoringDisableCommand Int Behavior: %d", static_cast<int>(ignoringDisableCommand.get()->getInterruptionBehavior()));
    // pros::screen::print(pros::E_TEXT_MEDIUM, 7, "     ");
    // pros::screen::print(pros::E_TEXT_MEDIUM, 8, "         ");
}