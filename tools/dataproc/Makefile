CC := gcc
CFLAGS := -O3 -Wno-unused-result
INCLUDE_DIRS := ../../include ./lib ./lib/include ../nitroarc/lib/include

SRCS := src/common.c lib/dataproc.c lib/json.c lib/yyjson.c lib/libenum.c lib/libexpr.c ../nitroarc/lib/nitroarc.c
INCLUDES = $(addprefix -I,$(INCLUDE_DIRS))

TEMPLATES_DIR := \"data\"
REPO_INCLUDE := \"include\"
REPO_BUILD := \"build\"
DATAPROC_FLAGS := -DTEMPLATES_DIR=$(TEMPLATES_DIR) -DREPO_INCLUDE=$(REPO_INCLUDE) -DREPO_BUILD=$(REPO_BUILD)

.PHONY: all clean

all: moveproc

clean:
	$(RM) moveproc$(EXE)

moveproc: $(SRCS) src/moveproc.c
	$(CC) $(CFLAGS) $(DATAPROC_FLAGS) $^ $(INCLUDES) -o $@
