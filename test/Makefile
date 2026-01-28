# ===============================
# DPDK test Makefile
# ===============================

APP := test_dpdk
SRC := test_dpdk.c

# 允许从外部覆盖（方便多版本 DPDK）
DPDK_ROOT ?= /home/kay/codebase/test/dpdk/deps/dpdk/x86_64/25.11
DPDK_LIB  := $(DPDK_ROOT)/lib/x86_64-linux-gnu

CC ?= gcc

# pkg-config
PKG_CONFIG ?= pkg-config
DPDK_CFLAGS  := $(shell $(PKG_CONFIG) --cflags libdpdk)
DPDK_LDFLAGS := $(shell $(PKG_CONFIG) --libs   libdpdk)

# 强制注入 RUNPATH（关键）
RPATH_FLAGS := -Wl,-rpath,$(DPDK_LIB)

CFLAGS  ?= -O2 -g -Wall -Wextra
LDFLAGS ?=

# ===============================
# Targets
# ===============================

.PHONY: all clean run sudo-run info

all: $(APP)

$(APP): $(SRC)
	@echo "  CC  $@"
	$(CC) $(CFLAGS) $(DPDK_CFLAGS) \
	      $(SRC) \
	      $(DPDK_LDFLAGS) $(RPATH_FLAGS) \
	      -o $@

run: $(APP)
	@echo "== Run (no sudo) =="
	./$(APP) -l 0 -n 4 \
	  --vdev=net_null0

sudo-run: $(APP)
	@echo "== Run (sudo) =="
	sudo ./$(APP) -l 0 -n 4 \
	  --vdev=net_null0

info: $(APP)
	@echo "== ELF RUNPATH =="
	readelf -d $(APP) | egrep 'RPATH|RUNPATH' || true
	@echo
	@echo "== Linked libs =="
	ldd $(APP) || true

clean:
	rm -f $(APP)

