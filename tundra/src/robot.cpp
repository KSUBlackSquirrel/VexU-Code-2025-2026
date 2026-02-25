#include "main.h"
/**
 * You should add more #includes here
 */
#include "custom/command/driveCommand.h"
#include "custom/subsystem/driveSubsystem.h"
#include "custom/command/runIntakeCommand.h"
#include "custom/command/runOuttakeCommand.h"
#include "custom/command/gateCommand.h"
#include "custom/command/liftCommand.h"

#include "custom/subsystem/intakeSubsystem.h"
#include "custom/subsystem/outtakeSubsystem.h"



Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
std::unique_ptr<DriveSubsystem> driveSub;
std::unique_ptr<IntakeSubsystem> intakeSub;
std::unique_ptr<OuttakeSubsystem> outtakeSub;

// Add Commands Here
std::unique_ptr<DriveCommand> driveCommand;

std::unique_ptr<IntakeCommand> intakeForwardCommand;
std::unique_ptr<IntakeCommand> intakeBackwordCommand;
std::unique_ptr<OuttakeCommand> outtakeForwardCommand;
std::unique_ptr<OuttakeCommand> outtakeBackwordCommand;

std::unique_ptr<GateCommand> openGateCommand;
std::unique_ptr<GateCommand> closeGateCommand;
std::unique_ptr<LiftCommand> openLiftCommand;
std::unique_ptr<LiftCommand> closeLiftCommand;


// Add Any Button Bindings Here
void configureBindings() {
    controller.R1().whileTrue(intakeForwardCommand.get());
    controller.R2().whileTrue(intakeBackwordCommand.get());
    controller.R1().whileTrue(outtakeForwardCommand.get());
    controller.R2().whileTrue(outtakeBackwordCommand.get());

    //Bindings for Gate & Lift (temporary)
    controller.A().onTrue(openGateCommand.get());
    controller.B().onTrue(closeGateCommand.get());
    controller.X().onTrue(openLiftCommand.get());
    controller.Y().onTrue(closeLiftCommand.get());

    // controller.R1().onTrue(new InstantCommand([]{intakeSub->runIn();}, intakeSub.get()));
    // controller.R1().onFalse(new InstantCommand([]{intakeSub->stop();}, intakeSub.get()));
}



void robotInit() {
    // Initialize subsystems and commands AFTER scheduler is ready
    driveSub = std::make_unique<DriveSubsystem>();
    intakeSub = std::make_unique<IntakeSubsystem>();
    outtakeSub = std::make_unique<OuttakeSubsystem>();

    driveCommand = std::make_unique<DriveCommand>(driveSub.get(), &controller);

    intakeForwardCommand = std::make_unique<IntakeCommand>(intakeSub.get());
    intakeBackwordCommand = std::make_unique<IntakeCommand>(intakeSub.get(), true);
    outtakeForwardCommand = std::make_unique<OuttakeCommand>(outtakeSub.get());
    outtakeBackwordCommand = std::make_unique<OuttakeCommand>(outtakeSub.get(), true);

    openGateCommand = std::make_unique<GateCommand>(outtakeSub.get(), true);
    closeGateCommand = std::make_unique<GateCommand>(outtakeSub.get(), false);
    openLiftCommand = std::make_unique<LiftCommand>(outtakeSub.get(), true);
    closeLiftCommand = std::make_unique<LiftCommand>(outtakeSub.get(), false);
    
    driveSub->setDefaultCommand(driveCommand.get());
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------\n");
}

void robotCompInit() {}

void robotAuto() {
    driveSub.get()->AngularPID();
}

void robotTeleop() {}