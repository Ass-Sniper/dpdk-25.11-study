APP := tenant-qos
BUILD_DIR := build
SRC_DIR := src

CC ?= aarch64-linux-gnu-gcc
PKG_CONFIG ?= pkg-config

DPDK_CFLAGS := $(shell $(PKG_CONFIG) --cflags libdpdk 2>/dev/null)
DPDK_LDFLAGS := $(shell $(PKG_CONFIG) --libs libdpdk 2>/dev/null)

CFLAGS ?= -O3 -g -Wall -Wextra -Wformat -Wformat-security
CFLAGS += -I$(SRC_DIR)
LDFLAGS ?=

SRCS := $(wildcard $(SRC_DIR)/*.c) \
        $(wildcard $(SRC_DIR)/dataplane/*.c) \
        $(wildcard $(SRC_DIR)/controlplane/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DPDK_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(APP): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(DPDK_LDFLAGS) -o $@

.PHONY: all clean

all: $(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
