SurfaceReplayer Documentation
===================

[go/SurfaceReplayer](go/SurfaceReplayer)

SurfaceReplayer is a playback mechanism that allows the replaying of traces recorded by
[SurfaceInterceptor](go/SurfaceInterceptor) from SurfaceFlinger. It specifically replays

* Creation and deletion of surfaces/displays
* Alterations to the surfaces/displays called Transactions
* Buffer Updates to surfaces
* VSync events

At their specified times to be as close to the original trace.

Usage
--------

###Creating a trace

SurfaceInterceptor is the mechanism used to create traces. The device needs to be rooted in order to
utilize it. To allow it to write to the device, run

`setenforce 0`

To start recording a trace, run

`service call SurfaceFlinger 1020 i32 1`

To stop recording, run

`service call SurfaceFlinger 1020 i32 0`

The default location for the trace is `/data/SurfaceTrace.dat`

###Executable

To replay a specific trace, execute

`/data/local/tmp/surfacereplayer /absolute/path/to/trace`

inside the android shell. This will replay the full trace and then exit. Running this command
outside of the shell by prepending `adb shell` will not allow for manual control and will not turn
off VSync injections if it interrupted in any way other than fully replaying the trace

The replay will not fill surfaces with their contents during the capture. Rather they are given a
random color which will be the same every time the trace is replayed. Surfaces modulate their color
at buffer updates.

**Options:**

- -m    pause the replayer at the start of the trace for manual replay
- -t [Number of Threads] uses specified number of threads to queue up actions (default is 3)
- -s [Timestamp] switches to manual replay at specified timestamp
- -n    Ignore timestamps and run through trace as fast as possible
- -l    Indefinitely loop the replayer
- -h    displays help menu

**Manual Replay:**
When replaying, if the user presses CTRL-C, the replay will stop and can be manually controlled
by the user. Pressing CTRL-C again will exit the replayer.

Manual replaying is similar to debugging in gdb. A prompt is presented and the user is able to
input commands to choose how to proceed by hitting enter after inputting a command. Pressing enter
without inputting a command repeats the previous command.

- n  - steps the replayer to the next VSync event
- ni - steps the replayer to the next increment
- c  - continues normal replaying
- c [milliseconds] - continue until specified number of milliseconds have passed
- s [timestamp]    - continue and stop at specified timestamp
- l  - list out timestamp of current increment
- h  - displays help menu

###Shared Library

To use the shared library include these shared libraries

`libsurfacereplayer`
`libprotobuf-cpp-full`
`libutils`

And the static library

`libtrace_proto`

Include the replayer header at the top of your file

`#include <replayer/Replayer.h>`

There are two constructors for the replayer

`Replayer(std::string& filename, bool replayManually, int numThreads, bool wait, nsecs_t stopHere)`
`Replayer(Trace& trace, ... ditto ...)`

The first constructor takes in the filepath where the trace is located and loads in the trace
object internally.
- replayManually - **True**: if the replayer will immediately switch to manual replay at the start
- numThreads - Number of worker threads the replayer will use.
- wait - **False**: Replayer ignores waits in between increments
- stopHere - Time stamp of where the replayer should run to then switch to manual replay

The second constructor includes all of the same parameters but takes in a preloaded trace object.
To use add

`#include <frameworks/native/cmds/surfacereplayer/proto/src/trace.pb.h>`

To your file

After initializing the Replayer call

    replayer.replay();

And the trace will start replaying. Once the trace is finished replaying, the function will return.
The layers that are visible at the end of the trace will remain on screen until the program
terminates.


**If VSyncs are broken after running the replayer** that means `enableVSyncInjections(false)` was
never executed. This can be fixed by executing

`service call SurfaceFlinger 23 i32 0`

in the android shell

Code Breakdown
-------------

The Replayer is composed of 5 components.

- The data format of the trace (Trace.proto)
- The Replayer object (Replayer.cpp)
- The synchronization mechanism to signal threads within the Replayer (Event.cpp)
- The scheduler for buffer updates per surface (BufferQueueScheduler.cpp)
- The Main executable (Main.cpp)

### Traces

Traces are represented as a protobuf message located in surfacereplayer/proto/src.

**Traces** contain *repeated* **Increments** (events that have occurred in SurfaceFlinger).
**Increments** contain the time stamp of when it occurred and a *oneof* which can be a

 - Transaction
 - SurfaceCreation
 - SurfaceDeletion
 - DisplayCreation
 - DisplayDeleteion
 - BufferUpdate
 - VSyncEvent
 - PowerModeUpdate

