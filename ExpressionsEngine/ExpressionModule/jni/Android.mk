LOCAL_PATH := (call my-dir)

include	$(CLEAN_VARS)

LOCAL_MODULE := ExpressionModule
LOCAL_SRC_FILES := ExpressionModule.cpp,ExpressionModule.h

include $(BUILD_SHARED_LIBRARY)