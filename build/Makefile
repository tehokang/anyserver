#
# Makefile for GameServer Framework
#
# Written by Teho.Kang
# 07.04.2016
#

-include config.mk

SILENT ?= @

SRCS := $(shell find $(BUILD_SRC_DIR) -name "*.cpp" ! -name ".*" 2>/dev/null)
INCS := $(addprefix -I, $(abspath $(shell find $(BUILD_SRC_DIR) -type d -not -path "*/.git*" 2>/dev/null)))
LIBS := $(addprefix -L, $(abspath $(shell find $(BUILD_SRC_DIR) -type d -not -path "*/.git*" 2>/dev/null)))

CXXFLAGS := $(INCS) -std=c++11 -fexceptions
LDFLAGS := $(LIBS) -L$(BUILD_OUT_DIR) -lpthread -lwebsockets -lmicrohttpd
DEFINES :=

ifeq ($(CONFIG_DEBUG),YES)
CXXFLAGS += -DCONFIG_DEBUG -O -g
else
CXXFLAGS += -O2
endif

OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

all: prepare $(TARGET_LIB) example

.PHONY: prepare
prepare:
	@if [ ! -d $(BUILD_OUT_DIR) ]; then \
		mkdir -p $(BUILD_OUT_DIR); \
	fi;
	cp -dprf $(EXTERNAL_DIR)/libwebsockets/lib/$(CHIP_CORE)/* $(BUILD_OUT_DIR)
	cp -dprf $(EXTERNAL_DIR)/libmicrohttpd/lib/$(CHIP_CORE)/* $(BUILD_OUT_DIR)
	cp -f $(FRAMEWORK_DIR)/src/$(CONFIG_FILE) $(BUILD_OUT_DIR)/

.PHONY: distclean
distclean: clean
	rm -rf $(BUILD_OUT_DIR)

.PHONY: clean
clean:
	rm -f $(OBJS) $(DEPS)

.cpp.o:
	$(ECHO) "Compiling ..."
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $(abspath $@)

%.d: %.cpp
	$(ECHO) "CXXFLAGS : " $(CXXFLAGS)
	$(ECHO) Generating dependencies for $< ...
	$(CXX) $(CXXFLAGS) -MM -MP -MT "$(@:.d=.o) $@" -MF $@ $<

$(TARGET_LIB) : $(OBJS)
	$(ECHO) "Creating anyserver library ... $(TARGET_LIB).so"
	$(CXX) -shared -Wl,-soname,$(TARGET_LIB).so $(LDFLAGS) -o $(BUILD_OUT_DIR)/$(TARGET_LIB).so.$(VERSION) $(realpath $(OBJS))
	ln -srf $(BUILD_OUT_DIR)/$(TARGET_LIB).so.$(VERSION) $(BUILD_OUT_DIR)/$(TARGET_LIB).so
	
.PHONY: example
example : $(TARGET_LIB)
	$(CXX) $(EXAMPLE_DIR)/main.cpp $(CXXFLAGS) -o example -l$(TARGET) $(LDFLAGS)
	mv -f example $(BUILD_OUT_DIR)/example
	sudo chmod 755 $(BUILD_OUT_DIR)/example
ifeq ($(STRIP),y)
	strip --strip-unneeded --strip-debug --discard-all $(BUILD_OUT_DIR)/example
endif

-include $(DEPS)
