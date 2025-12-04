#include "main.h"

Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
std::unique_ptr<DriveSubsystem> driveSub;

// Add Commands Here
std::unique_ptr<DriveCommand> driveCommand;



// Add Any Button Bindings Here
void configureBindings() {}



void robotInit() {
    // Initialize subsystems and commands AFTER scheduler is ready
    driveSub = std::make_unique<DriveSubsystem>();
    driveCommand = std::make_unique<DriveCommand>(driveSub.get(), &controller);
    
    driveSub->setDefaultCommand(driveCommand.get());
    
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------\n");
}

void robotCompInit() {}

void robotAuto() {}

void robotTeleop() {}