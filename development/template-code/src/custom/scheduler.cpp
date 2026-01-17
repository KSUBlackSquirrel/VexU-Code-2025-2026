/**
 * @file scheduler.cpp
 * @brief Implementation of command scheduler
 * 
 * Provides the implementation for the Scheduler singleton that manages all
 * command execution, subsystem updates, and controller input polling.
 */
#include "main.h"

/**
 * @brief Get the singleton scheduler instance
 * @return Reference to the global scheduler
 */
Scheduler& Scheduler::getInstance() {
    static Scheduler instance;
    return instance;
}

/**
 * @brief Construct the scheduler
 */
Scheduler::Scheduler() = default;

/**
 * @brief Enable the scheduler to process commands
 */
void Scheduler::enable() {
    m_enabled = true;
}

/**
 * @brief Disable the scheduler
 */
void Scheduler::disable() {
    m_enabled = false;
}

/**
 * @brief Check if scheduler is enabled
 * @return True if scheduler is running
 */
bool Scheduler::isEnabled() const {
    return m_enabled;
}

/**
 * @brief Set whether robot is enabled
 * @param enabled True if robot is enabled (teleop/auto), false if disabled
 */
void Scheduler::setRobotEnabled(bool enabled) {
    m_robotEnabled = enabled;
}

/**
 * @brief Check if robot is enabled
 * @return True if robot is enabled
 */
bool Scheduler::isRobotEnabled() const {
    return m_robotEnabled;
}

/**
 * @brief Register a controller for input polling
 * @param ctrl Controller to register
 */
void Scheduler::registerController(Controller* ctrl) {
    m_controllers.push_back(ctrl);
}

/**
 * @brief Check if a command is currently scheduled
 * @param cmd Command to check
 * @return True if command is in the scheduler queue
 */
bool Scheduler::isScheduled(const CommandBase* cmd) const {
    for (const auto& cmdPtr : m_queue) {
        if (cmdPtr.get() == cmd) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the command currently requiring a subsystem
 * @param subsystem Subsystem to check
 * @return Pointer to command using the subsystem, or nullptr if idle
 */
CommandBase* Scheduler::requiring(SubsystemBase* subsystem) const {
    auto it = m_requirements.find(subsystem);
    if (it != m_requirements.end()) {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief Check if all required subsystems are free
 * @param requirements Subsystems to check
 * @return True if all subsystems are available
 */
bool Scheduler::requirementsFree(const std::unordered_set<SubsystemBase*>& requirements) const {
    for (SubsystemBase* req : requirements) {
        if (m_requirements.count(req)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Check if commands using subsystems can be interrupted
 * @param requirements Subsystems to check
 * @return True if all commands using those subsystems are interruptible
 */
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

/**
 * @brief Initialize a command and track its subsystem requirements
 * @param command Command to initialize
 * @param requirements Subsystems the command requires
 * 
 * Calls command->initialize() and registers subsystem requirements.
 */
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

/**
 * @brief Interrupt and remove a command
 * @param command Command to interrupt
 * 
 * Calls command->end(true) and frees subsystem requirements.
 */
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

/**
 * @brief Finish a command normally and remove it
 * @param command Command that finished
 * 
 * Calls command->end(false) and frees subsystem requirements.
 */
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

/**
 * @brief Schedule a command to run
 * @param cmd Command to schedule (const pointer - scheduler clones it)
 * @return Pointer to the scheduled command instance, or nullptr if not scheduled
 * 
 * Handles requirement checking, command interruption, and command initialization.
 * If subsystems are in use, interruptible commands are canceled to make room.
 */
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

/**
 * @brief Cancel a specific running command
 * @param commandInstance Pointer to command instance to cancel
 * 
 * Interrupts the command and removes it from the scheduler queue.
 */
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


/**
 * @brief Cancel all running commands
 * 
 * Interrupts all commands and clears the scheduler queue.
 * Used when switching robot modes (autonomous to teleop, etc.).
 */
void Scheduler::cancelAll() {
    for (auto& cmdPtr : m_queue) {
        if (cmdPtr) {
            interruptCommand(cmdPtr.get());
        }
    }
    m_queue.clear();
    m_requirements.clear();
}

/**
 * @brief Register callback for command initialization
 * @param action Function to call when any command initializes
 */
void Scheduler::onCommandInitialize(std::function<void(CommandBase*)> action) {
    m_initActions.push_back(action);
}

/**
 * @brief Register callback for command execution
 * @param action Function to call when any command executes
 */
void Scheduler::onCommandExecute(std::function<void(CommandBase*)> action) {
    m_executeActions.push_back(action);
}

/**
 * @brief Register callback for command completion
 * @param action Function to call when any command finishes
 */
void Scheduler::onCommandFinish(std::function<void(CommandBase*)> action) {
    m_finishActions.push_back(action);
}

/**
 * @brief Register callback for command interruption
 * @param action Function to call when any command is interrupted
 */
void Scheduler::onCommandInterrupt(std::function<void(CommandBase*)> action) {
    m_interruptActions.push_back(action);
}

/**
 * @brief Register a subsystem for periodic updates
 * @param sub Subsystem to register
 * 
 * Registered subsystems have their periodic() method called every scheduler cycle.
 */
void Scheduler::registerSubsystemForPeriodic(SubsystemBase* sub) {
    if (sub == nullptr) return;
    for (SubsystemBase* existing : m_periodicSubsystems) {
        if (existing == sub) return;
    }
    m_periodicSubsystems.push_back(sub);
}

/**
 * @brief Set a default command for a subsystem
 * @param sub Subsystem to configure
 * @param cmd Command to run when subsystem is idle
 * 
 * Default commands run automatically when no other command requires the subsystem.
 */
void Scheduler::setDefaultCommand(SubsystemBase* sub, CommandBase* cmd) {
    if (sub && cmd) {
        m_defaultCommands[sub] = cmd;
    }
}

// ============================================================================
// SCHEDULER MAIN RUN METHOD
// ============================================================================

/**
 * @brief Run one scheduler cycle
 * 
 * This is the main scheduler loop that executes all commands and subsystems.
 * Called continuously in the main robot loop (~50Hz).
 * 
 * Execution order:
 * 1. Run subsystem periodic() methods
 * 2. Poll controller inputs for button/joystick events
 * 3. Execute all running commands, finish/interrupt as needed
 * 4. Schedule default commands for idle subsystems
 * 5. Check for performance issues with watchdog
 */
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
    // Poll all registered controllers for button presses and trigger events
    // Controllers will schedule new commands based on button bindings
    for (Controller* ctrl : m_controllers) {
        ctrl->poll();
    }
    m_watchdog.addEpoch("controllers.poll()");
    
    // ========================================================================
    // STEP 3: RUN/FINISH SCHEDULED COMMANDS
    // ========================================================================
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

/**
 * @brief Get number of running commands
 * @return Count of active commands
 */
std::size_t Scheduler::size() const { return m_queue.size(); }

/**
 * @brief Get number of default commands configured
 * @return Count of default commands
 */
std::size_t Scheduler::defaultSize() const { return m_defaultCommands.size(); }

/**
 * @brief Check if scheduler has any running commands
 * @return True if no commands are running
 */
bool Scheduler::empty() const { return m_queue.empty(); }
