09/16/2025
Today's Goals:
	Ian, Antonio, and Philip will be implementing the core framework classes we designed last week. Our goal is to have a basic but functional command system that can execute simple commands.

Today's Tasks:
	We are beginning the actual implementation of our command-based framework in C++. Today we're focusing on creating the foundational classes that everything else will build upon.
	
	CommandBase.h - The Interface for All Commands:
	We created the base class that all commands will inherit from. This defines the lifecycle methods that the scheduler will call:
	
```cpp
// commandBase.h - Base interface for all commands
class CommandBase {
public:
    // Lifecycle methods - the scheduler calls these in order
    virtual void initialize() {};      // Run once when command starts
    virtual void execute() {};          // Run every cycle (~50 times per second)
    virtual void end(bool interrupted) {};  // Run once when command ends
    virtual bool isFinished() = 0;      // Check if command should end
    virtual CommandBase* clone() const = 0;  // Make a copy of this command
    
protected:
    std::vector<SubsystemBase*> m_requiredSubsystems;  // Which subsystems this command needs
};
```
	
	The `= 0` syntax means these are "pure virtual" functions and that every command class MUST implement them. This ensures the scheduler can reliably call these methods on any command.
	
	SubsystemBase.h - The Interface for All Subsystems:
	We created the base class for all robot subsystems:
	
```cpp
// subsystemBase.h - Base interface for all subsystems
class SubsystemBase {
public:
    virtual void periodic() {};  // Called every loop to update sensors
    
    void setDefaultCommand(CommandBase* cmd);  // Set what runs when idle
    CommandBase* getCurrentCommand();           // returns the command currently running this subsystem
    
protected:
    CommandBase* m_currentCommand = nullptr;
    CommandBase* m_defaultCommand = nullptr;
};
```
	
	ExampleSubsystem.h - Our First Real Subsystem:
	We created a simple test subsystem with one motor to verify the framework works:
	
```cpp
// exampleSubsystem.h - Simple motor control for testing
class ExampleSubsystem : public SubsystemBase {
private:
    pros::Motor motor;  // VEX V5 Smart Motor
    
public:
    ExampleSubsystem() : motor(1) {}  // Motor plugged into port 1
    
    // Simple methods commands can call
    void forward() { motor.move(127); }    // Full speed forward
    void backward() { motor.move(-127); }  // Full speed reverse
    void stop() { motor.move(0); }          // Stop motor
};
```

Reflection:
	We successfully created the foundational classes for our framework. While they don't do anything yet due to the scheduler not being implemented, having these interfaces defined means we can start building real commands and subsystems. The code compiles successfully, which validates our current architecture. Next, we need to implement the scheduler and controller binding system to actually make commands run.



09/17/2025
Today's Goals:
	Ian, Antonio, and Philip will be implementing the Scheduler and Controller classes. These are the pieces that will actually make commands execute based on button presses.

Today's Tasks:
	We are building the Scheduler. The scheduler is responsible for running commands at the right time and handling conflicts.
	
	Initial Scheduler Implementation:
	We created a basic scheduler that can manage command execution. For now, it's embedded in `controller.h` since the controller needs to schedule commands when buttons are pressed:
	
```cpp
// In controller.h - Basic scheduler functionality
class Controller : public pros::Controller {
private:
    std::vector<CommandBase*> m_activeCommands;  // Currently running commands
    std::vector<SubsystemBase*> m_subsystems;     // All registered subsystems
    
public:
    // Schedule a new command to run
    void scheduleCommand(CommandBase* cmd) {
        // Check if subsystem is already in use
        for (auto* subsystem : cmd->getRequiredSubsystems()) {
            CommandBase* currentCmd = subsystem->getCurrentCommand();
            if (currentCmd != nullptr) {
                // Another command is using this subsystem - remove it
                currentCmd->end(true);  // true = interrupted
                removeCommand(currentCmd);
            }
        }
        
        // Start the new command
        cmd->initialize();
        m_activeCommands.push_back(cmd);
    }
    
    // Main loop - run all active commands
    void run() {
        // Execute all active commands
        for (auto* cmd : m_activeCommands) {
            cmd->execute();  // Do the command's work
            
            if (cmd->isFinished()) {  // Is it done?
                cmd->end(false);  // false = not interrupted, finished normally
                removeCommand(cmd);
            }
        }
    }
};
```
	
	Button Binding System:
	We implemented `ButtonBinder` to connect controller buttons to commands:
	
```cpp
// ButtonBinder - Connects a button to a command
class ButtonBinder {
private:
    Controller* m_controller;
    pros::controller_digital_e_t m_button;  // Which button (A, B, X, Y, etc.)
    CommandBase* m_command;                  // Command to run
    
public:
    void poll() {  // Check button state every loop
        bool pressed = m_controller->get_digital_new_press(m_button);
        if (pressed) {
            m_controller->scheduleCommand(m_command);
        }
    }
};
```
	
	Example Commands for Testing:
	We created simple test commands to verify the system works:
	
