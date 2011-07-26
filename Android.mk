ifeq ($(strip $(BOARD_USES_TINY_AUDIO)),true)

  LOCAL_PATH := $(call my-dir)

  include $(CLEAR_VARS)

  LOCAL_C_INCLUDES += external/tinyalsa/include

  LOCAL_SRC_FILES := \
	TinyHardware.cpp

  LOCAL_MODULE := libaudio

  LOCAL_STATIC_LIBRARIES += libaudiointerface

  LOCAL_SHARED_LIBRARIES := \
    libtinyalsa \
    libcutils \
    libutils \
    libmedia \
    libc

  include $(BUILD_SHARED_LIBRARY)

  # Policy manager

  include $(CLEAR_VARS)

  LOCAL_SRC_FILES := TinyAudioPolicyManager.cpp

  LOCAL_MODULE := libaudiopolicy

  LOCAL_WHOLE_STATIC_LIBRARIES += libaudiopolicybase

  LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia

  include $(BUILD_SHARED_LIBRARY)

endif
