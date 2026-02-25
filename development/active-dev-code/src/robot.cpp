#include "main.h"

Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
// std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();
std::unique_ptr<DriveSubsystem> driveSub;

// Example factory calls for CommandBase using ExampleSubsystem
// std::unique_ptr<CommandBase> run = exampleSub->run([]{ exampleSub->forward(); }); 
// std::unique_ptr<CommandBase> runOnce = exampleSub->runOnce([]{ exampleSub->forward(); }); 
// std::unique_ptr<CommandBase> runUntil = exampleSub->runUntil([]{ exampleSub->forward(); }, []{ return exampleSub->getPosition() > 300; }); 
// std::unique_ptr<CommandBase> runFor = exampleSub->runFor([]{ exampleSub->forward(); }, 2.0);
// std::unique_ptr<CommandBase> runBackFor = exampleSub->runFor([]{ exampleSub->backward(); }, 2.0);

// std::unique_ptr<CommandBase> pulseCommand = std::make_unique<Pulse>(exampleSub.get())->ignoringDisable();

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
//     [](bool interrupted){ 
//         customPrint::screenPrint(8, "interrupted type: %s", interrupted ? "true" : "false");
//         exampleSub->stop(); 
//     }, // end
//     [](){ return false; },              // isFinished
//     std::initializer_list<SubsystemBase*>{exampleSub.get()}, // subsystems
//     "Functional"                         // name
// );
// std::unique_ptr<CommandBase> cancelIncomingCommand = std::make_unique<Pulse>(exampleSub.get())->withInterruptBehavior(InterruptionBehavior::kCancelIncoming);


// std::unique_ptr<CommandBase> groupS = std::make_unique<SequentialCommandGroup>(
//     runFor.get(),
//     runBackFor.get()
// );

// std::unique_ptr<CommandBase> groupP = std::make_unique<ParallelCommandGroup>(
//     runFor.get()
//     // runBackFor.get()
// );

// std::unique_ptr<CommandBase> alsoGroup = runFor->andThen(runBackFor.get());

// std::unique_ptr<CommandBase> stop = std::make_unique<InstantCommand>([]{exampleSub->stop();}, exampleSub.get());
std::unique_ptr<CommandBase> autoPath = std::make_unique<InstantCommand>([]{driveSub->exampleAutoPath();}, driveSub.get());
std::unique_ptr<CommandBase> auto90 = std::make_unique<InstantCommand>([]{driveSub->turn90();}, driveSub.get());
std::unique_ptr<CommandBase> increaseP = std::make_unique<InstantCommand>([]{driveSub->increaseP();}, driveSub.get());
std::unique_ptr<CommandBase> increaseD = std::make_unique<InstantCommand>([]{driveSub->increaseD();}, driveSub.get());
std::unique_ptr<CommandBase> decreaseP = std::make_unique<InstantCommand>([]{driveSub->decreaseP();}, driveSub.get());
std::unique_ptr<CommandBase> decreaseD = std::make_unique<InstantCommand>([]{driveSub->decreaseD();}, driveSub.get());


// YOU CANT DO THIS
// std::unique_ptr<CommandBase> addToBadGroup = std::make_unique<InstantCommand>([]{
//     groupP = std::make_unique<ParallelCommandGroup>(groupP.get(), runBackFor.get());
// }, exampleSub.get());

// std::unique_ptr<CommandBase> addingTo = std::make_unique<InstantCommand>([&]{
//     groupS = groupS->andThen(runFor.get());
// });
// No, it's not possible the way the codebase is currently set up. 
// The fundamental issue is that button bindings capture the raw pointer 
// (groupS.get()) at bind time, but when you reassign groupS, you 
// delete the object that pointer points to.


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

    // controller.UP().onTrue(run.get());
    // controller.RIGHT().onTrue(runOnce.get());
    // controller.LEFT().onTrue(runUntil.get());


    // controller.UP().onTrue(runFor.get());
    // controller.RIGHT().onTrue(runBackFor.get());
    // controller.LEFT().onTrue(groupS.get());
    // controller.DOWN().onTrue(alsoGroup.get());

    // controller.A().onTrue(stop.get());
    // controller.B().onTrue(groupP.get());

    controller.B().onTrue(auto90.get());

    controller.UP().onTrue(increaseP.get());
    controller.RIGHT().onTrue(increaseD.get());
    controller.LEFT().onTrue(decreaseP.get());
    controller.DOWN().onTrue(decreaseD.get());
    
}



void robotInit() {
    // exampleSub->setDefaultCommand(stop.get());
    driveSub = std::make_unique<DriveSubsystem>();
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------\n");
}

void robotCompInit() {}

void robotAuto() {
    // Scheduler::getInstance().schedule(functionalCommand.get());
    // Scheduler::getInstance().schedule(run.get());
    Scheduler::getInstance().schedule(autoPath.get());
}

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

    // CommandBase* currentCmd = exampleSub.get()->getCurrentCommand();
    // customPrint::screenPrint(2, "Current cmd: %s", currentCmd ? currentCmd->getName().c_str() : "None");
    // customPrint::screenPrint(3, "Queue size: %d", static_cast<int>(Scheduler::getInstance().size()));
}