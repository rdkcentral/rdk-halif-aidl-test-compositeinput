#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

.PHONY: all clean

CXX ?= g++

AIDL_LIB_NAME := aidl_client
AIDL_LIB      := lib$(AIDL_LIB_NAME).so

BUILD_DIR    ?= $(CURDIR)/../build
AIDL_OBJ_DIR := $(BUILD_DIR)/obj/aidl_lib

# Versioned src/include dirs resolved from rdk-halif-aidl by build.sh.
# Guard errors so 'make clean' works without these being set.
ifeq ($(filter clean,$(MAKECMDGOALS)),)
HALIF_AIDL_SRC_DIR     ?= $(error HALIF_AIDL_SRC_DIR is not set - run via build.sh)
HALIF_AIDL_INC_DIR     ?= $(error HALIF_AIDL_INC_DIR is not set - run via build.sh)
BINDER_SDK_INCLUDE_DIR ?= $(error BINDER_SDK_INCLUDE_DIR is not set - run via build.sh)
BINDER_SDK_LIB_DIR     ?= $(error BINDER_SDK_LIB_DIR is not set - run via build.sh)

# Source files from the versioned AIDL src directory
AIDL_SRC := $(shell find $(HALIF_AIDL_SRC_DIR) -type f -name '*.cpp')
AIDL_OBJ := $(patsubst $(HALIF_AIDL_SRC_DIR)/%.cpp,$(AIDL_OBJ_DIR)/%.o,$(AIDL_SRC))
endif

AIDL_LIB_PATH := $(BUILD_DIR)/usr/lib/$(AIDL_LIB)

# Include flags: versioned compositeinput headers, staged dep headers (common etc.
# staged to build/usr/include by install_halif_headers in build.sh), and the
# binder SDK.
AIDL_LIB_INC_FLAGS := -I$(HALIF_AIDL_INC_DIR) \
                      -I$(BUILD_DIR)/usr/include \
                      -I$(BINDER_SDK_INCLUDE_DIR)

# Link against libcommon-cpp.so (staged to build/usr/lib by install_halif_libs
# in build.sh) and the binder runtime libs.
AIDL_LIB_LD_LIBS := -lcommon-cpp -lbinder -lutils

AIDL_LIB_LDFLAGS := -L$(BUILD_DIR)/usr/lib \
                    -Wl,-rpath-link=$(BUILD_DIR)/usr/lib \
                    -L$(BINDER_SDK_LIB_DIR) \
                    -Wl,-rpath-link=$(BINDER_SDK_LIB_DIR)

AIDL_LIB_CFLAGS := -Wall -Wextra -std=c++17 -ggdb -fPIC

# Default target
all: $(AIDL_LIB_PATH)

# Build shared library
$(AIDL_LIB_PATH): $(AIDL_OBJ)
	mkdir -p $(dir $@)
	$(CXX) -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

# Compile objects
$(AIDL_OBJ_DIR)/%.o: $(HALIF_AIDL_SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(AIDL_LIB_CFLAGS) $(AIDL_LIB_INC_FLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(BUILD_DIR)/usr/lib/libaidl_client.so
	rm -rf $(AIDL_OBJ_DIR)
