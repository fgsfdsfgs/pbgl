PBGL_VERSION := 0.1
PBGL_GL_VERSION := 1.2

PBGL_DIR = $(NXDK_DIR)/lib/pbgl

PBGL_SRCS := $(wildcard $(PBGL_DIR)/src/*.c)

NXDK_CFLAGS += -I$(PBGL_DIR)/include -DXBOX
NXDK_CXXFLAGS += -I$(PBGL_DIR)/include -DXBOX

PBGL_CFLAGS := -DPBGL_VERSION=$(PBGL_VERSION) -DPBGL_GL_VERSION=$(PBGL_GL_VERSION)

ifeq ($(DEBUG),y)
	PBGL_CFLAGS += -DPBGL_DEBUG
endif

PBGL_OBJS = $(addsuffix .obj, $(basename $(PBGL_SRCS)))

LIB_DEPS += $(addsuffix .d, $(PBGL_SRCS))

$(NXDK_DIR)/lib/libpbgl.lib: CFLAGS += $(PBGL_CFLAGS)
$(NXDK_DIR)/lib/libpbgl.lib: CXXFLAGS += $(PBGL_CFLAGS)
$(NXDK_DIR)/lib/libpbgl.lib: $(PBGL_OBJS)

main.exe: $(NXDK_DIR)/lib/libpbgl.lib

CLEANRULES += clean-pbgl

.PHONY: clean-pbgl
clean-pbgl:
	$(VE)rm -f $(PBGL_OBJS) $(NXDK_DIR)/lib/libpbgl.lib
