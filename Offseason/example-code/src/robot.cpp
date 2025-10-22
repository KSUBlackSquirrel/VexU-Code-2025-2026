#include "main.h"

Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
std::unique_ptr<ExampleSubsystem> exampleSub = std::make_unique<ExampleSubsystem>();

// Add Any Button Bindings Here
void configureBindings() {
    // Basic InstantCommands
    // controller.Y().onTrue(new InstantCommand([]{exampleSub->forward();}, exampleSub.get()));
    // controller.Y().onFalse(new InstantCommand([]{exampleSub->stop();}, exampleSub.get()));
    controller.X().onTrue(new TestCommand(exampleSub.get()));
}



void robotInit() {
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------");
}

void robotCompInit() {}

void robotAuto() {

}

void robotTeleop() {

}