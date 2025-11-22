PBGL_VERSION := 0.1
PBGL_GL_VERSION := 1.2

PBGL_PATH := $(realpath $(lastword $(MAKEFILE_LIST)))
PBGL_DIR := $(dir $(PBGL_PATH))

PBGL_LIB ?= $(PBGL_DIR)/libpbgl.lib

PBGL_SRCS := $(wildcard $(PBGL_DIR)/src/*.c)

PBGL_CFLAGS := -I$(PBGL_DIR)/include

LOCAL_CFLAGS := $(PBGL_CFLAGS)
LOCAL_CFLAGS += -DXBOX -DPBGL_VERSION=$(PBGL_VERSION) -DPBGL_GL_VERSION=$(PBGL_GL_VERSION)

ifeq ($(DEBUG),y)
	PBGL_CFLAGS += -DPBGL_DEBUG
endif

PBGL_OBJS = $(addsuffix .obj, $(basename $(PBGL_SRCS)))
PBGL_DEPS = $(addsuffix .d, $(PBGL_SRCS))

$(PBGL_LIB): CFLAGS += $(LOCAL_CFLAGS)
$(PBGL_LIB): CXXFLAGS += $(LOCAL_CFLAGS)
$(PBGL_LIB): $(PBGL_OBJS)

CLEANRULES += clean-pbgl

.PHONY: clean-pbgl
clean-pbgl:
	$(VE)rm -f $(PBGL_OBJS) $(PBGL_LIB) $(PBGL_DEPS)

clean: clean-pbgl
