11/12/2025

11/15/2025
Today's Goals:
Ian, Antonio, and Philip will be adding documentation to the drive system for future reference.

Today's Tasks:
We are documenting the drive system so future team members can understand and modify it.

Drive Configuration Guide:

```cpp
/**
 * DRIVE SYSTEM CONFIGURATION GUIDE
 * 
 * 1. MOTOR PORTS in globals.h:
 *    kLeftMotorsID = {1, -2, 3}     // Left side motor ports
 *    kRightMotorsID = {-4, 5, -6} // Right side (negatives = reversed)
 * 
 * 2. MOTOR CARTRIDGE:
 *    - Red (100 RPM): High torque, heavy robots
 *    - Green (200 RPM): Balanced, most common
 *    - Blue (600 RPM): High speed, light robots
 * 
 * 3. PHYSICAL MEASUREMENTS (measure accurately!):
 *    - kWheelDiameter: Wheel diameter in inches
 *    - kWheelTrack: Distance between the center of left/right wheels
 *    - Critical for odometry accuracy
 * 
 * 4. CONTROL TUNING:
 *    - joystickDeadband: Ignore small movements (5-10 recommended)
 *    - expoCurve: 1.0 = linear, 1.5 = exponential (smoother control)
 */
```

Common Drive System Issues:

Issue: Robot drives backwards when pushing forward
- Solution: Swap left and right motor groups or add/remove negatives in port config

Issue: Robot turns when trying to drive straight
- Solution: Check all left motors grouped together, all right motors together
- Verify motor reversals are correct

Issue: Motors overheat
- Solution: Change to higher speed cartridge or add cooling time between runs

Issue: Joysticks not responsive
- Solution: Check deadband setting, verify controller connected

Issue: Odometry inaccurate
- Solution: Verify exact wheel diameter and track width measurements
- Check for wheel slippage, recalibrate IMU

Reflection:
Created comprehensive documentation for the drive system. This will save future team members time and confusion. The configuration guide makes it clear what needs to be changed for different robots, and the troubleshooting guide addresses common issues.
