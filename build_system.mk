##### CMake-inspired build system

# https://stackoverflow.com/questions/16144115/makefile-remove-duplicate-words-without-sorting
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

ALL_TARGETS=$(EXECUTABLE_TARGETS) $(STATIC_LIB_TARGETS) $(SHARED_LIB_TARGETS) $(OBJECT_LIB_TARGETS)

.PHONY: all
all: $(ALL_TARGETS);

.PHONY: clean
clean: $(foreach TARGET,$(ALL_TARGETS),clean_$(TARGET));

.PHONY: debug
debug: $(foreach TARGET,$(ALL_TARGETS) $(PSEUDO_TARGETS),debug_$(TARGET));

# The eval make_xxx_prologue, eval make_xxx approach keeps the amount of $$ escaping in make_xxx small
define make_transitive_target_variables_prologue
$(TARGET)_PUBLIC_$(ATTR)_SAVED := $($(TARGET)_PUBLIC_$(ATTR))
$(TARGET)_PRIVATE_$(ATTR)_SAVED := $($(TARGET)_PRIVATE_$(ATTR))
$(TARGET)_INTERFACE_$(ATTR)_SAVED := $($(TARGET)_INTERFACE_$(ATTR))

endef

define make_transitive_target_variables
$(TARGET)_PUBLIC_$(ATTR) = $($(TARGET)_PUBLIC_$(ATTR)_SAVED) 
$(TARGET)_PUBLIC_$(ATTR) += $(foreach DEP,$($(TARGET)_PUBLIC_DEPS),$$($(DEP)_PUBLIC_$(ATTR)) $$($(DEP)_INTERFACE_$(ATTR)))

$(TARGET)_PRIVATE_$(ATTR) = $($(TARGET)_PRIVATE_$(ATTR)_SAVED) 
$(TARGET)_PRIVATE_$(ATTR) += $(foreach DEP,$($(TARGET)_PRIVATE_DEPS),$$($(DEP)_PUBLIC_$(ATTR)) $$($(DEP)_INTERFACE_$(ATTR)))

$(TARGET)_INTERFACE_$(ATTR) = $($(TARGET)_INTERFACE_$(ATTR)_SAVED)
$(TARGET)_INTERFACE_$(ATTR) += $(foreach DEP,$($(TARGET)_INTERFACE_DEPS),$$($(DEP)_PUBLIC_$(ATTR)) $$($(DEP)_INTERFACE_$(ATTR)))

$(TARGET)_OWN_$(ATTR) = $$($(TARGET)_PUBLIC_$(ATTR)) $$($(TARGET)_PRIVATE_$(ATTR))

endef

ATTR_LIST := SOURCES FLAGS OBJECT_FLAGS

define make_derived_target_variables_prologue
$$(foreach ATTR,$(ATTR_LIST),$$(eval $$(make_transitive_target_variables_prologue)) $$(eval $$(make_transitive_target_variables)))

endef

define make_derived_target_variables
$(TARGET)_CXX_SOURCES = $(filter %.cpp,$($(TARGET)_OWN_SOURCES))
$(TARGET)_CXX_OBJECTS = $$(patsubst %.cpp,%.o,$$($(TARGET)_CXX_SOURCES))
$(TARGET)_C_SOURCES = $(filter %.c,$($(TARGET)_OWN_SOURCES))
$(TARGET)_C_OBJECTS = $$(patsubst %.c,%.o,$$($(TARGET)_C_SOURCES))
$(TARGET)_OBJECTS = $$($(TARGET)_CXX_OBJECTS) $$($(TARGET)_C_OBJECTS)

endef

define make_executable_target_variables
$(TARGET)_IS_EXECUTABLE:=yes
$(TARGET)_BINARY:=$(TARGET)

endef

define make_shared_lib_target_variables
$(TARGET)_IS_SHARED_LIB:=yes
$(TARGET)_BINARY:=lib$(TARGET).so

endef

define make_static_lib_target_variables
$(TARGET)_IS_STATIC_LIB:=yes
$(TARGET)_BINARY:=lib$(TARGET).a

endef

define make_object_lib_target_variables
$(TARGET)_IS_OBJECT_LIB:=yes
$(TARGET)_BINARY:=

endef

define make_relocatable_object
ifneq ($(strip $($(TARGET)_C_OBJECTS)),)
$(call prepend_build_dir,$($(TARGET)_C_OBJECTS)): $(call prepend_build_dir,%.o) : %.c
	$(CC) -o $$@ \
	$(FLAGS) \
	$(CFLAGS) \
	$(OBJECT_FLAGS) \
	$(OBJECT_CFLAGS) \
	$(if $($(TARGET)_IS_EXECUTABLE),$(EXECUTABLE_FLAGS) $(EXECUTABLE_OBJECT_CFLAGS)) \
	$(if $($(TARGET)_IS_SHARED_LIB),$(SHARED_LIB_FLAGS) $(SHARED_LIB_OBJECT_CFLAGS)) \
	$(if $($(TARGET)_IS_STATIC_LIB),$(STATIC_LIB_FLAGS) $(STATIC_LIB_OBJECT_CFLAGS)) \
	$(if $($(TARGET)_IS_OBJECT_LIB),$(OBJECT_LIB_FLAGS) $(OBJECT_LIB_OBJECT_CFLAGS)) \
	$($(TARGET)_FLAGS) \
	$($(TARGET)_C_FLAGS) \
	$($(TARGET)_OBJECT_FLAGS) \
	$($(TARGET)_OBJECT_CFLAGS) \
	$$^
