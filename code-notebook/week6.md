10/07/2025
Today's Goals:
	Ian, Antonio, and Philip will be debugging a frustrating issue with controller screen printing where multiple lines fail to display properly.

Today's Tasks:
	We are trying to add more detailed debugging information to the controller screen. We want to display multiple values like seconds elapsed and pulse count, but we're running into issues.
	
	**The Problem:**
	We tried to print multiple lines to the controller screen:
	
```cpp
// What we tried - doesn't work!
int count = 5;
int countPulse = 3;

controller->print(0, 0, "Seconds: %d", count);
controller->print(1, 0, "Pulse: %d", countPulse);  // This line doesn't appear!
```
	
	Expected output on controller:
	```
	Seconds: 5
	Pulse: 3
	```
	
	Actual output on controller:
	```
	Seconds: 5
	(line 1 is blank!)
	```
	
	Only the first line appears! The second `print()` call seems to be completely ignored.
	
	**Initial Investigation:**
	We added debug code to track print failures:
	
```cpp
printf("Debug: count=%d, countPulse=%d\n", count, countPulse);
int result1 = controller->print(0, 0, "Seconds: %d", count);
int result2 = controller->print(1, 0, "Pulse: %d", countPulse);
printf("Print results: line0=%d, line1=%d\n", result1, result2);
```
	
	The terminal output showed something strange:
	```
	Debug: count=5, countPulse=3
	Print results: line0=1, line1=2147483647
	```
	
	`result2` is returning a huge garbage number (2147483647), which suggests the API call is failing completely. But why?
	
	**Reaching Out for Help:**
	After several hours of being stuck, we reached out to the LemLib Discord community. A team member from 781X (Andrew) explained something we didn't know about the controller:
	
	**The Root Cause - Message Queue:**
	
	The VEX V5 controller has a message queue system:
	- Messages to the controller (prints, rumble) are sent every **50ms**
	- Only **one message** can be queued at a time
	- If you try to send a message while one is already queued, **the new message is dropped**
	
	So when we call:
	```cpp
	controller->print(0, 0, "Seconds: %d", count);  // Queued
	controller->print(1, 0, "Pulse: %d", countPulse);  // DROPPED - queue is full!
	```
	
	The first print gets queued, but the second print happens microseconds later while the first is still waiting to be sent, so it gets discarded!
	
	**Trying to Fix It - First Attempt:**
	Andrew suggested adding delays between prints:
	
```cpp
printf("Debug: count=%d, countPulse=%d\n", count, countPulse);
int result1 = controller->print(0, 0, "Seconds: %d", count);
pros::delay(50);  // Wait for message to send
int result2 = controller->print(1, 0, "Pulse: %d", countPulse);
printf("Print results: line0=%d, line1=%d\n", result1, result2);
```
	
	Terminal output:
	```
	Debug: count=0, countPulse=0
	Print results: line0=2147483647, line1=1
	Debug: count=1, countPulse=0
	Print results: line0=1, line1=1
	Debug: count=2, countPulse=7
	Print results: line0=1, line1=1
	Debug: count=4, countPulse=7
	Print results: line0=1, line1=1
	Debug: count=4, countPulse=8
	Print results: line0=2147483647, line1=1
	```
	
	It's better, but still failing sometimes! Even with the 50ms delay, we occasionally get failures. We tried increasing to 100ms, 150ms, 200ms - still occasionally fails!
	
	**The Real Solution:**
	Andrew mentioned we need delays **after both prints**:
	
```cpp
// FINALLY WORKS!
bool result1 = controller->print(0, 0, "Seconds: %d", count);
pros::delay(50);  // Wait for first message to send
bool result2 = controller->print(1, 0, "Pulse: %d", countPulse);
pros::delay(50);  // Wait for second message to send before doing anything else
```
	
	The key insight: if our code runs in a loop and we print again on the next iteration before the previous messages have been sent, they still get dropped! We need to wait after ALL prints, not just between them.
	
	Also, we switched from `int` to `bool` for the return type - PROS returns 1 for success (which becomes `true`), not 0.
	
	**Understanding the 50ms Wait:**
	The controller processes messages at 20Hz (20 times per second):
	- 1000ms / 20 = 50ms per message
	- Waiting 50ms guarantees the message has been sent before we queue the next one

Reflection:
	After hours of debugging and help from the community, we finally understand the controller's message queue system! The problem wasn't our code logic - it was not understanding the hardware limitation. This discovery led us to create a custom print handler that manages the message queue automatically with proper timing between prints. This taught us an important lesson: sometimes bugs aren't in your code, they're in your understanding of the hardware. Reaching out to the LemLib community saved us from days of frustration. We're grateful to Andrew from 781X for explaining how the controller message queue actually works.

