DIR = ../../src/autogen
ALL = $(DIR)/MPH_to_3DGroup.h $(DIR)/MPH_to_AudioPlayer.h $(DIR)/MPH_to_AudioRecorder.h \
    $(DIR)/MPH_to_Engine.h $(DIR)/MPH_to_LEDDevice.h $(DIR)/MPH_to_Listener.h \
    $(DIR)/MPH_to_MetadataExtractor.h $(DIR)/MPH_to_MidiPlayer.h $(DIR)/MPH_to_OutputMix.h \
    $(DIR)/MPH_to_Vibra.h

$(ALL) : mphtogen
	./mphtogen

mphtogen : mphtogen.c MPH_to.c MPH.h MPH_to.h
# Add -DANDROID if both (a) building for Android, and (b) not
# using -DUSE_DESIGNATED_INITIALIZERS in ../../src/Android.mk
	gcc -o $@ -DUSE_DESIGNATED_INITIALIZERS -Wall -Werror mphtogen.c MPH_to.c

clean :
	$(RM) mphtogen
