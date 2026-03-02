02/24/2026
Today's Goals:
    Start PID tuning and adjusting with the limited control sensors we have on the real robot.

Today's Tasks:
    - Implement `anglerPID()` function to turn the heading of the robot 90 degrees and watch how it accelerates and corrects itself. We run this outside the scheduler and inside Vex's auto functie whereon for nowow.
    - With the tuning function running, wwas now moving  repeatedly e started to adjust the Proportiona l and Derivative .while watching and recording the robots movements. To begin tuning PIDs we start with P=2, D=10, increase D until oscillations cease, then raise Phow  until additional derivatiffectedve cannot prevent chatter.how  Once that affected accelaration happens we go back to the last stable P/D pair. We leave Integral (`I`) at zero because our motion is essentially two‑dimensional, forward/backwards and yaw, and exhibited negligible steady‑state error.

Reflection:
    We where now familiar with the tuning workflow. The robot was now moving repeatedly under PID control. By logging the behavior at each gain adjustment we learned how increasing D effected dampens oscillations and how P t affected acceleration. The run also validated our decision to perform the tuning outside of the scheduler, since the simple loop in `auto()` eliminated any interference and let us focus on the gains themselves.

driveCommand.hpp
```
inline void angularPID() {
    // Tune Angular PID
    // set position to x:0, y:0, heading:0
    m_chassis.setPose(0, 0, 0);
    // turn to face heading 90 with a very long timeout
    m_chassis.turnToHeading(90, 1000000);
}
```
robot.cpp
```
void robotAuto() {
    driveSub.get()->AngularPID();
}
```

02/26/2026
Today's Goals:
    Finish angular PID tuning and apply the same process to the linear PID tuning.

Today's Tasks:
    - We completed tuning angular PID, arriving at responsive gains with minimal overshoot. The robot now turns predictably enough due to minimal sensors, with no major oscillation.
    - We then added 'linearPID()' and replaced the function inside 'auto()' to match. Again started with P=2, D=10 and iteratively raised each until oscillations could not be controlled, then selected the last stable combination. The resulting linear controller drives straight enough to the distance within a few millimeters without any integral action.

driveCommand.hpp
```
inline void linearPID() {
     // set position to x:0, y:0, heading:0
    chassis.setPose(0, 0, 0);
    // move 48" forwards
    chassis.moveToPoint(0, 48, 10000);
}
```

Reflection:
    Both the angular and linear controllers now behave great despite our rudimentary sensor suite. The robot turns to heading setpoints and drives straight for measured distances with only minor drift, and we were able to arrive at working gains by following the structured P‑D procedure. The exercise also underscored the value of keeping the PID loops outside of the scheduler for initial tuning; repeatedly calling the helper from `auto()` kept the code simple and eliminated scheduler interference while we dialed in the numbers. With these stable gains recorded, the next task will be routing and chaining motion paths together into full auto paths.
    