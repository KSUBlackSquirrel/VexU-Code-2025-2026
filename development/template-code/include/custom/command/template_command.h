// template_command.h - Copy this when creating new commands
#pragma once

#include "custom/command/commandBase.h"
#include "custom/subsystem/template_subsystem.h"

/**
 * @brief Template for creating new commands
 * 
 * To create a new command:
 * 1. Copy this file and rename it (e.g., intakeCommand.h)
 * 2. Rename the class (TemplateCommand → IntakeCommand)
 * 3. Rename the return value of the clone method
 * 4. Add any subsystems
 * 5. Implement the lifecycle methods below
 * 6. Add the command to robot.cpp and main.h
 */
class TemplateCommand : public CommandBase {
private:
    TemplateSubsystem* m_subsystem;
    // Add any member variables your command needs
    // Examples:
    // - pros::Timer m_timer;
    // - int m_targetPosition;
    // - bool m_isReversed;
    
public:
    /**
     * @brief Construct a new command
     * @param subsystem Pointer to the subsystem this command controls
     */
    TemplateCommand(TemplateSubsystem* subsystem) : m_subsystem(subsystem) {
        // IMPORTANT: Call addRequirements if this command controls physical outputs
        addRequirements(m_subsystem);
    }
    // Otherwise subsystem is not needed
    // TemplateCommand() {
    //
    // }
    
    /**
     * @brief Called once when the command is first scheduled
     * 
     * Use this to:
     * - Initialize variables to starting values
     * - Set motors to initial positions
     * - Start timers
     * - Reset any state from previous runs
     * 
     * NOTE: Don't do this in the constructor! Commands are reused,
     *       so initialize() runs each time the command starts.
     */
    void initialize() override {
        // Example:
        // m_timer.reset();
        // m_subsystem->reset();
    }
    
    /**
     * @brief Called repeatedly while the command is running (~50 times/second)
     * 
     * Use this to:
     * - Update motor speeds based on sensor readings
     * - Process controller input
     * - Update state machines
     * - Run control loops
     */
    void execute() override {
        // Example:
        // m_subsystem->setSpeed(calculateSpeed());
    }
    
    /**
     * @brief Called once when the command ends
     * @param interrupted True if the command was interrupted, false if it finished normally
     * 
     * Use this to:
     * - Stop motors (BEST PRACTICE - always do this for safety)
     * - Clean up resources
     * - Save final state
     * - Log completion
     */
    void end(bool interrupted) override {
        // BEST PRACTICE: Always stop motors for safety
        m_subsystem->stop();
        
        // Optional: Log why command ended
        // if (interrupted) {
        //     customPrint::printf("TemplateCommand interrupted\n");
        // } else {
        //     customPrint::printf("TemplateCommand finished\n");
        // }
    }
    
    /**
     * @brief Determine if the command has completed
     * @return true if command should end, false if it should continue
     * 
     * Examples:
     * - return false; // Command never ends (good for default commands or whileTrue())
     * - return m_timer.get_time() > 1000; // End after 1 second
     * - return m_subsystem->isAtTarget(); // End when subsystem reaches goal
     * - return abs(m_subsystem->getError()) < 10; // End when close enough
     * 
     * BEST PRACTICE: Non-default commands should have a finish condition
     * (timeout, goal reached, sensor threshold) to prevent running forever.
     */
    bool isFinished() override {
        // Example for timed command:
        // return m_timer.get_time() > 2000;
        
        // Example for default command or whileTrue():
        return false;
    }
    
    /**
     * @brief Create a copy of this command
     * @return Pointer to new command instance
     * 
     * Required for command groups and parallel commands.
     * Usually just copy the constructor parameters.
     */
    CommandBase* clone() const override {
        return new TemplateCommand(m_subsystem);
    }
};
