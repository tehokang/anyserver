#
# Makefile for GameServer Framework
#
# Written by Teho.Kang
# 07.04.2016
#

-include config.mk

SILENT ?= @
TARGET ?= $(TARGET_NAME)

SRCS := $(shell find $(BUILD_SRC_DIR) -name "*.cpp" ! -name ".*" 2>/dev/null)
INCS := $(addprefix -I, $(abspath $(shell find $(BUILD_SRC_DIR)/ -type d 2>/dev/null)))

CXXFLAGS := $(INCS) -std=c++11
LDFLAGS := -lpthread
DEFINES := -DSERVER_PORT=$(SERVER_PORT)

ifeq ($(CONFIG_DEBUG),YES)
CXXFLAGS += -DCONFIG_DEBUG
endif

OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

all: prepare $(TARGET)

.PHONY: prepare
prepare:
	@if [ ! -d $(BUILD_OUT_DIR) ]; then \
		mkdir -p $(BUILD_OUT_DIR); \
	fi;

.PHONY: distclean
distclean: clean
	rm -rf $(BUILD_OUT_DIR)

.PHONY: clean
clean:
	rm -f $(OBJS) $(DEPS)

.cpp.o:
	$(ECHO) "Compiling ..."
	$(CXX) $(CXXFLAGS) -c $< -o $(abspath $@)

%.d: %.cpp
	$(ECHO) Generating dependencies for $< ...
	$(CXX) $(CXXFLAGS) -MM -MP -MT "$(@:.d=.o) $@" -MF $@ $<

$(TARGET).unstripped: $(OBJS)
	$(ECHO) "Linking ... $(TARGET)"
	$(CXX) $(realpath $(OBJS)) $(LDFLAGS) -o $(abspath $(TARGET).unstripped)

.PHONY: $(TARGET)
$(TARGET) : $(TARGET).unstripped
	mv -f $(TARGET).unstripped $(BUILD_OUT_DIR)/$(TARGET)
ifeq ($(STRIP),y)
	strip --strip-unneeded --strip-debug --discard-all $(BUILD_OUT_DIR)/$(TARGET)
endif

-include $(DEPS)
