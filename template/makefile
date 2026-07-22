.DEFAULT_GOAL := all

MODE ?= debug

PROJECT_NAME := {{project-name}}

# Directory configuration
SRC_DIR   := src
CORE_SUBMODULE_PATH := third_party/cpp-core
CORE_SRC_DIR := $(CORE_SUBMODULE_PATH)/src
BUILD_DIR := build/$(MODE)
MOD_DIR   := $(BUILD_DIR)/modules
OBJ_DIR   := $(BUILD_DIR)/obj
COMPDB_DIR := $(BUILD_DIR)/compdb

# Cross-platform utilities
RM := rm -rf
MKDIR := mkdir -p

# Binary output
TARGET := build/$(PROJECT_NAME)_$(MODE)
COMPDB_FILE := compile_commands.json

# Toolchain flags
CXX := clang++

BASE_FLAGS := -std=c++26 -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -I./$(CORE_SRC_DIR)

ifeq ($(MODE), release)
	CXXFLAGS := $(BASE_FLAGS) -O3 -march=native -DNDEBUG
else ifeq ($(MODE), lto)
	CXXFLAGS := $(BASE_FLAGS) -O3 -march=native -DNDEBUG -flto=full
else ifeq ($(MODE), instrument)
	CXXFLAGS := $(BASE_FLAGS) -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer -DBUILD_INSTRUMENT
else
	MODE := debug
	CXXFLAGS := $(BASE_FLAGS) -O0 -g -DBUILD_DEBUG
endif

MOD_FLAGS := -fprebuilt-module-path=$(MOD_DIR)
MJFLAGS   = $(if $(GENERATE_COMPDB),-MJ $(COMPDB_DIR)/$(subst /,_,$(patsubst $(OBJ_DIR)/%,%,$@)).json)

# Recursive source discovery
APP_CPPM_SRCS := $(shell find $(SRC_DIR) -type f -name '*.cppm' | sort)
CORE_CPPM_SRCS := $(shell [ -d $(CORE_SRC_DIR) ] && find $(CORE_SRC_DIR) -type f -name '*.cppm' | sort || true)
CPP_SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp' | sort)

MODULE_SRCS := $(APP_CPPM_SRCS) $(CORE_CPPM_SRCS)

module_obj_from_src = $(OBJ_DIR)/$(patsubst %.cppm,%.o,$(1))
module_pcm_from_src = $(MOD_DIR)/$(patsubst %.cppm,%.pcm,$(1))

MODULE_OBJS := $(foreach src,$(MODULE_SRCS),$(call module_obj_from_src,$(src)))
CPP_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
ALL_OBJS := $(MODULE_OBJS) $(CPP_OBJS)

LINK_MOD_FLAGS := $(foreach src,$(MODULE_SRCS),-fmodule-file=$(basename $(notdir $(src)))=$(call module_pcm_from_src,$(src)))

# Import-based dependency extraction for module interface units.
module_obj_from_name = $(call module_obj_from_src,$(firstword $(filter %/$(1).cppm,$(MODULE_SRCS))))
imports_of_src = $(shell sed -n 's/^[[:space:]]*\(export[[:space:]]\+\)\?import[[:space:]]\([A-Za-z0-9_.]*\)[[:space:]]*;.*/\2/p' $(1))
module_import_obj_deps = $(strip $(foreach m,$(call imports_of_src,$(1)),$(call module_obj_from_name,$(m))))

$(foreach src,$(MODULE_SRCS),$(eval $(call module_obj_from_src,$(src)): $(call module_import_obj_deps,$(src))))

.PHONY: all debug release lto instrument do-build clean clean_all bear init-submodule verify-submodule-pin

all: init-submodule
	@$(MAKE) MODE=$(MODE) do-build

do-build: $(TARGET)

debug:
	@$(MAKE) MODE=debug all

release:
	@$(MAKE) MODE=release all

lto:
	@$(MAKE) MODE=lto all

instrument:
	@$(MAKE) MODE=instrument all

init-submodule:
	@sh ci/init_cpp_core_submodule.sh

verify-submodule-pin:
	@sh ci/verify_cpp_core_submodule.sh

bear: init-submodule
	@$(RM) $(COMPDB_FILE)
	@$(RM) $(BUILD_DIR)
	@$(MKDIR) $(COMPDB_DIR)
	@$(MAKE) MODE=$(MODE) GENERATE_COMPDB=1 do-build
	@{ printf '[\n'; find $(COMPDB_DIR) -type f -name '*.json' | sort | xargs -r cat; } | sed '$$s/,$$//' > $(COMPDB_FILE)
	@printf '\n]\n' >> $(COMPDB_FILE)

$(TARGET): $(ALL_OBJS)
	@$(MKDIR) build
	$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.cppm
	@$(MKDIR) $(dir $(MOD_DIR)/$*.pcm) $(@D) $(if $(GENERATE_COMPDB),$(COMPDB_DIR))
	$(CXX) $(CXXFLAGS) $(MOD_FLAGS) $(LINK_MOD_FLAGS) $(MJFLAGS) -fmodule-output=$(MOD_DIR)/$*.pcm -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(MODULE_OBJS)
	@$(MKDIR) $(@D) $(if $(GENERATE_COMPDB),$(COMPDB_DIR))
	$(CXX) $(CXXFLAGS) $(LINK_MOD_FLAGS) $(MJFLAGS) -c $< -o $@

clean:
	$(RM) build/$(MODE)
	$(RM) $(TARGET)

clean_all:
	$(RM) build
	$(RM) $(COMPDB_FILE)
