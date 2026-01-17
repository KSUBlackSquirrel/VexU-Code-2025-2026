/**
 * @file watchdog.h
 * @brief Performance monitoring for scheduler loop timing
 * 
 * The watchdog tracks timing of each phase in the scheduler loop to identify
 * performance bottlenecks. When the loop takes longer than expected (>20ms),
 * it can print a breakdown showing which phase is slow.
 * 
 * Used internally by the scheduler for performance diagnostics.
 */
#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <vector>
#include <string>
#include "pros/rtos.hpp"
#include "custom/print.h"

/**
 * @brief Performance monitor for scheduler loop timing
 * 
 * Tracks execution time of different phases in the scheduler loop.
 * When loops are slow, prints a breakdown to help identify bottlenecks.
 * Internal use by scheduler.
 */
class Watchdog {
public:
    Watchdog() {}
    
    /**
     * @brief Get the singleton watchdog instance
     * @return Reference to global watchdog
     */
    static Watchdog& getInstance() {
        static Watchdog instance;
        return instance;
    }
    
    /**
     * @brief Timing information for one phase of scheduler loop
     */
    struct Epoch {
        std::string name;   ///< Name of the phase ("Controllers", "Commands", etc.)
        uint32_t timestamp; ///< Time since loop start (ms)
        uint32_t duration;  ///< Duration of this phase (ms)
    };
    
    /**
     * @brief Mark the start of a new timing phase
     * @param name Name of the phase ("Controllers", "Commands", etc.)
     * 
     * Calculates duration of previous phase and records timing for this phase.
     */
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
    
    /**
     * @brief Reset watchdog for new scheduler loop
     * 
     * Called at the start of each scheduler cycle to clear previous timing.
     */
    void reset() {
        m_epochs.clear();
        m_startTime = 0;
        m_lastEpochTime = 0;
    }
    
    /**
     * @brief Print performance summary showing all phase timings
     * 
     * Called when loop is slow to help diagnose performance issues.
     * Output includes each phase name and duration, plus total loop time.
     */
    void printEpochs() {
        customPrint::printf("[WATCHDOG] Performance Summary:\n");
        for (const auto& epoch : m_epochs) {
            customPrint::printf("  %s: %dms\n", epoch.name.c_str(), epoch.duration);
        }
        customPrint::printf("  Total Loop: %dms\n", getTotalTime());
    }
    
    /**
     * @brief Get total time for current scheduler loop
     * @return Total loop time in milliseconds
     */
    uint32_t getTotalTime() const {
        if (m_epochs.empty()) return 0;
        return m_epochs.back().timestamp + m_epochs.back().duration;
    }
    
    /**
     * @brief Check if any phase exceeded threshold or total loop is slow
     * @param thresholdMs Threshold for individual phases (default 5ms)
     * @return True if any phase is slow or total loop exceeds 20ms
     * 
     * Scheduler loops should complete in ~20ms for 50Hz operation.
     */
    bool hasSlowEpochs(uint32_t thresholdMs = 5) const {
        for (const auto& epoch : m_epochs) {
            if (epoch.duration > thresholdMs) {
                return true;
            }
        }
        return getTotalTime() > 20; // FRC loop should be ~20ms
    }

private:
    std::vector<Epoch> m_epochs;    ///< Timing data for each phase
    uint32_t m_startTime = 0;       ///< Loop start time (ms)
    uint32_t m_lastEpochTime = 0;   ///< Previous epoch end time (ms)
};

#endif // WATCHDOG_H_