#
# Makefile for GameServer Framework
#
# Written by Teho.Kang
# 07.04.2016
#

# Global Configuration 
# e.g. project configuration
-include globalconfig

CROSS :=
CHIP_CORE := 

ifeq ($(CROSS),)
SYSTEM := x86
CXX := g++
else
ifeq ($(CHIP_CORE),mipsel)
SYSTEM := mipsel
elif ($(CHIP_CORE),arm)
SYSTEM := arm
endif
CXX := $(CROSS)g++
endif

# Build Configuration
ECHO := @echo
BUILD_SRC_DIR := $(PWD)/../
BUILD_OUT_DIR := $(PWD)/out/
FRAMEWORK_DIR := $(BUILD_SRC_DIR)/framework
3RD_PARTY_DIR := $(FRAMEWORK_DIR)/src/3rdparty/

TARGET_ARCH := x86
TARGET_NAME := anyserver

CONFIG_DEBUG := YES

# User Configuration
SERVER_PORT := 9998

