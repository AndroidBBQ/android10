Simple atomic FIFO for passing data between threads or processes.
This does not require mutexes.

One thread modifies the readCounter and the other thread modifies the writeCounter.

TODO The internal low-level implementation might be merged in some form with audio_utils fifo
and/or FMQ [after confirming that requirements are met].
The higher-levels parts related to AAudio use of the FIFO such as API, fds, relative
location of indices and data buffer, mapping, allocation of memmory will probably be kept as-is.
