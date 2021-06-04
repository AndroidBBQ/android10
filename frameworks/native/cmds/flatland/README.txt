Flatland is a benchmark for measuring GPU performance in various 2D UI
rendering and window compositing scenarios.  It is designed to be used early
in the device development process to evaluate GPU hardware (e.g. for SoC
selection).  It uses OpenGL ES 2.0, gralloc, and the Android explicit
synchronization framework, so it can only be run on devices with drivers
supporting those HALs.


Preparing a Device

Because it's measuring hardware performance, flatland should be run in as
consistent and static an environment as possible.  The display should be
turned off and background services should be stopped before running the
benchmark.  Running 'adb shell stop' after turning off the display is probably
sufficient for this, but if there are device- specific background services
that consume much CPU cycles, memory bandwidth, or might otherwise interfere
with GPU rendering, those should be stopped as well (and ideally they'd be
fixed or eliminated for production devices).

Additionally, all relevant hardware clocks should be locked at a particular
frequency when running flatland.  At a minimum this includes the CPU, GPU, and
memory bus clocks.  Running flatland with dynamic clocking essentially
measures the behavior of the dynamic clocking algorithm under a fairly
unrealistic workload, and will likely result in unstable and useless results.

If running the benchmark with the clocks locked causes thermal issues, the -s
command line option can be used to insert a sleep (specified in milliseconds)
in between each benchmark sample run.  Regardless of the scenario being
measured, each sample measurement runs for between 50 and 200 ms, so a sleep
time between 10 and 50 ms should address most thermal problems.


Interpreting the Output

The output of flatland should look something like this:

 cmdline: flatland
               Scenario               | Resolution  | Time (ms)
 16:10 Single Static Window           | 1280 x  800 |   fast
 16:10 Single Static Window           | 2560 x 1600 |  5.368
 16:10 Single Static Window           | 3840 x 2400 | 11.979
 16:10 App -> Home Transition         | 1280 x  800 |  4.069
 16:10 App -> Home Transition         | 2560 x 1600 | 15.911
 16:10 App -> Home Transition         | 3840 x 2400 | 38.795
 16:10 SurfaceView -> Home Transition | 1280 x  800 |  5.387
 16:10 SurfaceView -> Home Transition | 2560 x 1600 | 21.147
 16:10 SurfaceView -> Home Transition | 3840 x 2400 |   slow

The first column is simply a description of the scenario that's being
simulated.  The second column indicates the resolution at which the scenario
was measured.  The third column is the measured benchmark result.  It
indicates the expected time in milliseconds that a single frame of the
scenario takes to complete.

The third column may also contain one of three other values:

    fast - This indicates that frames of the scenario completed too fast to be
    reliably benchmarked.  This corresponds to a frame time less than 3 ms.
    Rather than spending time trying (and likely failing) to get a stable
    result, the scenario was skipped.

    slow - This indicates that frames of the scenario took too long to
    complete.  This corresponds to a frame time over 50 ms.  Rather than
    simulating a scenario that is obviously impractical on this device, the
    scenario was skipped.

    varies - This indicates that the scenario was measured, but it did not
    yield a stable result.  Occasionally this happens with an otherwise stable
    scenario.  In this case, simply rerunning flatland should yield a valid
    result.  If a scenario repeatedly results in a 'varies' output, that
    probably indicates that something is wrong with the environment in which
    flatland is being run.  Check that the hardware clock frequencies are
    locked and that no heavy-weight services / daemons are running in the
    background.
