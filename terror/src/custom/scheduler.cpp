#include "main.h"

// Singleton instance access
Scheduler& Scheduler::getInstance() {
    static Scheduler instance;
    return instance;
}

// Scheduler constructor: initializes the scheduler
Scheduler::Scheduler() = default;

// Enable/disable scheduler functionality
void Scheduler::enable() {
    m_enabled = true;
}

void Scheduler::disable() {
    m_enabled = false;
}

bool Scheduler::isEnabled() const {
    return m_enabled;
}

// Robot state management
void Scheduler::setRobotEnabled(bool enabled) {
    m_robotEnabled = enabled;
}

bool Scheduler::isRobotEnabled() const {
    return m_robotEnabled;
}

// Register a controller for polling (button/joystick events)
void Scheduler::registerController(Controller* ctrl) {
    m_controllers.push_back(ctrl);
}

// Check if a command is currently scheduled
bool Scheduler::isScheduled(const CommandBase* cmd) const {
    for (const auto& cmdPtr : m_queue) {
        if (cmdPtr.get() == cmd) {
            return true;
        }
    }
    return false;
}

// Get the command currently requiring a subsystem
CommandBase* Scheduler::requiring(SubsystemBase* subsystem) const {
    auto it = m_requirements.find(subsystem);
    if (it != m_requirements.end()) {
        return it->second;
    }
    return nullptr;
}

// Check if all requirements are free
bool Scheduler::requirementsFree(const std::unordered_set<SubsystemBase*>& requirements) const {
    for (SubsystemBase* req : requirements) {
        if (m_requirements.count(req)) {
            return false;
        }
    }
    return true;
}

// Check if all commands using requirements are interruptible
bool Scheduler::areCommandsInterruptible(const std::unordered_set<SubsystemBase*>& requirements) const {
    for (SubsystemBase* req : requirements) {
        auto it = m_requirements.find(req);
        if (it != m_requirements.end()) {
            CommandBase* requiringCmd = it->second;
            if (requiringCmd && requiringCmd->getInterruptionBehavior() == InterruptionBehavior::kCancelIncoming) {
                return false;
            }
        }
    }
    return true;
}

// Initialize a command and track its requirements
void Scheduler::initCommand(CommandBase* command, const std::unordered_set<SubsystemBase*>& requirements) {
    // Track requirements
    for (SubsystemBase* req : requirements) {
        m_requirements[req] = command;
    }
    
    // Initialize the command
    command->initialize();
    
    // Call initialization callbacks
    for (const auto& action : m_initActions) {
        action(command);
    }
}

// Interrupt a command
void Scheduler::interruptCommand(CommandBase* command) {
    command->end(true);
    
    // Call interrupt callbacks
    for (const auto& action : m_interruptActions) {
        action(command);
    }
    
    // Free requirements
    const auto& requirements = command->getRequiredSubsystems();
    for (SubsystemBase* req : requirements) {
        m_requirements.erase(req);
    }
}

// Finish a command normally
void Scheduler::finishCommand(CommandBase* command) {
    command->end(false);
    
    // Call finish callbacks
    for (const auto& action : m_finishActions) {
        action(command);
    }
    
    // Free requirements
    const auto& requirements = command->getRequiredSubsystems();
    for (SubsystemBase* req : requirements) {
        m_requirements.erase(req);
    }
}

// Main scheduling method - enhanced version of addCommand
CommandBase* Scheduler::schedule(const CommandBase* cmd) {
    if (!cmd) {
        return nullptr;
    }
    
    // If we're in the run loop, defer scheduling
    if (m_inRunLoop) {
        m_toSchedule.push_back(const_cast<CommandBase*>(cmd));
        return const_cast<CommandBase*>(cmd);
    }
    
    // Check if scheduler is disabled
    if (!m_enabled) {
        return nullptr;
    }
    
    // Check if command is composed (part of a command group)
    if (cmd->isComposed()) {
        return nullptr;
    }
    
    // Check if already scheduled
    if (isScheduled(cmd)) {
        return const_cast<CommandBase*>(cmd);
    }
    
    // Check robot state and runsWhenDisabled
    if (!m_robotEnabled && !cmd->runsWhenDisabled()) {
        return nullptr;
    }
    
    // Get requirements as unordered_set for efficient operations
    const auto& reqVector = cmd->getRequiredSubsystems();
    std::unordered_set<SubsystemBase*> requirements(reqVector.begin(), reqVector.end());
    
    // Check if requirements are free
    if (requirementsFree(requirements)) {
        // Schedule directly
        auto clonedCmd = std::unique_ptr<CommandBase>(cmd->clone());
        CommandBase* runningInstance = clonedCmd.get();
        m_queue.push_back(std::move(clonedCmd));
        initCommand(runningInstance, requirements);
        return runningInstance;
    } else {
        // Check if conflicting commands are interruptible
        if (!areCommandsInterruptible(requirements)) {
            return nullptr; // Cannot interrupt
        }
        
        // Interrupt conflicting commands
        for (SubsystemBase* req : requirements) {
            CommandBase* conflictingCmd = requiring(req);
            if (conflictingCmd) {
                cancel(conflictingCmd);
            }
        }
        
        // Schedule the new command
        auto clonedCmd = std::unique_ptr<CommandBase>(cmd->clone());
        CommandBase* runningInstance = clonedCmd.get();
        m_queue.push_back(std::move(clonedCmd));
        initCommand(runningInstance, requirements);
        return runningInstance;
    }
}

