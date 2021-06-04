libnbaio (for "Non-Blocking Audio I/O") was originally intended to
be a purely non-blocking API.  It has evolved to now include
a few blocking implementations of the interface.

Note: as used here, "short transfer count" means the return value for
read() or write() that indicates the actual number of successfully
transferred frames is less than the requested number of frames.

Pipe
----
supports 1 writer and N readers

no mutexes, so safe to use between SCHED_NORMAL and SCHED_FIFO threads

writes:
  non-blocking
  never return a short transfer count
  overwrite data if not consumed quickly enough

reads:
  non-blocking
  return a short transfer count if not enough data
  will lose data if reader doesn't keep up

MonoPipe
--------
supports 1 writer and 1 reader

no mutexes, so safe to use between SCHED_NORMAL and SCHED_FIFO threads

write are optionally blocking:
  if configured to block, then will wait until space available before returning
  if configured to not block, then will return a short transfer count
    and will never overwrite data

reads:
  non-blocking
  return a short transfer count if not enough data
  never lose data