endif

ifneq ($(strip $($(TARGET)_CXX_OBJECTS)),)
$(call prepend_build_dir,$($(TARGET)_CXX_OBJECTS)): $(call prepend_build_dir,%.o) : %.cpp
	$(CXX) -o $$@ \
	$(FLAGS) \
	$(CXXFLAGS) \
	$(OBJECT_FLAGS) \
	$(OBJECT_CXXFLAGS) \
	$(if $($(TARGET)_IS_EXECUTABLE),$(EXECUTABLE_FLAGS) $(EXECUTABLE_OBJECT_CXXFLAGS)) \
	$(if $($(TARGET)_IS_SHARED_LIB),$(SHARED_LIB_FLAGS) $(SHARED_LIB_OBJECT_CXXFLAGS)) \
	$(if $($(TARGET)_IS_STATIC_LIB),$(STATIC_LIB_FLAGS) $(STATIC_LIB_OBJECT_CXXFLAGS)) \
	$(if $($(TARGET)_IS_OBJECT_LIB),$(OBJECT_LIB_FLAGS) $(OBJECT_LIB_OBJECT_CXXFLAGS)) \
	$($(TARGET)_FLAGS) \
	$($(TARGET)_CXXFLAGS) \
	$($(TARGET)_OBJECT_FLAGS) \
	$($(TARGET)_OBJECT_CXXFLAGS) \
	$$^
endif

endef

define make_clean_target
.PHONY: clean_$(TARGET)
clean_$(TARGET):
	-rm -rf $($(TARGET)_BINARY) $($(TARGET)_OBJECTS)

endef

define make_debug_target
.PHONY: debug_$(TARGET)
debug_$(TARGET):
	@echo TARGET = $(TARGET)
	@echo type = $(if $($(TARGET)_IS_EXECUTABLE),exe,$(if $($(TARGET)_IS_SHARED_LIB),shared library,$(if $($(TARGET)_IS_STATIC_LIB),static library,$(if $($(TARGET)_IS_OBJECT_LIB),object library,psuedo library))))
	@$(foreach ATTR,$(ATTR_LIST),\
	echo $(TARGET)_PUBLIC_$(ATTR) = $($(TARGET)_PUBLIC_$(ATTR));\
	echo $(TARGET)_PRIVATE_$(ATTR) = $($(TARGET)_PRIVATE_$(ATTR));\
	echo $(TARGET)_INTERFACE_$(ATTR) = $($(TARGET)_INTERFACE_$(ATTR));\
	)
	@echo $(TARGET)_OBJECTS = $(call prepend_build_dir,$($(TARGET)_OBJECTS))
	@echo $(TARGET)_BINARY = $(call prepend_build_dir,$($(TARGET)_BINARY))
	@echo

endef

prepend_build_dir = $(addprefix $(BUILD_DIR)/,$(1))

define make_targets
ifneq ($($(TARGET)_BINARY),)

.PHONY: $(TARGET)
$(TARGET): $(call prepend_build_dir,$($(TARGET)_BINARY));

$(call prepend_build_dir,$($(TARGET)_BINARY)): $(call prepend_build_dir,$($(TARGET)_OBJECTS))

else

.PHONY: $(TARGET)
$(TARGET): $(call prepend_build_dir,$($(TARGET)_OBJECTS));

endif

$(make_clean_target)

$(make_debug_target)

endef

define make_executable_targets
$(call prepend_build_dir,$($(TARGET)_BINARY)):
	$(CXX) -o $$@ $(FLAGS) $(LDFLAGS) $(EXECUTABLE_LDFLAGS) $($(TARGET)_OBJECTS) $(foreach DEP,$($(TARGET)_DEPS),$($(DEP)_BINARY))

endef

define make_shared_lib_targets
$(call prepend_build_dir,$($(TARGET)_BINARY)):
	$(CXX) -o $$@ $(FLAGS) $(LDFLAGS) $(SHARED_LIB_LDFLAGS) $$^

endef

define make_static_lib_targets
$(call prepend_build_dir,$($(TARGET)_BINARY)):
	$(AR) rs $$@ $$^

endef

define make_object_lib_targets

endef

$(foreach TARGET,$(ALL_TARGETS) $(PSEUDO_TARGETS),$(eval $(make_derived_target_variables_prologue)) $(eval $(make_derived_target_variables)))
$(foreach TARGET,$(EXECUTABLE_TARGETS),$(eval $(make_executable_target_variables)))
$(foreach TARGET,$(SHARED_LIB_TARGETS),$(eval $(make_shared_lib_target_variables)))
$(foreach TARGET,$(STATIC_LIB_TARGETS),$(eval $(make_static_lib_target_variables)))
$(foreach TARGET,$(OBJECT_LIB_TARGETS),$(eval $(make_object_lib_target_variables)))
$(foreach TARGET,$(ALL_TARGETS),$(eval $(make_relocatable_object)))
$(foreach TARGET,$(ALL_TARGETS),$(eval $(make_targets)))
$(foreach TARGET,$(PSEUDO_TARGETS),$(eval $(make_debug_target)))
$(foreach TARGET,$(EXECUTABLE_TARGETS),$(eval $(make_executable_targets)))
$(foreach TARGET,$(SHARED_LIB_TARGETS),$(eval $(make_shared_lib_targets)))
$(foreach TARGET,$(STATIC_LIB_TARGETS),$(eval $(make_static_lib_targets)))
$(foreach TARGET,$(OBJECT_LIB_TARGETS),$(eval $(make_object_lib_targets)))

