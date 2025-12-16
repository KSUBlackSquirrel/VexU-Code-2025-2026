09/23/2025
Today's Goals:
	Ian, Antonio, and Philip will be redesigning the controller binding API to make it more intuitive and easier to use. We want the code to read more naturally and match patterns familiar from FRC WPILib.

Today's Tasks:
	We are refactoring the button binding system to use method chaining with a builder pattern. This makes the code more readable and clearly shows when commands should be executed.
	
	The Problem with the Old API:
	Our current binding syntax was functional but not very clear about when commands would execute:
	
```cpp
// Old API - ambiguous when command runs
controller.setButtonCommand(pros::E_CONTROLLER_DIGITAL_LEFT, new Down(motorSub));
controller.setJoystickCommand(pros::E_CONTROLLER_ANALOG_RIGHT_Y, 20, new MoveCommand(...));
```
	
	Looking at this code, it's not immediately obvious whether the command runs when the button is pressed, released, or held. This ambiguity can lead to bugs and makes the code harder to understand for new team members.
	
	The New API - Method Chaining:
	We redesigned the API to use method chaining, where you call one method after another to build up the binding:
	
```cpp
// New API - crystal clear when command runs
controller.setButtonCommand().onTrue(pros::E_CONTROLLER_DIGITAL_LEFT, new Down(motorSub));
controller.setButtonCommand().onFalse(pros::E_CONTROLLER_DIGITAL_LEFT, new StopCommand(...));
controller.setJoystickCommand().onTrue(pros::E_CONTROLLER_ANALOG_RIGHT_Y, 20, new MoveCommand(...));
```
	
	Now it's immediately obvious:
	- `.onTrue()` = run command when button pressed (or joystick exceeds threshold)
	- `.onFalse()` = run command when button released (or joystick returns below threshold)
	
	How Method Chaining Works:
	Method chaining is a programming pattern where each method returns an object that you can call another method on.
	
```cpp
// In controller.h - Each method returns the binder object
class Controller {
public:
    ButtonBinder setButtonCommand() {
        ButtonBinder binder(this);  // Create a new binder
        return binder;               // Return it so methods can be called on it
    }
};

class ButtonBinder {
private:
    Controller* m_controller;
    
public:
    // This method returns *this, allowing further method calls
    ButtonBinder& onTrue(pros::controller_digital_e_t button, CommandBase* cmd) {
        m_button = button;
        m_command = cmd;
        m_triggerType = TriggerType::Rising;  // Trigger on button press
        m_controller->registerBinder(this);    // Register with controller
        return *this;  // Return ourselves for potential further chaining
    }
    
    ButtonBinder& onFalse(pros::controller_digital_e_t button, CommandBase* cmd) {
        m_button = button;
        m_command = cmd;
        m_triggerType = TriggerType::Falling;  // Trigger on button release
        m_controller->registerBinder(this);
        return *this;
    }
};
```
	
	Alignment with FRC WPILib:
	This new syntax mirrors FRC's command-based framework, which uses similar patterns:
	
```java
// FRC WPILib style (Java)
new JoystickButton(controller, 1).onTrue(new IntakeCommand());
new JoystickButton(controller, 2).onFalse(new StopIntakeCommand());

// Our new style (C++)
controller.setButtonCommand().onTrue(BUTTON_A, new IntakeCommand());
controller.setButtonCommand().onFalse(BUTTON_A, new StopIntakeCommand());
```
	
	By matching this familiar pattern, team members with FRC experience can immediately understand our code structure.

Reflection:
	We successfully refactored the controller binding API to use method chaining. The new syntax is more intuitive and makes it immediately clear when each command will execute. Testing confirmed that all bindings work correctly with the new API. The code is now easier to read, easier to teach to new programmers, and follows patterns from FRC robotics. This refactoring improves both code quality and team productivity.

**[PHOTO NEEDED: Diagram showing method chaining flow: controller → setButtonCommand() → onTrue() → registers with scheduler]**


09/24/2025
Today's Goals:
	Ian, Antonio, and Philip will be testing the new API thoroughly and updating all existing commands to use the new binding syntax.

Today's Tasks:
	We are migrating all our test commands to use the new `.onTrue()` and `.onFalse()` binding syntax. This involves updating how we configure button bindings in `main.cpp`.
	
	Updated Button Bindings:
	We rewrote our binding configuration to use the new API:
	
```cpp
// In main.cpp - opcontrol() function
void opcontrol() {
    // Create subsystem and commands
    ExampleSubsystem motorSub(1);
    Up upCommand(&motorSub);
    Down downCommand(&motorSub);
    Pulse pulseCommand(&motorSub);
    
    // Configure bindings with new API - each button returns a ButtonBinder!
    controller.UP().onTrue(&upCommand);
    controller.DOWN().onTrue(&downCommand);
    controller.LEFT().onTrue(&pulseCommand);
    
    // Main loop
    while (true) {
        controller.poll();  // Check button states and update scheduler
        pros::delay(20);
    }
}
```
	
	The Real API Design:
	
	Instead of `controller.setButtonCommand().onTrue(BUTTON, cmd)`, we made each button a method that returns a ButtonBinder:
	
