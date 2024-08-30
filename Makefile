AR := ar
CPP := cpp
CC := gcc
CXX := g++

FLAGS :=
CFLAGS :=
CXXFLAGS := -std=c++20 -Werror
LDFLAGS :=
OBJECT_FLAGS := -c -I'$(PWD)' -Iexternal
OBJECT_CFLAGS :=
OBJECT_CXXFLAGS :=
EXECUTABLE_FLAGS := -fpie
SHARED_LIB_FLAGS := -fpic
STATIC_LIB_FLAGS := -fpic
OBJECT_LIB_FLAGS := -fpic
EXECUTABLE_OBJECT_CFLAGS :=
SHARED_LIB_OBJECT_CFLAGS :=
STATIC_LIB_OBJECT_CFLAGS :=
EXECUTABLE_OBJECT_CXXFLAGS := 
SHARED_LIB_OBJECT_CXXFLAGS := 
STATIC_LIB_OBJECT_CXXFLAGS := 

EXECUTABLE_TARGETS := 
SHARED_LIB_TARGETS :=
STATIC_LIB_TARGETS := 
# Use object lib if we just want to make a bunch of relocatable objects (.o) without any further linking/archiving.
# It is a simple way of categorising a bunch of object files we want to build. Useful for development purposes.
OBJECT_LIB_TARGETS := simulator assembler
PSEUDO_TARGETS := linenoise

# In CMake terminology,
# XXX are private
# INTERFACE_XXX are interface
# For simplicity, we don't handle public
# Because then we would need to do topological sort...
# We can use this target based approach to most of the variables
# but for simplicity, let us start with FLAGS, OBJECT_FLAGS and SOURCES.

simulator_SOURCES := instruction.cpp register.cpp machine.cpp

assembler_SOURCES := assembler.cpp

# simulator_DEPS := linenoise

linenoise_DIR := external/linenoise/

linenoise_INTERFACE_OBJECT_FLAGS := 

linenoise_INTERFACE_SOURCES := $(linenoise_DIR)/linenoise.c 

# https://stackoverflow.com/questions/16144115/makefile-remove-duplicate-words-without-sorting
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

.PHONY: all
all: $(EXECUTABLE_TARGETS) $(STATIC_LIB_TARGETS) $(SHARED_LIB_TARGETS) $(OBJECT_LIB_TARGETS);

.PHONY: clean
clean: $(foreach TARGET,$(EXECUTABLE_TARGETS) $(STATIC_LIB_TARGETS) $(SHARED_LIB_TARGETS) $(OBJECT_LIB_TARGETS),clean_$(TARGET));

define make_target_variables_prologue
$(TARGET)_SOURCES_ORIG := $($(TARGET)_SOURCES)
$(TARGET)_SOURCES = $$($(TARGET)_SOURCES_ORIG)
$(TARGET)_SOURCES += $(foreach DEP,$($(TARGET)_DEPS),$($(DEP)_INTERFACE_SOURCES))

$(TARGET)_FLAGS_ORIG := $($(TARGET)_FLAGS)
$(TARGET)_FLAGS = $$($(TARGET)_FLAGS_ORIG)
$(TARGET)_FLAGS += $(foreach DEP,$($(TARGET)_DEPS),$($(DEP)_INTERFACE_FLAGS))

$(TARGET)_OBJECT_FLAGS_ORIG := $($(TARGET)_OBJECT_FLAGS)
$(TARGET)_OBJECT_FLAGS = $$($(TARGET)_OBJECT_FLAGS_ORIG)
$(TARGET)_OBJECT_FLAGS += $(foreach DEP,$($(TARGET)_DEPS),$($(DEP)_INTERFACE_OBJECT_FLAGS))

$$(info $(TARGET): $$($(TARGET)_SOURCES))
endef

define make_target_variables
$(TARGET)_CXX_SOURCES = $(filter %.cpp,$($(TARGET)_SOURCES))
$(TARGET)_CXX_OBJECTS = $$(patsubst %.cpp,%.o,$$($(TARGET)_CXX_SOURCES))
$(TARGET)_C_SOURCES = $(filter %.c,$($(TARGET)_SOURCES))
$(TARGET)_C_OBJECTS = $$(patsubst %.c,%.o,$$($(TARGET)_C_SOURCES))
$(TARGET)_OBJECTS = $$($(TARGET)_CXX_OBJECTS) $$($(TARGET)_C_OBJECTS)

$$(info $(TARGET): $$($(TARGET)_OBJECTS))
endef

# Despite the lack of an explicit enum, exactly one of 
# IS_EXECUTABLE, IS_SHARED_LIB, IS_STATIC_LIB, IS_OBJECT_LIB
# is 'yes'.

define make_executable_relocatable_object_prologue
IS_EXECUTABLE:=yes
IS_SHARED_LIB:=
IS_STATIC_LIB:=
IS_OBJECT_LIB:=

endef

define make_shared_lib_relocatable_object_prologue
IS_EXECUTABLE:=
IS_SHARED_LIB:=yes
IS_STATIC_LIB:=
IS_OBJECT_LIB:=

endef

define make_static_lib_relocatable_object_prologue
IS_EXECUTABLE:=
IS_SHARED_LIB:=
IS_STATIC_LIB:=yes
IS_OBJECT_LIB:=

endef

define make_object_lib_relocatable_object_prologue
IS_EXECUTABLE:=
IS_SHARED_LIB:=
IS_STATIC_LIB:=
IS_OBJECT_LIB:=yes

