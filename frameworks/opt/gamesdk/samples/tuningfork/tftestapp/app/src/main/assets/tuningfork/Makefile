
FPS:=1 2 3 4 5 6
FP_OUT_FILES:=$(patsubst %, dev_tuningfork_fidelityparams_%.bin, ${FPS})
FP_IN_FILES:=$(patsubst %.bin, %.txt, ${FP_OUT_FILES})
PROTOC:=protoc
INC_DIR:=../../proto

all: ${FP_OUT_FILES} tuningfork_settings.bin

clean:
	rm -f *.bin

tuningfork_settings.bin: tuningfork_settings.txt
	${PROTOC} --encode=com.google.tuningfork.Settings -I${INC_DIR} ${INC_DIR}/tuningfork.proto < $< > $@

%.bin: %.txt
	${PROTOC} --encode=com.google.tuningfork.FidelityParams -I${INC_DIR} ${INC_DIR}/dev_tuningfork.proto < $< > $@
