#=====================================================================
# Include Subdirectories
#=====================================================================

# Not building RSoV modules in PDK builds, as libSPIRV is not available in PDK.
ifneq ($(TARGET_BUILD_PDK), true)

LOCAL_PATH:=$(call my-dir)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif # TARGET_BUILD_PDK