endef

define make_relocatable_object
ifneq ($(strip $($(TARGET)_C_OBJECTS)),)
$($(TARGET)_C_OBJECTS): %.o : %.c
	$(CC) -o $$@ \
	$(FLAGS) \
	$(CFLAGS) \
	$(OBJECT_FLAGS) \
	$(OBJECT_CFLAGS) \
	$(if $(IS_EXECUTABLE),$(EXECUTABLE_FLAGS) $(EXECUTABLE_OBJECT_CFLAGS)) \
	$(if $(IS_SHARED_LIB),$(SHARED_LIB_FLAGS) $(SHARED_LIB_OBJECT_CFLAGS)) \
	$(if $(IS_STATIC_LIB),$(STATIC_LIB_FLAGS) $(STATIC_LIB_OBJECT_CFLAGS)) \
	$(if $(IS_OBJECT_LIB),$(OBJECT_LIB_FLAGS) $(OBJECT_LIB_OBJECT_CFLAGS)) \
	$($(TARGET)_FLAGS) \
	$($(TARGET)_C_FLAGS) \
	$($(TARGET)_OBJECT_FLAGS) \
	$($(TARGET)_OBJECT_CFLAGS) \
	$$^
endif

ifneq ($(strip $($(TARGET)_CXX_OBJECTS)),)
$($(TARGET)_CXX_OBJECTS): %.o : %.cpp
	$(CXX) -o $$@ \
	$(FLAGS) \
	$(CXXFLAGS) \
	$(OBJECT_FLAGS) \
	$(OBJECT_CXXFLAGS) \
	$(if $(IS_EXECUTABLE),$(EXECUTABLE_FLAGS) $(EXECUTABLE_OBJECT_CXXFLAGS)) \
	$(if $(IS_SHARED_LIB),$(SHARED_LIB_FLAGS) $(SHARED_LIB_OBJECT_CXXFLAGS)) \
	$(if $(IS_STATIC_LIB),$(STATIC_LIB_FLAGS) $(STATIC_LIB_OBJECT_CXXFLAGS)) \
	$(if $(IS_OBJECT_LIB),$(OBJECT_LIB_FLAGS) $(OBJECT_LIB_OBJECT_CXXFLAGS)) \
	$($(TARGET)_FLAGS) \
	$($(TARGET)_CXXFLAGS) \
	$($(TARGET)_OBJECT_FLAGS) \
	$($(TARGET)_OBJECT_CXXFLAGS) \
	$$^
endif

endef

define make_executable_target
$(TARGET): $($(TARGET)_OBJECTS)
	$(CXX) -o $$@ $$^

clean_$(TARGET):
	rm -rf $(TARGET) $($(TARGET)_OBJECTS)

endef

define make_shared_lib_target
.PHONY: $(TARGET)
$(TARGET): lib$(TARGET).so;

lib$(TARGET).so: $($(TARGET)_OBJECTS)
	$(CXX) -o $$@ -shared $$^

.PHONY: clean_$(TARGET)
clean_$(TARGET):
	rm -rf lib$(TARGET).so $($(TARGET)_OBJECTS)

endef

define make_static_lib_target
.PHONY: $(TARGET)
$(TARGET): lib$(TARGET).a;

lib$(TARGET).a: $($(TARGET)_OBJECTS)
	$(AR) rs $$@ $$^

.PHONY: clean_$(TARGET)
clean_$(TARGET):
	rm -rf lib$(TARGET).a $($(TARGET)_OBJECTS)

endef

define make_object_lib_target
.PHONY: $(TARGET)
$(TARGET): $($(TARGET)_OBJECTS);

.PHONY: clean_$(TARGET)
clean_$(TARGET):
	rm -rf $($(TARGET)_OBJECTS)

endef

$(foreach TARGET,$(EXECUTABLE_TARGETS) $(STATIC_LIB_TARGETS) $(SHARED_LIB_TARGETS) $(OBJECT_LIB_TARGETS),$(eval $(make_target_variables_prologue)) $(eval $(make_target_variables)))
EXTERNAL_LIBS:=$(call uniq,$(foreach TARGET,$(EXECUTABLE_TARGETS) $(STATIC_LIB_TARGETS) $(SHARED_LIB_TARGETS),$($(TARGET)_EXTERNAL_LIBS)))
$(foreach TARGET,$(EXECUTABLE_TARGETS),$(eval $(make_executable_relocatable_object_prologue)) $(eval $(make_relocatable_object)))
$(foreach TARGET,$(SHARED_LIB_TARGETS),$(eval $(make_shared_lib_relocatable_object_prologue)) $(eval $(make_relocatable_object)))
$(foreach TARGET,$(STATIC_LIB_TARGETS),$(eval $(make_static_lib_relocatable_object_prologue)) $(eval $(make_relocatable_object)))
$(foreach TARGET,$(OBJECT_LIB_TARGETS),$(eval $(make_object_lib_relocatable_object_prologue)) $(eval $(make_relocatable_object)))
$(foreach TARGET,$(EXECUTABLE_TARGETS),$(eval $(make_executable_target)))
$(foreach TARGET,$(SHARED_LIB_TARGETS),$(eval $(make_shared_lib_target)))
$(foreach TARGET,$(STATIC_LIB_TARGETS),$(eval $(make_static_lib_target)))
$(foreach TARGET,$(OBJECT_LIB_TARGETS),$(eval $(make_object_lib_target)))