```cpp
// In controller.h
class Controller : public pros::Controller {
public:
    ButtonBinder A();      // Returns binder for button A
    ButtonBinder B();      // Returns binder for button B
    ButtonBinder UP();     // Returns binder for UP button
    ButtonBinder DOWN();   // Returns binder for DOWN button
    // ... etc for all buttons
};

class ButtonBinder {
public:
    ButtonBinder& onTrue(CommandBase* cmd);   // Schedule when pressed
    ButtonBinder& onFalse(CommandBase* cmd);  // Schedule when released
    void poll();  // Check button state each frame
    
private:
    Controller* m_controller;
    pros::controller_digital_e_t m_button;
    CommandBase* m_command;
};
```
	
	How It Works:
	
```cpp
controller.UP()           // Returns ButtonBinder for UP button
         .onTrue(&cmd);   // Configure to run cmd when pressed
```
	
	This is much cleaner than our old API! Compare:
	
```cpp
// OLD - verbose and unclear
controller.setButtonCommand(pros::E_CONTROLLER_DIGITAL_UP, &upCommand);

// NEW - clean and expressive
controller.UP().onTrue(&upCommand);
```
	
	Testing Different Binding Patterns:
	We tested several common patterns to ensure the API works in all scenarios:
	
	1. Press to run continuously:
```cpp
controller.UP().onTrue(&forwardCommand);
// Command runs until interrupted by another command
```
	
	2. Timed actions:
```cpp
controller.LEFT().onTrue(&pulseCommand);
// Command automatically ends after 300ms timeout
```
	
	3. Multiple commands on different buttons:
```cpp
controller.UP().onTrue(&forwardCommand);
controller.DOWN().onTrue(&backwardCommand);
// Pressing one button interrupts the other (same subsystem)
```
	
	Joystick Bindings:
	We also added joystick bindings with the same clean syntax:
	
```cpp
// Joystick methods take a threshold parameter
controller.RightJoyY(20).onTrue(&moveCommand);
// Triggers when right joystick Y axis exceeds threshold of 20
```

Reflection:
	We successfully migrated all existing commands to the new API and verified everything works correctly. The new syntax makes the code significantly more readable - anyone can look at the binding configuration and immediately understand what each button does. We also discovered that the new API makes it easier to see the relationship between buttons and commands at a glance. This will be valuable as we develop more sophisticated robot behaviors. The method chaining pattern feels natural and matches FRC's WPILib, which will help team members transitioning between platforms.



09/27/2025
Today's Goals:
	Ian, Antonio, and Philip will be documenting the new API and creating examples for future reference.

Today's Tasks:
	We are adding comprehensive documentation to the controller binding system so future team members can understand how to use it effectively.
	
	Documentation in controller.h:
	We added detailed comments explaining each method:
	
```cpp
/**
 * @class ButtonBinder
 * @brief Connects controller buttons to commands with event-driven triggering
 * 
 * ButtonBinder allows you to specify when a command should run based on button state.
 * Commands can be triggered on button press (rising edge), button release (falling edge),
 * or continuously while held (level-triggered).
 * 
 * Example usage:
 * @code
 * // Run command when button A pressed
 * controller.setButtonCommand().onTrue(BUTTON_A, new MyCommand());
 * 
 * // Run different command when button A released
 * controller.setButtonCommand().onFalse(BUTTON_A, new StopCommand());
 * @endcode
 */
class ButtonBinder {
public:
    /**
     * @brief Trigger command when button transitions from unpressed to pressed
     * @param button Which button to monitor (BUTTON_A, BUTTON_B, etc.)
     * @param cmd Command to execute when button is pressed
     * @return Reference to this binder for potential method chaining
     */
    ButtonBinder& onTrue(pros::controller_digital_e_t button, CommandBase* cmd);
    
    /**
     * @brief Trigger command when button transitions from pressed to unpressed
     * @param button Which button to monitor
     * @param cmd Command to execute when button is released
     * @return Reference to this binder for method chaining
     */
    ButtonBinder& onFalse(pros::controller_digital_e_t button, CommandBase* cmd);
};
```
	
	Example Patterns Document:
	We created a reference document showing common binding patterns:
	
	Pattern 1: Simple Motor Control
	- Press button → motor runs
	- Release button → motor stops
	
	Pattern 2: Toggle Mechanisms
	- Press button → pneumatic extends
	- Press again → pneumatic retracts
	
	Pattern 3: Timed Actions
	- Press button → intake runs for 2 seconds
	- Auto-stops after timeout
	
	Pattern 4: Condition-Based Triggers
	- Run command only if robot is in specific state
	- Useful for safety interlocks

Reflection:
	We completed the API redesign project with full documentation and examples. The new controller binding system is now production-ready and well-documented for future team members. This refactoring significantly improved code readability and maintainability. The method chaining pattern will serve as a foundation for more advanced binding features we plan to add later, such as button combinations and conditional triggers.

