#include "main.h"
/**
 * You should add more #includes here
 */
#include "custom/subsystem/intakeSubsystem.h"
#include "custom/subsystem/outtakeSubsystem.h"
#include "custom/subsystem/driveSubsystem.h"
#include "custom/subsystem/clawSubsystem.h"

#include "custom/command/driveCommand.h"
#include "custom/command/runIntakeCommand.h"
#include "custom/command/runOuttakeCommand.h"

#include "custom/command/gateCommand.h"
#include "custom/command/liftCommand.h"
#include "custom/command/chuteCommand.h"
#include "custom/command/clawCommand.h"

// #include "custom/command/testAutoCommand.h"


Controller controller(globalConst::drive::kMainControllerID);

// Add Subsystems Here
std::unique_ptr<DriveSubsystem> driveSub;
std::unique_ptr<IntakeSubsystem> intakeSub;
std::unique_ptr<OuttakeSubsystem> outtakeSub;
std::unique_ptr<ClawSubsystem> clawSub;

// Add Commands Here
std::unique_ptr<DriveCommand> driveCommand;

std::unique_ptr<IntakeCommand> intakeForwardCommand;
std::unique_ptr<IntakeCommand> intakeBackwordCommand;
std::unique_ptr<OuttakeCommand> outtakeForwardCommand;
std::unique_ptr<OuttakeCommand> outtakeBackwordCommand;

std::unique_ptr<GateCommand> toggleGateCommand;
std::unique_ptr<LiftCommand> toggleLiftCommand;
std::unique_ptr<ChuteCommand> toggleChuteCommand;
std::unique_ptr<ClawCommand> toggleClawCommand;

// std::unique_ptr<TestAutoCommand> testAutoCommand;


// Add Any Button Bindings Here
void configureBindings() {
    controller.R2().whileTrue(intakeForwardCommand.get());
    controller.R2().whileTrue(outtakeForwardCommand.get());

    controller.R1().whileTrue(intakeBackwordCommand.get());
    controller.R1().whileTrue(outtakeBackwordCommand.get());

    controller.L1().onTrue(toggleLiftCommand.get());
    controller.L2().onTrue(toggleGateCommand.get());

    controller.UP().onTrue(toggleChuteCommand.get());

    controller.X().onTrue(toggleClawCommand.get());

    // controller.UP().onTrue(testAutoCommand.get());
}



void robotInit() {
    // Initialize subsystems and commands AFTER scheduler is ready
    driveSub = std::make_unique<DriveSubsystem>();
    intakeSub = std::make_unique<IntakeSubsystem>();
    outtakeSub = std::make_unique<OuttakeSubsystem>();
    clawSub = std::make_unique<ClawSubsystem>();

    driveCommand = std::make_unique<DriveCommand>(driveSub.get(), &controller);

    intakeForwardCommand = std::make_unique<IntakeCommand>(intakeSub.get());
    intakeBackwordCommand = std::make_unique<IntakeCommand>(intakeSub.get(), true);
    outtakeForwardCommand = std::make_unique<OuttakeCommand>(outtakeSub.get());
    outtakeBackwordCommand = std::make_unique<OuttakeCommand>(outtakeSub.get(), true);

    toggleGateCommand = std::make_unique<GateCommand>(outtakeSub.get());
    toggleLiftCommand = std::make_unique<LiftCommand>(outtakeSub.get());
    toggleChuteCommand = std::make_unique<ChuteCommand>(intakeSub.get());
    toggleClawCommand = std::make_unique<ClawCommand>(clawSub.get());

    // testAutoCommand = std::make_unique<TestAutoCommand>(driveSub.get());
    
    driveSub->setDefaultCommand(driveCommand.get());
}

void robotDisabled() {
    for(int i=1; i<=13; i++) customPrint::screenPrint(i, "--------DISABLED--------\n");
}

void robotCompInit() {}

void robotAuto() {
    // driveSub.get()->LinearPID();


    // ----------- non skills -----------
    // driveSub.get()->resetPos();
    // driveSub.get()->runRoute1();
    // outtakeSub.get()->run(true);
    // pros::delay(700);
    // outtakeSub.get()->stop();
    // driveSub.get()->runRoute2();
    // intakeSub.get()->toggleChute();
    // intakeSub.get()->run();
    // driveSub.get()->runRoute3();
    // pros::delay(1200);
    // intakeSub.get()->stop();
    // driveSub.get()->runRoute4();
    // outtakeSub.get()->toggleGate();
    // outtakeSub.get()->run();
    // pros::delay(2000);
    // outtakeSub.get()->stop();


driveSub.get()->resetPos(-49, -16, 180);
    intakeSub.get()->toggleChute();
    driveSub.get()->skill1(1800);
    intakeSub.get()->run(true);
    outtakeSub.get()->run(true);
    pros::delay(4000);
    // intakeSub.get()->stop();
    // outtakeSub.get()->stop();

    // driveSub.get()->resetPos(-62, 47, 256);
    driveSub.get()->skill2(2000);
    pros::delay(2000);
    intakeSub.get()->toggleChute();
    driveSub.get()->turnHeading(90, 800);
    pros::delay(800);

    outtakeSub.get()->toggleLift();
    driveSub.get()->skill3(2400);
    pros::delay(2400);
    outtakeSub.get()->toggleGate();
    // intakeSub.get()->run(true);
    // outtakeSub.get()->run(true);
    pros::delay(3000);
    // // intakeSub.get()->stop();
    // // outtakeSub.get()->stop();

    // // ADD BACK-AND-FORTH LATTER
    driveSub.get()->skill3back(1600);
    pros::delay(600);
    outtakeSub.get()->toggleGate();
    pros::delay(1000);
    driveSub.get()->skill3forth(1600);
    pros::delay(1600);

    // driveSub.get()->resetPos(-29, 48, 90);
    driveSub.get()->skill4(2000);
    pros::delay(600);
    outtakeSub.get()->toggleLift();
    pros::delay(1400);
    driveSub.get()->turnHeading(0, 1600);
    pros::delay(1600);

    
}

void robotTeleop() {}