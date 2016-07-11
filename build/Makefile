#
# Makefile for GameServer Framework
#
# Written by Teho.Kang
# 07.04.2016
#

-include config.mk

SILENT ?= @
TARGET ?= $(TARGET_NAME)

SRCS := $(shell find $(BUILD_SRC_DIR) -name "*.cc" ! -name ".*" 2>/dev/null)
INCS := $(addprefix -I, $(abspath $(shell find $(BUILD_SRC_DIR)/ -type d 2>/dev/null)))

CXXFLAGS := $(INCS) -std=c++11
LDFLAGS :=
DEFINES := -DSERVER_PORT=$(SERVER_PORT)

OBJS := $(SRCS:.cc=.o)
DEPS := $(SRCS:.cc=.d)

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

.cc.o:
	$(ECHO) "Compiling ..."
	$(CXX) $(CXXFLAGS) -c $< -o $(abspath $@)

%.d: %.cc
	$(ECHO) "Generating dependencies ... " $<
	$(CXX) -MM -MP -MT "$(@:.d=.o) $@" -MF $@ $<

$(TARGET).unstripped: $(OBJS)
	$(ECHO) "Linking ... $(TARGET)"
	$(CXX) $(realpath $(OBJS)) $(LDFLAGS) -o $(abspath $(TARGET).unstripped)

.PHONY: $(TARGET)
$(TARGET) : $(TARGET).unstripped
	mv -f $(TARGET).unstripped $(BUILD_OUT_DIR)/$(TARGET)
ifeq ($(STRIP),y)
	strip --strip-unneeded --strip-debug --discard-all $(BUILD_OUT_DIR)/$(TARGET)
endif

