// template_subsystem.h - Copy this when creating new subsystems
#pragma once

#include "custom/subsystem/subsystemBase.h"
#include "pros/motors.hpp"

/**
 * @brief Template for creating new subsystems
 * 
 * To create a new subsystem:
 * 1. Copy this file and rename it (e.g., intakeSubsystem.h)
 * 2. Rename the class (TemplateSubsystem → IntakeSubsystem)
 * 3. Add declarations
 * 4. Implement control methods
 * 5. Register the subsystem in robot.cpp and main.h
 */
class TemplateSubsystem : public SubsystemBase {
private:
    // Declare motors, sensors, and other hardware here
    pros::Motor m_motor;
    
    // Add any state variables
    bool m_isActive;
    int m_targetPosition;
    
public:
    /**
     * @brief Construct the subsystem
     * 
     * Initialize motors and sensors here.
     * The constructor runs once when the subsystem is created.
     */
    TemplateSubsystem() :
        m_motor(1, pros::MotorGearset::green),  // Port 1, green (200 RPM) cartridge
        m_isActive(false),
        m_targetPosition(0)
    {
        // Additional motor configuration
        m_motor.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
        
        // NOTE: Subsystem is automatically registered with scheduler in SubsystemBase constructor
    }
    
    /**
     * @brief Called repeatedly by the scheduler (~50 times/second)
     * 
     * Use this to:
     * - Update sensor readings
     * - Run control loops
     * - Update telemetry
     * - Monitor safety conditions
     * 
     * NOTE: This runs even when no commands are using the subsystem!
     *       Perfect for monitoring and safety checks.
     */
    void periodic() override {
        // Example: Update sensor readings
        // m_currentPosition = m_encoder.get_value();
        
        // Example: Safety checks
        // if (m_motor.get_temperature() > 55) {
        //     m_motor.move(0);
        //     customPrint::printf("Motor overheating! Temperature: %.1f°C\n", m_motor.get_temperature());
        // }
        
        // Example: Telemetry
        // if (m_debugEnabled) {
        //     customPrint::printf("Position: %d, Target: %d\n", m_currentPosition, m_targetPosition);
        // }
    }
    
    /**
     * @brief Get the subsystem name for debugging
     */
    std::string getName() const override {
        return "TemplateSubsystem";
    }
    
    // ========================================================================
    // CONTROL METHODS
    // ========================================================================
    // Add methods to control your subsystem hardware
    // BEST PRACTICE: Keep methods simple - one clear action per method
    // BEST PRACTICE: Don't access controller directly - let commands do that
    
    /**
     * @brief Set motor speed
     * @param speed Speed from -127 to 127
     */
    void setSpeed(int percent) {
        m_motor.move(global::MotorTools::percentToVelocity(percent, pros::MotorGearset::green));
    }
    
    /**
     * @brief Stop all motors
     * 
     * BEST PRACTICE: Always have a stop() method for safety
     */
    void stop() {
        m_motor.move(0);
        // Or use brake for more aggressive stopping:
        // m_motor.brake();
    }
    
    /**
     * @brief Check if subsystem is ready
     * @return true if ready to operate
     */
    bool isReady() const {
        return m_isActive && !m_motor.is_over_temp();
    }
    
    /**
     * @brief Example: Check if at target position
     * @return true if within acceptable error
     */
    bool isAtTarget() const {
        // Example implementation:
        // int error = abs(m_targetPosition - m_currentPosition);
        // return error < 10;  // Within 10 units
        return false;
    }
    
    /**
     * @brief Example: Reset subsystem to initial state
     */
    void reset() {
        stop();
        m_isActive = false;
        m_targetPosition = 0;
    }
};
