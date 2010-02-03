LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := milkytracker

CG_SUBDIRS := 	src/compression \
				src/compression/lha \
				src/compression/zziplib \
				src/compression/zziplib/generic \
				src/fx \
				src/milkyplay \
				src/milkyplay/drivers/sdl \
				src/ppui \
				src/ppui/osinterface \
				src/ppui/osinterface/sdl \
				src/ppui/sdl \
				src/tracker \
				src/tracker/sdl

# Add more subdirs here, like src/subdir1 src/subdir2

LOCAL_CFLAGS := $(foreach D, $(CG_SUBDIRS), -I$(LOCAL_PATH)/$(D)) \
				-I$(LOCAL_PATH)/src/ppui/osinterface/posix \
				-I$(LOCAL_PATH) \
				-I$(LOCAL_PATH)/../sdl/include \
				-DHAVE_CONFIG_H -DMILKYTRACKER -D__THREADTIMER__ -DDRIVER_UNIX -D_GNU_SOURCE=1 -D_REENTRANT -D__LOWRES__ -D__FORCE_SDL_AUDIO__

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += src/ppui/osinterface/posix/PPSystem_POSIX.cpp src/ppui/osinterface/posix/PPPath_POSIX.cpp

# Uncomment to also add C sources
# LOCAL_SRC_FILES += $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_STATIC_LIBRARIES := sdl

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

include $(BUILD_SHARED_LIBRARY)

