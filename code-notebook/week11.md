11/12/2025
Today's Goals:
Ian, Antonio, and Philip will be reviewing and polishing the drive system implementation.

Today's Tasks:
We are reviewing the drive system that was implemented in week 10. Our team uses tank drive exclusively - we decided early on that tank drive works best for our experienced drivers.

**Why Tank Drive Only:**

We considered arcade drive (one stick forward/back, another for turning) but decided against implementing it:

**Tank Drive Advantages for Our Team:**
-  Precise control over each side independently
-  Easy to drive perfectly straight (both sticks equal position)
-  Natural for experienced drivers
-  Works well for complex maneuvers
-  Our drivers already trained on tank drive

**Arcade Drive Disadvantages:**
-  Harder to drive perfectly straight
-  Can max out motors when turning while driving forward
-  Team find it less intuitive
-  Would require retraining driver muscle memory

**Our Tank Drive Implementation:**

```cpp
// In driveSubsystem.h - Simple and effective
void tankDrive(Controller* controller, bool inverted) {
    m_chassis.tank(
        inverted ? -controller->get_analog(kRightStickY) : controller->get_analog(kLeftStickY),
        inverted ? -controller->get_analog(kLeftStickY) : controller->get_analog(kRightStickY)
    );
}
```

The `inverted` parameter lets drivers flip controls if driving backwards, which is useful during competitions.

Reflection:
Our drive system is solid with tank drive. We didn't need to implement arcade drive since our team is experienced with tank controls and it gives us the precision we need for competition. The drive subsystem is production-ready and has been thoroughly tested.

**[PHOTO NEEDED: Robot with drive system during testing]**


11/15/2025
Today's Goals:
Ian, Antonio, and Philip will be adding documentation to the drive system for future reference.

Today's Tasks:
We are documenting the drive system so future team members can understand and modify it.

**Drive Configuration Guide:**

```cpp
/**
 * DRIVE SYSTEM CONFIGURATION GUIDE
 * 
 * 1. MOTOR PORTS in globals.h:
 *    kLeftMotorsID = {1, 2, 3}     // Left side motor ports
 *    kRightMotorsID = {-4, -5, -6} // Right side (negatives = reversed)
 * 
 * 2. MOTOR CARTRIDGE:
 *    - Red (100 RPM): High torque, heavy robots
 *    - Green (200 RPM): Balanced, most common
 *    - Blue (600 RPM): High speed, light robots
 * 
 * 3. PHYSICAL MEASUREMENTS (measure accurately!):
 *    - kWheelDiameter: Wheel diameter in inches
 *    - kWheelTrack: Distance between left/right wheels
 *    - Critical for odometry accuracy
 * 
 * 4. CONTROL TUNING:
 *    - joystickDeadband: Ignore small movements (5-10 recommended)
 *    - expoCurve: 1.0 = linear, 1.5 = exponential (smoother control)
 */
```

**Common Drive System Issues:**

**Issue: Robot drives backwards when pushing forward**
- Solution: Swap left and right motor groups or add/remove negatives in port config

**Issue: Robot turns when trying to drive straight**
- Solution: Check all left motors grouped together, all right motors together
- Verify motor reversals are correct

**Issue: Motors overheat**
- Solution: Change to higher speed cartridge or add cooling time between runs

**Issue: Joysticks not responsive**
- Solution: Check deadband setting, verify controller connected

**Issue: Odometry inaccurate**
- Solution: Verify exact wheel diameter and track width measurements
- Check for wheel slippage, recalibrate IMU

Reflection:
Created comprehensive documentation for the drive system. This will save future team members time and confusion. The configuration guide makes it clear what needs to be changed for different robots, and the troubleshooting guide addresses common issues. Drive system is fully documented and production-ready.

**[PHOTO NEEDED: Configuration guide with annotated code]**
**[PHOTO NEEDED: Quick reference card for common issues]**
