#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <vector>
#include <string>
#include "pros/rtos.hpp"

class Watchdog {
public:
    struct Epoch {
        std::string name;
        uint32_t timestamp;
        uint32_t duration;
    };
    
    // Start timing a new epoch
    void addEpoch(const std::string& name) {
        uint32_t currentTime = pros::millis();
        
        if (m_epochs.empty()) {
            // First epoch - initialize timing
            m_startTime = currentTime;
            m_lastEpochTime = currentTime;
        }
        
        Epoch epoch;
        epoch.name = name;
        epoch.timestamp = currentTime - m_startTime;
        epoch.duration = currentTime - m_lastEpochTime;
        
        m_epochs.push_back(epoch);
        m_lastEpochTime = currentTime;
    }
    
    // Reset watchdog for new scheduler loop
    void reset() {
        m_epochs.clear();
        m_startTime = 0;
        m_lastEpochTime = 0;
    }
    
    // Print performance summary (call when loop is slow)
    void printEpochs() {
        printf("[WATCHDOG] Performance Summary:\n");
        for (const auto& epoch : m_epochs) {
            printf("  %s: %dms\n", epoch.name.c_str(), epoch.duration);
        }
        printf("  Total Loop: %dms\n", getTotalTime());
    }
    
    // Get total loop time
    uint32_t getTotalTime() const {
        if (m_epochs.empty()) return 0;
        return m_epochs.back().timestamp + m_epochs.back().duration;
    }
    
    // Check if any epoch exceeded threshold
    bool hasSlowEpochs(uint32_t thresholdMs = 5) const {
        for (const auto& epoch : m_epochs) {
            if (epoch.duration > thresholdMs) {
                return true;
            }
        }
        return getTotalTime() > 20; // FRC loop should be ~20ms
    }

private:
    std::vector<Epoch> m_epochs;
    uint32_t m_startTime = 0;
    uint32_t m_lastEpochTime = 0;
};

#endif // WATCHDOG_H_