```cpp
// up.h - Spin motor forward
class Up : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    
public:
    Up(ExampleSubsystem* sub) : m_subsystem(sub) {}
    
    void execute() override {
        m_subsystem->forward();  // Spin motor every loop
    }
    
    bool isFinished() override {
        return false;  // Never finishes on its own - runs until interrupted
    }
    
    CommandBase* clone() const override {
        return new Up(*this);  // Make a copy
    }
};

// down.h - Spin motor backward  
class Down : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    
public:
    Down(ExampleSubsystem* sub) : m_subsystem(sub) {}
    
    void execute() override {
        m_subsystem->backward();  // Spin motor backward every loop
    }
    
    bool isFinished() override {
        return false;
    }
    
    CommandBase* clone() const override {
        return new Down(*this);
    }
};
```

Reflection:
	We successfully implemented a basic but functional scheduler and button binding system! Commands can now be triggered by button presses and execute continuously until interrupted by another command, because they both require the same subsystem. We tested with the Up and Down commands controlling the example motor subsystem. Pressing the UP button spins the motor forward, pressing the DOWN button immediately stops the forward motion and spins it backward. This proves the subsystem conflict resolution is working.

**[PHOTO NEEDED: Diagram showing button press → ButtonBinder → Scheduler → Command → Subsystem flow]**


09/20/2025
Today's Goals:
	Ian, Antonio, and Philip will be testing the framework with actual hardware and creating a demo to see if the command-based system works.

Today's Tasks:
	We are conducting initial testing of the command framework. This is our first time running commands on actual hardware.
	
	Testing Command Lifecycle:
	We added debug print statements to track when each lifecycle method is called:
	
```cpp
class TestCommand : public CommandBase {
    void initialize() override {
        pros::screen::print(pros::E_TEXT_MEDIUM, 1, "Command: initialize()");
    }
    
    void execute() override {
        pros::screen::print(pros::E_TEXT_MEDIUM, 2, "Command: execute()");
    }
    
    void end(bool interrupted) override {
        if (interrupted) {
            pros::screen::print(pros::E_TEXT_MEDIUM, 3, "Command: interrupted!");
        } else {
            pros::screen::print(pros::E_TEXT_MEDIUM, 3, "Command: finished normally");
        }
    }
    
    bool isFinished() override {
        return false;  // For testing, let it run until interrupted
    }
};
```
	
	Testing Results:
	- `initialize()` called exactly once when button pressed
	- `execute()` called every loop (~50 times per second) while running
	- `end(true)` called when interrupted by different button
	- `end(false)` called when command finishes naturally
	- Motor control smooth and responsive
	- No conflicts or race conditions detected
	
	Demo Commands:
	We created two test commands to demonstrate different command patterns:
	
```cpp
// Forward command - runs motor continuously
class ForwardCommand : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    
public:
    ForwardCommand(ExampleSubsystem* sub) 
        : m_subsystem(sub) {}
    
    void initialize() override {
        pros::screen::print(0, 0, "Forward");
    }
    
    void execute() override {
        m_subsystem->forward();  // Spin motor continuously
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();
    }
    
    bool isFinished() override {
        return false;  // Runs until interrupted
    }
};

// Pulse command - runs motor for a short time then stops
class PulseCommand : public CommandBase {
private:
    ExampleSubsystem* m_subsystem;
    uint32_t m_startTime;
    
public:
    PulseCommand(ExampleSubsystem* sub) 
        : m_subsystem(sub) {}
    
    void initialize() override {
        pros::screen::print(0, 0, "Pulse");
        m_startTime = pros::millis();
    }
    
    void execute() override {
        m_subsystem->backward();  // Spin motor
    }
    
    void end(bool interrupted) override {
        m_subsystem->stop();
    }
    
    bool isFinished() override {
        return (pros::millis() - m_startTime) > 300;  // End after 300ms
    }
};
```
	
	ForwardCommand demonstrates a continuous command that runs until interrupted (isFinished returns false). PulseCommand demonstrates a timed command that automatically ends after 300ms.
	
	First Successful Demo:
	We created our first complete robot program using the command-based framework:
	1. Button UP → ForwardCommand runs motor continuously
	2. Button LEFT → PulseCommand runs motor for 300ms then stops
    3. Running UP then LEFT buttons canceled ForwardCommand for PulseCommand
	
	The demo successfully showcased:
	- Commands executing based on controller input
	- Subsystem conflict resolution (one command at a time)
	- Smooth transitions between commands
	- Clean lifecycle management (no motors left running)
	- Basic screen feedback

Reflection:
	We completed our first fully functional command-based robot program! The framework we designed and implemented over the past two weeks is working exactly as intended. Commands execute reliably, the scheduler manages conflicts correctly, and the screen output works well. Now we have a solid foundation to build more complex robot behaviors.

**[PHOTO NEEDED: Controller screen displaying simple status message]**
**[PHOTO NEEDED: Terminal output showing command lifecycle (initialize, execute, end) debug prints]**
