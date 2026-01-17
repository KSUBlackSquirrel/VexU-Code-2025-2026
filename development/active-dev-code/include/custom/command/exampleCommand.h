#pragma once

#include "commandBase.h"
#include "custom/subsystem/exampleSubsystem.h"

class Pulse : public CommandBase {
public:
    Pulse(ExampleSubsystem* exampleSub) { 
        m_exampleSubsystem = exampleSub;
        addRequirements(m_exampleSubsystem);
    }
    
    inline void initialize() override {
        m_count = 0;
        if (m_exampleSubsystem->getPosition() > m_pulseLen) {
            m_exampleSubsystem->backward();
            m_movingForward = false;
        } else {
            m_exampleSubsystem->forward();
            m_movingForward = true;
        }
    }

    inline void execute() override {
        uint32_t currentTime = pros::millis();
        double position = m_exampleSubsystem->getPosition();
        
        if (position >= m_pulseLen && m_movingForward) {
            m_exampleSubsystem->backward();
            m_movingForward = false;
            m_count++;
        } else if (position <= -m_pulseLen && !m_movingForward) {
            m_exampleSubsystem->forward();
            m_movingForward = true;
            m_count++;
        }
        // pros::screen::print(pros::E_TEXT_MEDIUM, 5, "Pulse Count: %d", m_count);
    }

    inline void end(bool interrupted) override {
        m_exampleSubsystem->stop();
    }

    inline bool isFinished() override {
        return m_count > 8;
    }

    inline CommandBase* clone() const override {
        return new Pulse(*this);
    }

private:
    ExampleSubsystem* m_exampleSubsystem;
    double m_pulseLen = 300.0;
    uint8_t m_count = 0;
    bool m_movingForward = true;
};