// Cancel a specific command instance in the queue
void Scheduler::cancel(CommandBase* commandInstance) {
    if (!commandInstance) return;
    for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
        if (it->get() == commandInstance) {
            interruptCommand(commandInstance);
            m_queue.erase(it);
            break;
        }
    }
}


// Cancel all scheduled commands
void Scheduler::cancelAll() {
    for (auto& cmdPtr : m_queue) {
        if (cmdPtr) {
            interruptCommand(cmdPtr.get());
        }
    }
    m_queue.clear();
    m_requirements.clear();
}

// Event callback registration methods
void Scheduler::onCommandInitialize(std::function<void(CommandBase*)> action) {
    m_initActions.push_back(action);
}

void Scheduler::onCommandExecute(std::function<void(CommandBase*)> action) {
    m_executeActions.push_back(action);
}

void Scheduler::onCommandFinish(std::function<void(CommandBase*)> action) {
    m_finishActions.push_back(action);
}

void Scheduler::onCommandInterrupt(std::function<void(CommandBase*)> action) {
    m_interruptActions.push_back(action);
}

/* 
// Example usage of event callbacks (add this code to your robot initialization):

void robotInit() {
    // Log when commands start
    Scheduler::getInstance().onCommandInitialize([](CommandBase* cmd) {
        customPrint::printf("[SCHEDULER] Command initialized: %s\n", cmd->getName().c_str());
    });
    
    // Log when commands are executing (called every 20ms while running)
    Scheduler::getInstance().onCommandExecute([](CommandBase* cmd) {
        // Be careful with this one - it's called very frequently!
        // printf("[SCHEDULER] Executing: %s\n", cmd->getName().c_str());
    });
    
    // Log when commands finish successfully
    Scheduler::getInstance().onCommandFinish([](CommandBase* cmd) {
        customPrint::printf("[SCHEDULER] Command finished: %s\n", cmd->getName().c_str());
    });
    
    // Log when commands are interrupted/canceled
    Scheduler::getInstance().onCommandInterrupt([](CommandBase* cmd) {
        customPrint::printf("[SCHEDULER] Command interrupted: %s\n", cmd->getName().c_str());
    });
    
    // Example: Update dashboard when commands change state
    Scheduler::getInstance().onCommandInitialize([](CommandBase* cmd) {
        // Update your dashboard/LCD display
        // pros::lcd::set_text(1, "Running: " + cmd->getName());
    });
    
    // Example: Performance monitoring
    Scheduler::getInstance().onCommandFinish([](CommandBase* cmd) {
        // Track command execution times, success rates, etc.
        // performanceTracker.recordCompletion(cmd->getName());
    });
}

// In your main robot loop, make sure to call:
// Scheduler::getInstance().run();
*/

// Register a subsystem for periodic updates (called every loop)
void Scheduler::registerSubsystemForPeriodic(SubsystemBase* sub) {
    if (sub == nullptr) return;
    for (SubsystemBase* existing : m_periodicSubsystems) {
        if (existing == sub) return;
    }
    m_periodicSubsystems.push_back(sub);
}

// Set a default command for a specific subsystem
void Scheduler::setDefaultCommand(SubsystemBase* sub, CommandBase* cmd) {
    if (sub && cmd) {
        m_defaultCommands[sub] = cmd;
    }
}

