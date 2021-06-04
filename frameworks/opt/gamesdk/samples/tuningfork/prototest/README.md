This sample contains examples of the flow of protobuf messages within Tuning Fork.
In order to build and run the tests, first make sure you have protobuf installed, e.g. in /usr/local
Then invoke make like so:
make PROTOBUF_ROOT=/usr/local

It should build and run 4 executables:
./play : a simulation of the play console, where you enter fidelity parameters for the game
./game : simulation of a native game
./gameeng : 2 games that use a game engine, one native and one non-native
./sztool : a tool for estimating the size of data uploaded to clearcut
