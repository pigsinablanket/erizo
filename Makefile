# Source files
SRC :=       \
	src/ao       \
	src/app      \
	src/backdrop \
	src/bitmap   \
	src/camera   \
	src/instance \
	src/icosphere\
	src/loader   \
	src/log      \
	src/mat      \
	src/model    \
	src/shader   \
	src/theme    \
	src/vset     \
	src/window   \
	src/worker   \
	# end of source files

# Force the version-generation script to run before
# anything else, generating version.c and log_align.h
_GEN := $(shell sh gen.sh $(SRC))

# Generated files
# (listed separately so that 'make clean' deletes them)
GEN :=       \
	src/version  \
	# end of generated files

BUILD_DIR := build

CFLAGS := -Wall -Werror -g -O3 -pedantic -Iinc -Ivendor
LDFLAGS = -L/usr/local/lib -lglfw -lglew $(CFLAGS)

# Build with Clang's undefined behavior sanitizer:
# make clean; env UBSAN=1 make
ifeq ($(UBSAN),1)
	CFLAGS  += -fsanitize=undefined
	LDFLAGS += -lstdc++ -lc++abi
endif

# Build with Clang's address sanitizer:
# make clean; env ASAN=1 make
ifeq ($(ASAN),1)
	CFLAGS  += -fsanitize=address
endif

# Platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	SRC +=  platform/darwin platform/posix
	LDFLAGS += -framework Foundation             \
	           -framework Cocoa                  \
	           -framework OpenGL                 \
	           -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
	PLATFORM += -DPLATFORM_DARWIN
endif

OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:=.o) $(GEN:=.o))
DEP := $(OBJ:.o=.d)

all: erizo erizo-test

erizo: $(BUILD_DIR)/src/main.o $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

erizo-test: $(BUILD_DIR)/src/test.o $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PLATFORM) -c -o $@ -std=c99 $<
$(BUILD_DIR)/%.o: %.mm | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PLATFORM) -c -o $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
$(BUILD_DIR)/%.d: %.c | $(BUILD_DIR)
	$(CC) $< $(PLATFORM) $(CFLAGS) -MM -MT $(@:.d=.o) > $@
$(BUILD_DIR)/%.d: %.mm | $(BUILD_DIR)
	$(CC) $< $(PLATFORM) -Iinc -MM -MT $(@:.d=.o) > $@
endif

$(BUILD_DIR):
	mkdir -p $(sort $(dir $(OBJ)))

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(GEN:=.c)
	rm -f erizo
	rm -f erizo-test
