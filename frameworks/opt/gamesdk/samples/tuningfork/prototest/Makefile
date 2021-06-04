# Install protobuf and set PROTOBUF_ROOT to its location
ifndef PROTOBUF_ROOT
$(error PROTOBUF_ROOT is not set)
endif
PROTOC:=LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PROTOBUF_ROOT}/lib ${PROTOBUF_ROOT}/bin/protoc
CC:=g++

BUILD_DIR:=build
PROTO_BUILD_DIR:=${BUILD_DIR}/proto
PROTO_DIR:=proto
SRC_DIR:=src

GAME_OBJS:=${PROTO_BUILD_DIR}/tuningfork_extensions.pb.o ${BUILD_DIR}/game.o
ENG_GAME_OBJS:=${PROTO_BUILD_DIR}/eng_tuningfork_extensions.pb.o ${BUILD_DIR}/eng_game.o
TUNINGFORK_OBJS:= ${PROTO_BUILD_DIR}/tuningfork.pb.o ${BUILD_DIR}/mocktuningfork.o
GAMEENGINE_OBJS:= ${PROTO_BUILD_DIR}/eng_tuningfork.pb.o ${BUILD_DIR}/gameengine.o
SZTOOL_OBJS:=${PROTO_BUILD_DIR}/tuningfork.pb.o ${PROTO_BUILD_DIR}/tuningfork_clearcut_log.pb.o ${BUILD_DIR}/sztool.o ${BUILD_DIR}/dynamicproto.o
PLAY_OBJS:=${BUILD_DIR}/dynamicproto.o ${BUILD_DIR}/play.o

CDEFS:=-I${PROTOBUF_ROOT}/include -fPIC -I${BUILD_DIR} -std=c++11
LIBS:=-lprotobuf -L${PROTOBUF_ROOT}/lib
TUNINGFORK_LIB:=${BUILD_DIR}/mocktuningfork.so
GAMEENGINE_LIB:=${BUILD_DIR}/gameengine.so

all: run

run: play game eng_game sztool
	./play proto/tuningfork_extensions.proto > fidelityparams.pbin
	./game
	./eng_game
	./sztool proto/tuningfork_extensions.proto tuningfork_settings.txt

clean:
	rm -rf ${BUILD_DIR} play sztool game eng_game

game: ${TUNINGFORK_LIB} ${GAME_OBJS}
	${CC} -o $@ $^ ${LIBS}

eng_game: ${GAMEENGINE_LIB} ${TUNINGFORK_LIB} ${ENG_GAME_OBJS}
	${CC} -o $@ $^ ${LIBS}

${TUNINGFORK_LIB}: ${TUNINGFORK_OBJS}
	${CC} -o $@ $^ -shared

${GAMEENGINE_LIB}: ${GAMEENGINE_OBJS}
	${CC} -o $@ $^ -shared

play: ${PLAY_OBJS}
	${CC} -o $@ $^ ${LIBS}

sztool: ${SZTOOL_OBJS}
	${CC} -o $@ $^ ${LIBS}

${BUILD_DIR}/%.o: ${SRC_DIR}/%.cc | ${BUILD_DIR}
	${CC} -o $@ -c ${CDEFS} $<

${PROTO_BUILD_DIR}/%.pb.o: ${PROTO_BUILD_DIR}/%.pb.cc | ${PROTO_BUILD_DIR}
	${CC} -o $@ -c ${CDEFS} $<

${PROTO_BUILD_DIR}/%.pb.cc: ${PROTO_DIR}/%.proto | ${PROTO_BUILD_DIR}
	${PROTOC} --cpp_out=${BUILD_DIR}/proto -Iproto -I${PROTOBUF_ROOT}/include $<

${BUILD_DIR}:
	mkdir -p $@

${PROTO_BUILD_DIR}:
	mkdir -p $@

.PHONY: all run clean