**Transactions** contain whether the transaction was synchronous or animated and *repeated*
**SurfaceChanges** and **DisplayChanges**

- **SurfaceChanges** contain an id of the surface being manipulated and can be changes such as
position, alpha, hidden, size, etc.
- **DisplayChanges** contain the id of the display being manipulated and can be changes such as
size, layer stack, projection, etc.

**Surface/Display Creation** contain the id of the surface/display and the name of the
surface/display

**Surface/Display Deletion** contain the id of the surface/display to be deleted

**Buffer Updates** contain the id of the surface who's buffer is being updated, the size of the
buffer, and the frame number.

**VSyncEvents** contain when the VSync event has occurred.

**PowerModeUpdates** contain the id of the display being updated and what mode it is being
changed to.

To output the contents of a trace in a readable format, execute

`**aprotoc** --decode=Trace \
-I=$ANDROID_BUILD_TOP/frameworks/native/cmds/surfacereplayer/proto/src \
$ANDROID_BUILD_TOP/frameworks/native/cmds/surfacereplayer/proto/src/trace.proto \
 < **YourTraceFile.dat** > **YourOutputName.txt**`


###Replayer

Fundamentally the replayer loads a trace and iterates through each increment, waiting the required
amount of time until the increment should be executed, then executing the increment. The first
increment in a trace does not start at 0, rather the replayer treats its time stamp as time 0 and
goes from there.

Increments from the trace are played asynchronously rather than one by one, being dispatched by
the main thread, queued up in a thread pool and completed when the main thread deems they are
ready to finish execution.

When an increment is dispatched, it completes as much work as it can before it has to be
synchronized (e.g. prebaking a buffer for a BufferUpdate). When it gets to a critical action
(e.g. locking and pushing a buffer), it waits for the main thread to complete it using an Event
object. The main thread holds a queue of these Event objects and completes the
corresponding Event base on its time stamp. After completing an increment, the main thread will
dispatch another increment and continue.

The main thread's execution flow is outlined below

    initReplay() //queue up the initial increments
    while(!pendingIncrements.empty()) { //while increments remaining
        event = pendingIncrement.pop();
        wait(event.time_stamp(); //waitUntil it is time to complete this increment

        event.complete() //signal to let event finish
        if(increments remaing()) {
            dispatchEvent() //queue up another increment
        }
    }

A worker thread's flow looks like so

    //dispatched!
    Execute non-time sensitive work here
    ...
    event.readyToExecute() //time sensitive point...waiting for Main Thread
    ...
    Finish execution


### Event

An Event is a simple synchronization mechanism used to facilitate communication between the main
and worker threads. Every time an increment is dispatched, an Event object is also created.

An Event can be in 4 different states:

- **SettingUp** - The worker is in the process of completing all non-time sensitive work
- **Waiting** - The worker is waiting on the main thread to signal it.
- **Signaled** - The worker has just been signaled by the main thread
- **Running** - The worker is running again and finishing the rest of its work.

When the main thread wants to finish the execution of a worker, the worker can either still be
**SettingUp**, in which the main thread will wait, or the worker will be **Waiting**, in which the
main thread will **Signal** it to complete. The worker thread changes itself to the **Running**
state once **Signaled**. This last step exists in order to communicate back to the main thread that
the worker thread has actually started completing its execution, rather than being preempted right
after signalling. Once this happens, the main thread schedules the next worker. This makes sure
there is a constant amount of workers running at one time.

This activity is encapsulated in the `readyToExecute()` and `complete()` functions called by the
worker and main thread respectively.

### BufferQueueScheduler

During a **BuferUpdate**, the worker thread will wait until **Signaled** to unlock and post a
buffer that has been prefilled during the **SettingUp** phase. However if there are two sequential
**BufferUpdates** that act on the same surface, both threads will try to lock a buffer and fill it,
which isn't possible and will cause a deadlock. The BufferQueueScheduler solves this problem by
handling when **BufferUpdates** should be scheduled, making sure that they don't overlap.

When a surface is created, a BufferQueueScheduler is also created along side it. Whenever a
**BufferUpdate** is read, it schedules the event onto its own internal queue and then schedules one
every time an Event is completed.

### Main

The main exectuable reads in the command line arguments. Creates the Replayer using those
arguments. Executes `replay()` on the Replayer. If there are no errors while replaying it will exit
gracefully, if there are then it will report the error and then exit.
