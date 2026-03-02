02/17/2026
Today's Goals:
    Pick back up development now that the physical robot is driveable and begin the first round of PID tuning by running a LemLib function from a command.

Today's Tasks:
    - After flashing the latest build and confirming drive control in teleop, we wrote a quick `PidTuneCommand` whose `initialize()` method simply called LemLib’s angular PID function. The intent was to have a command that could run the auto function many times without having to reupload the code for pid tuning.
    - We attempted to schedule the command however the robot only twitched each time we scheduled it.
    - Spent the latter part of the day reviewing LemLib documentation and our own previous code. We left off uncertain whether the issue was with the scheduler or with the library itself.

Reflection:
    Our plan to kick off PID tuning by running a command was sound, but LemLib’s behavior prevented even the tuning routine from holding the heading. The problem clearly stemmed from our framework and LemLib's library.


02/19/2026
Today's Goals:
    Understand the incompatibility between our framework and LemLib’s functions.

Today's Tasks:
    - Exhaustively tested various command placements: `initialize()`, `execute()`, and even embedding a manual `while(!done)` loop inside a command. Every variation behaved the same when scheduled. the command would be interrupted after a single cycle and the auto function would report “done.”
    - Ruled out scheduler bugs by writing simple dummy commands; they behaved correctly. The issue clearly lay with the library helper itself, which appears to maintain internal state that only updates when it is ticked repeatedly.

Reflection:
    With no immediate fix in sight, we decided to start the next meeting by creating a new PROS code base without our framework and look at how other teams use LemLib to isolation the issue.


02/21/2026
Today's Goals:
    Build a minimal PROS project containing only LemLib to observe the functions’s behavior without our framework.

Today's Tasks:
    - Initialized a fresh PROS workspace, added LemLib, and wrote a tiny control loop that called a angular PID function in a `while (true)` loop. The result matched the library examples, updated the heading continuously in the loop.
    - During this test we discovered that the documentation for tuning PIDs wanted the variables; `kl_small_error_range`, `kl_small_error_range_timeout`, `kl_large_error_range`, `kl_large_error_range_timeout` set to zero because it caused the cassis to declare success prematurely.

Reflection:
    We Concluded that our earlier assumption about the helper was incorrect. It isn’t a blocking, self‑contained routine but rather a stateful function that must be called on each loop iteration.
    We decided to draft a short‑term plan do to not having enough time to fully find a better solution before our competition: Execute full autonomous sequences manually from `auto()` (outside the scheduler) so the code can simply loop through LemLib calls.
