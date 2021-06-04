#ifndef ANDROID_DVR_DISPLAY_MANAGER_H_
#define ANDROID_DVR_DISPLAY_MANAGER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#include <dvr/dvr_display_types.h>
#include <dvr/dvr_surface.h>

__BEGIN_DECLS

typedef struct DvrBuffer DvrBuffer;
typedef struct DvrDisplayManager DvrDisplayManager;
typedef struct DvrSurfaceState DvrSurfaceState;
typedef struct DvrReadBufferQueue DvrReadBufferQueue;

typedef uint64_t DvrSurfaceUpdateFlags;

// Attempts to connect to the display manager service.
// @return 0 on success. Otherwise returns a negative error value.
int dvrDisplayManagerCreate(DvrDisplayManager** client_out);

// Destroys the display manager client object.
void dvrDisplayManagerDestroy(DvrDisplayManager* client);

// Returns an fd used to signal when surface updates occur. Note that depending
// on the underlying transport, only a subset of the real event bits may be
// supported. Use dvrDisplayManagerClientTranslateEpollEventMask to get the real
// event flags.
// @return the fd on success. Otherwise returns a negative error value.
int dvrDisplayManagerGetEventFd(DvrDisplayManager* client);

// @param in_events pass in the epoll revents that were initially returned by
// poll/epoll.
// @param on success, this value will be overwritten with the true poll/epoll
// values.
// @return 0 on success. Otherwise returns a negative error value.
int dvrDisplayManagerTranslateEpollEventMask(DvrDisplayManager* client,
                                             int in_events, int* out_events);

// Queries the display manager service for the current state of the display
// surfaces and stores the results in the given surface state object.
// @return 0 on success. Otherwise returns a negative error value.
int dvrDisplayManagerGetSurfaceState(DvrDisplayManager* client,
                                     DvrSurfaceState* surface_state);

// Gets a read buffer queue from the surface |surface_id| named |queue_id|. Each
// call returns a different read buffer queue connected to the same write buffer
// queue. Callers should cache these instead of requesting new ones when
// possible.
int dvrDisplayManagerGetReadBufferQueue(DvrDisplayManager* client,
                                        int surface_id, int queue_id,
                                        DvrReadBufferQueue** queue_out);

// Creates a new surface state object. This object may be used to receive the
// results of a surface state query. More than one state object may be created
// to keep multiple snapshots, if desired.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateCreate(DvrSurfaceState** surface_state);

// Destorys the surface state object.
void dvrSurfaceStateDestroy(DvrSurfaceState* surface_state);

// Writes the number of surfaces described in the state object into |count_out|.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetSurfaceCount(DvrSurfaceState* surface_state,
                                   size_t* count_out);

// Returns the update flags for the surface at |surface_index| in the state
// object. The flags may be used to determine what changes, if any, occurred to
// the surface since the last state update.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetUpdateFlags(DvrSurfaceState* surface_state,
                                  size_t surface_index,
                                  DvrSurfaceUpdateFlags* flags_out);

// Returns the unique identifier of surface at |surface_index| in the state
// object. The identifier may be used to distinguish between surfaces.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetSurfaceId(DvrSurfaceState* surface_state,
                                size_t surface_index, int* surface_id_out);

// Returns the process id of surface at |surface_index| in the state object.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetProcessId(DvrSurfaceState* surface_state,
                                size_t surface_index, int* process_id_out);

// Writes the number of queues in the surface at |surface_index| in the state
// object into |count_out|.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetQueueCount(DvrSurfaceState* surface_state,
                                 size_t surface_index, size_t* count_out);

// Returns up to |max_count| queue ids for the queues belonging to the surface
// at |surface_index| in the state object.
// @return The number of queue ids written on success. Otherwise returns a
// negative error value.
ssize_t dvrSurfaceStateGetQueueIds(DvrSurfaceState* surface_state,
                                   size_t surface_index, int* queue_ids,
                                   size_t max_count);

// Writes the z-order of the surface at |surface_index| in surface state object
// into |z_order_out|.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceStateGetZOrder(DvrSurfaceState* surface_state,
                             size_t surface_index, int* z_order_out);

// Writes the visible state of the surface at |surface_index| in the surface
// state object into |visible_out|.
// @return 0 on success. Otherwise it returns a negative error value.
int dvrSurfaceStateGetVisible(DvrSurfaceState* surface_state,
                              size_t surface_index, bool* visible_out);

// Writes the number of attributes on the surface at |surface_index| in the
// state object into |count_out|.
// @return 0 on success. Otherwise it returns a negative error value.
int dvrSurfaceStateGetAttributeCount(DvrSurfaceState* surface_state,
                                     size_t surface_index, size_t* count_out);

// Writes the list of attribute key/value pairs for the surface at
// |surface_index| in the surface state object into |attributes|.
// @return The number of attributes written on success. Otherwise returns a
// negative error value.
ssize_t dvrSurfaceStateGetAttributes(DvrSurfaceState* surface_state,
                                     size_t surface_index,
                                     DvrSurfaceAttribute* attributes,
                                     size_t max_count);

__END_DECLS

#endif  // ANDROID_DVR_DISPLAY_MANAGER_H_
