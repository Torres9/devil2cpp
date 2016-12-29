LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := unity
LOCAL_SRC_FILES := devil2cpp.c

LOCAL_CFLAGS += -std=c99

include $(BUILD_SHARED_LIBRARY)