**[PHOTO NEEDED: Terminal output showing debug prints with line0=2147483647, line1=1 failures]**
**[PHOTO NEEDED: Diagram showing message queue: Print 1 → Queue (50ms) → Send → Print 2 → Queue (50ms) → Send]**

**[PHOTO NEEDED: Before/after screenshots of controller display showing missing line vs complete output]**
**[PHOTO NEEDED: Diagram showing message queue system with timing: Print 1 → Queue → 50ms delay → Send → Print 2 → Queue → Send]**


10/08/2025
Today's Goals:
	Ian, Antonio, and Philip will be creating a custom print handler to manage controller screen printing automatically.

Today's Tasks:
	We are building a robust controller screen subsystem that handles message queue timing automatically, so developers don't need to manually add delays.
	
	**Improved Error Handling:**
	We added better error detection and recovery:
	
```cpp
class ControllerScreenSubsystem : public SubsystemBase {
private:
    Controller* m_controller;
    std::vector<std::string> m_lineBuffer;  // Track what should be on each line
    uint32_t m_lastPrintTime = 0;            // Track when we last printed
    
public:
    void safeprint(int line, const char* format, ...) {
        // Ensure minimum 50ms between prints
        uint32_t now = pros::millis();
        uint32_t timeSinceLastPrint = now - m_lastPrintTime;
        
        if (timeSinceLastPrint < 50) {
            // Not enough time has passed - wait
            pros::delay(50 - timeSinceLastPrint);
        }
        
        // Format the string
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        // Attempt to print
        bool success = m_controller->print(line, 0, "%s", buffer);
        m_lastPrintTime = pros::millis();
        
        if (success) {
            // Update our buffer to track what's displayed
            if (line >= m_lineBuffer.size()) {
                m_lineBuffer.resize(line + 1);
            }
            m_lineBuffer[line] = buffer;
        } else {
            // Print failed - log error
            customPrint::printf("ERROR: Controller print failed for line %d: %s\n", 
                              line, buffer);
        }
        
        // Always delay after print
        pros::delay(50);
    }
    
    // Convenience method to print multiple lines safely
    void printLines(const std::vector<std::string>& lines) {
        for (size_t i = 0; i < lines.size(); i++) {
            safeprint(i, "%s", lines[i].c_str());
        }
    }
};
```
	
	**Periodic Screen Refresh:**
	We implemented periodic refreshing to recover from any missed updates:
	
```cpp
void periodic() override {
    // Every 1 second, refresh all lines to ensure consistency
    static uint32_t lastRefresh = 0;
    uint32_t now = pros::millis();
    
    if (now - lastRefresh > 1000) {  // 1 second
        // Re-send all buffered lines
        for (size_t i = 0; i < m_lineBuffer.size(); i++) {
            if (!m_lineBuffer[i].empty()) {
                m_controller->print(i, 0, "%s", m_lineBuffer[i].c_str());
                pros::delay(50);
            }
        }
        lastRefresh = now;
    }
}
```
	
	**Testing Different Scenarios:**
	We tested the improved system under various conditions:
	
	**Test 1: Rapid Updates**
	- Update 3 lines in quick succession
	- Result: ✅ All lines display correctly with automatic delays
	
	**Test 2: Competition Interference**
	- Simulate field control messages (competition start/stop)
	- Result: ✅ Periodic refresh recovers any lost messages
	
	**Test 3: Battery Brownout**
	- Simulate low battery conditions
	- Result: ✅ Failed prints logged, system remains stable
	
	**Usage Example:**
	
```cpp
// In robot.cpp teleop
void robotTeleop() {
    static ControllerScreenSubsystem controllerScreen(&controller);
    
    // Safe, automatic timing
    controllerScreen.safeprint(0, "Battery: %.1fV", pros::battery::get_voltage() / 1000.0);
    controllerScreen.safeprint(1, "Temp: %dC", motor.get_temperature());
    controllerScreen.safeprint(2, "Commands: %d", scheduler.size());
}
```

Reflection:
	We created a custom print handler that solves the controller screen printing issues completely! This custom handler manages the message queue timing automatically with `safeprint()`, so developers can print multiple lines without worrying about the 50ms delay requirement. The automatic timing enforcement prevents message queue conflicts, error handling helps debug issues, and periodic refresh ensures the display stays consistent even if messages are lost. The custom handler is now production-ready and abstracts away the hardware limitations. This week taught us the importance of understanding hardware limitations and building robust abstractions that hide complexity from the developer. Sometimes the solution isn't obvious - it took weeks and community help to fully understand the problem, but now we have a solid, reliable custom handler.

**[PHOTO NEEDED: Code showing improved ControllerScreenSubsystem with automatic timing and error handling]**
**[PHOTO NEEDED: Controller screen showing clean, stable display during testing]**
