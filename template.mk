# Template for use with the configure.sh script
SRC_DIR := @SRC_DIR@
MAKE_ARGS := @MAKE_ARGS@

.PHONY: all clean debug
all clean debug:
	make -C $(SRC_DIR) BUILD_DIR=$$(pwd) $(MAKE_ARGS) $@

echo_test:
	echo $(PWD) # This prints the where make is being called
	echo $(CURDIR) # This prints the directory after -C is resolved
	# which is usually the directory of this makefile
	# The only way that can break is if -f is used.
	echo $$(pwd) # This prints the directory of this makefile, this should be preferred
