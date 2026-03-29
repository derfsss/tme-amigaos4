# vulkan-sdk Makefile — Builds libvulkan_loader.a for clib4
#
# Usage: make          — build library
#        make test     — build and link test program
#        make clean    — remove build artifacts
#        make verify   — check symbols in built library

CC       = ppc-amigaos-gcc
AR       = ppc-amigaos-ar
NM       = ppc-amigaos-nm
CFLAGS   = -mcrt=clib4 -O2 -Wall -Werror \
           -I include
LDFLAGS  = -mcrt=clib4

BUILD    = build
LIB      = lib
SRC      = src
TEST     = test

.PHONY: all clean test verify

all: $(LIB)/libvulkan_loader.a

$(BUILD)/vulkan_autoinit.o: $(SRC)/vulkan_autoinit.c \
                             include/interfaces/vulkan.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB)/libvulkan_loader.a: $(BUILD)/vulkan_autoinit.o
	@mkdir -p $(LIB)
	$(AR) rcs $@ $<

verify: $(LIB)/libvulkan_loader.a
	@echo "=== Checking exported symbols ==="
	$(NM) $< | grep -E "VulkanBase|IVulkan|__init_vulkan|__exit_vulkan"
	@echo "=== Expected: VulkanBase (B), IVulkan (B), __init_vulkan (T), __exit_vulkan (T) ==="

test: $(LIB)/libvulkan_loader.a $(TEST)/test_vulkan_init.c
	@mkdir -p $(BUILD)
	$(CC) $(LDFLAGS) -I include -D__USE_INLINE__ \
	    $(TEST)/test_vulkan_init.c \
	    -L $(LIB) -lvulkan_loader -lauto \
	    -o $(BUILD)/test_vulkan_init
	@echo "=== Test binary built: $(BUILD)/test_vulkan_init ==="

clean:
	rm -rf $(BUILD) $(LIB)
