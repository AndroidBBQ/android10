==== Summary ====

android.frameworks.sensorservice@1.0 is a package that mimics the sensors API in
NDK (sensor.h). It includes a subset of these APIs and introduces a few
adaptations.

=== Design Details ===

- ISensorManager
ISensorMangaer includes member functions that adapts the ASensorManager_*
series in NDK. An instance of ISensorManager must be able to
    - retrieve sensors
    - create direct report channel
    - create event queue

- IDirectReportChannel
IDirectReportChannel corresponds to a channel ID, an integer obtained in
ASensorManager_createSharedMemoryDirectChannel and
ASensorManager_createHardwareBufferDirectChannel. An instance of
IDirectReportChannel must also destroy it against the sensor manager. An
IDirectReportChannel must be able to configure itself (an adaptation to
ASensorManager_configureDirectReport). The implementation must also call
ASensorManager_destroyEventQueue on destruction of IDirectReportChannel.

Usage typically looks like this (transaction errors are not handled):

    sp<ISensorManager> manager = ISensorManager::getService();
    int32_t sensorHandle;
    manager->getDefaultSensor(SensorType::GYROSCOPE,
            [&sensorHandle] (const auto &info) {
                sensorHandle = info.sensorHandle;
            });
    hidl_memory mem;
    const uint64_t size = 4096;
    ::android::hidl::memory::V1_0::IAllocator::getService()->allocate(size,
            [&](auto, const auto &m) { mem = m; });
    if (!mem.handle()) {
        /* error handling */
    }
    sp<IDirectChannel> chan;
    Result res;
    manager->createAshmemDirectChannel(mem, size,
            [&chan, &res] (const auto &c, auto r) {
                chan = c; res = r;
            });
    if (res != Result::OK) { /* error handling */ }
    chan->configure(sensorHandle, RateLevel::FAST, [&](auto token, auto result) {
        if (result != Result::OK) {
            /* error handling */
        }
    });

    /* obtain sensor events from shared memory */

    chan->configure(sensorHandle, RateLevel::STOP, [&](auto token, auto result) {
        if (result != Result::OK) {
            /* error handling */
        }
    });

    /*
     * Free the channel.
     * kernel calls decStrong() on server side implementation of IDirectChannel,
     * hence resources are freed as well.
     */
     chan = nullptr;

- IEventQueue, IEventQueueCallback
IEventQueue includes member functions that adapts some of the
ASensorEventQueue_* seeries in NDK. An instance of IEventQueue must be able to
    - enable  selected sensors (adapts ASensorEventQueue_registerSensor)
    - disable selected sensors (adapts ASensorEventQueue_disableSensor)

The implementation must free all resources related to this IEventQueue instance
and call ASensorManager_destroyEventQueue on destruction of IEventQueue.

Unlike NDK ASensorEventQueue_hasEvents and ASensorEventQueue_getEvents, which
implies a poll model for sensor events, IEventQueue uses a push model by using
callbacks. When creating an event queue, client must provide an instance of
IEventQueueCallback. The implementation of IEventQueue must either use a global
looper or create a new looper to call on ASensorManager_createEventQueue. The
server implementation must use this looper to constantly poll for events, then
invoke the callback when any event is fired.

IEventQueueCallback.onEvent is designed to be oneway, because the server should
not wait for the client to finish handling the event. The callback
should finish in a predictably short time, and should not block or run for an
extended period of time. The callbacks can be invoked in a very high frequency;
a long running callback means delay in handling of subsequent events and filling
up the (kernel binder buffer) memory space of the client process, eventually the
server sees a transaction error when issuing the callback. It is up to the
client to be configured single-threaded or multi-threaded to handle these
callbacks.
    - Single-threaded clients receive events in the correct order in the same
      thread.
    - Multi-threaded clients receive events in the correct order but in
      different threads; it is the clients' responsibility to deal with
      concurrency issues and handle events in the expected order to avoid race
      conditions.

Usage typically looks like this (transaction errors are not handled):

    struct Callback : IEventQueueCallback {
        Return<void> onEvent(const Event &e) {
            /* handle sensor event e */
        }
    };
    sp<ISensorManager> manager = ISensorManager::getService();
    int32_t sensorHandle;
    manager->getDefaultSensor(SensorType::GYROSCOPE,
            [&sensorHandle] (const auto &info) {
                sensorHandle = info.sensorHandle;
            });
    sp<IEventQueue> queue;
    Result res;
    manager->createEventQueue(new Callback(),
            [&queue, &res] (const auto &q, auto r) {
                queue = q; res = r;
            });
    /* Server side implementation of IEventQueue holds a strong reference to
     * the callback. */
    if (res != Result::OK) { /* error handling */ }

    if (q->enableSensor(sensorHandle,
            20000 /* sample period */, 0 /* latency */) != Result::OK) {
        /* error handling */
    }

    /* start receiving events via onEvent */

    if (q->disableSensor(sensorHandle) != Result::OK) {
        /* error handling */
    }

    /*
     * Free the event queue.
     * kernel calls decStrong() on server side implementation of IEventQueue,
     * hence resources (including the callback) are freed as well.
     */
    queue = nullptr;