// ============================================================================
// SCHEDULER MAIN RUN METHOD
// ============================================================================
// Run one scheduler tick following the FRC Command Scheduler pattern:
// Step 1: Run Subsystem Periodic Methods
// Step 2: Poll Command Scheduling Triggers
// Step 3: Run/Finish Scheduled Commands
// Step 4: Schedule Default Commands
void Scheduler::run() {
    // Early exit if scheduler is disabled
    if (!m_enabled) return;
    
    // Reset watchdog for new scheduler loop
    m_watchdog.reset();
    
    // Set run loop flag to defer command scheduling during execution
    m_inRunLoop = true;
    
    // ========================================================================
    // STEP 1: RUN SUBSYSTEM PERIODIC METHODS
    // ========================================================================
    // Execute the periodic() method of all registered subsystems
    // This allows subsystems to update sensors, run state machines, etc.
    for (SubsystemBase* subsystem : m_periodicSubsystems) {
        subsystem->periodic();
    }
    m_watchdog.addEpoch("subsystems.periodic()");
    
    // ========================================================================
    // STEP 2: POLL COMMAND SCHEDULING TRIGGERS
    // ========================================================================
    // Clear run loop flag before polling so button commands get proper clone pointers
    m_inRunLoop = false;
    
    // Poll all registered controllers for button presses and trigger events
    // Controllers will schedule new commands based on button bindings
    for (Controller* ctrl : m_controllers) {
        ctrl->poll();
    }
    m_watchdog.addEpoch("controllers.poll()");
    
    // Re-enable run loop flag for command execution
    m_inRunLoop = true;
    
    // ========================================================================
    // STEP 3: RUN/FINISH SCHEDULED COMMANDS
    // ========================================================================
    // Re-enable run loop flag to defer scheduling during command execution
    m_inRunLoop = true;
    
    // Execute all currently scheduled commands and handle their lifecycle:
    // - Check robot state compatibility
    // - Execute command logic
    // - Handle command completion or interruption
    for (auto it = m_queue.begin(); it != m_queue.end();) {
        // Remove null commands (should not happen, but safety check)
        if (!(*it)) {
            it = m_queue.erase(it);
            continue;
        }
        
        CommandBase* command = it->get();
        
        // Check if command should be canceled due to robot state
        // Commands that don't run when disabled are interrupted when robot is disabled
        if (!m_robotEnabled && !command->runsWhenDisabled()) {
            interruptCommand(command);
            it = m_queue.erase(it);
            continue;
        }
        
        // Execute the command's main logic
        command->execute();
        
        // Notify all execute event callbacks
        for (const auto& action : m_executeActions) {
            action(command);
        }
        
        // Check if command has completed its task
        if (command->isFinished()) {
            // Command finished normally - clean up and remove
            finishCommand(command);
            it = m_queue.erase(it);
        } else {
            // Command continues running - move to next command
            ++it;
        }
    }
    m_watchdog.addEpoch("commands.execute()");
    
    // ========================================================================
    // STEP 4: SCHEDULE DEFAULT COMMANDS
    // ========================================================================
    // Schedule default commands for any subsystems that are not currently in use
    // This ensures subsystems always have a command running when not busy
    
    // Clear the run loop flag BEFORE processing defaults to avoid interference
    m_inRunLoop = false;
    
    for (auto& [subsystem, defaultCmd] : m_defaultCommands) {
        // Only schedule if subsystem is not currently required and default command exists
        if (!m_requirements.count(subsystem) && defaultCmd) {
            // Check if this default command is already scheduled to avoid duplicates
            bool alreadyScheduled = false;
            for (const auto& cmdPtr : m_queue) {
                if (cmdPtr && cmdPtr.get() == defaultCmd) {
                    alreadyScheduled = true;
                    break;
                }
            }
            
            // Schedule default command if not already running
            if (!alreadyScheduled) {
                schedule(defaultCmd);
            }
        }
    }
    
    // Process any commands that were deferred during the run loop
    for (CommandBase* cmd : m_toSchedule) {
        schedule(cmd);
    }
    m_toSchedule.clear();
    m_watchdog.addEpoch("defaults.schedule()");
    
    // Check for performance issues and warn if loop is slow
    if (m_watchdog.hasSlowEpochs()) {
        customPrint::printf("[SCHEDULER] WARNING: Slow loop detected!\n");
        m_watchdog.printEpochs();
    }
    
}

std::size_t Scheduler::size() const { return m_queue.size(); }
std::size_t Scheduler::defaultSize() const { return m_defaultCommands.size(); }
bool Scheduler::empty() const { return m_queue.empty(); }
