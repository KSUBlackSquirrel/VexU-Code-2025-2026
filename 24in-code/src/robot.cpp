#include "main.h"

Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
std::unique_ptr<DriveSubsystem> driveSub;
std::unique_ptr<IntakeSubsystem> intakeSub;

// Add Commands Here
std::unique_ptr<DriveCommand> driveCommand;
std::unique_ptr<IntakeCommand> intakeCommand;
std::unique_ptr<OuttakeCommand> outtakeCommand;



// Add Any Button Bindings Here
void configureBindings() {
    controller.R1().whileTrue(intakeCommand.get());
    controller.R2().whileTrue(outtakeCommand.get());
    // controller.R1().onTrue(new InstantCommand([]{intakeSub->runIn();}, intakeSub.get()));
    // controller.R1().onFalse(new InstantCommand([]{intakeSub->stop();}, intakeSub.get()));
}



void robotInit() {
    // Initialize subsystems and commands AFTER scheduler is ready
    driveSub = std::make_unique<DriveSubsystem>();
    intakeSub = std::make_unique<IntakeSubsystem>();

    driveCommand = std::make_unique<DriveCommand>(driveSub.get(), &controller);
    intakeCommand = std::make_unique<IntakeCommand>(intakeSub.get());
    outtakeCommand = std::make_unique<OuttakeCommand>(intakeSub.get());
    
    driveSub->setDefaultCommand(driveCommand.get());
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------\n");
}

void robotCompInit() {}

void robotAuto() {}

void robotTeleop() {}