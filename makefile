# ----------------------------------------------------------
# ---- C11/C++11 Thread Library ----------------------------
# ----------------------------------------------------------

# Detect OS name
ifeq ($(OS),)
	OS := $(shell uname -s)
endif

# Tools setting
CC  := gcc
CXX := g++
LD  := g++
AR  := ar rcs

# Setting
OUTDIR  := lib
OUTPUT  := $(OUTDIR)/libc11thrd.a
TEMPDIR := temp
INCDIR  :=
INCDIR  += -Iinclude
LIBDIR  :=
CFLAGS  :=
CFLAGS  += -Wall
CFLAGS  += -O3
ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINVER=0x0601
else
	CFLAGS += -pthread
endif
LDFLAGS :=
LDFLAGS += -s
ifneq ($(OS),Windows_NT)
	LDFLAGS += -pthread
endif
SRCS    :=
SRCS    += src/mutex.cpp
SRCS    += src/threads.c
SRCS    += src/spinlock.c
LIBS    :=
OBJS    := $(notdir $(SRCS))
OBJS    := $(addprefix $(TEMPDIR)/,$(OBJS))
OBJS    := $(OBJS:%.c=%.o)
OBJS    := $(OBJS:%.cpp=%.o)
DEPS    := $(OBJS:%.o=%.d)

# Process summary
.PHONY: all clean
.PHONY: pre_step create_dir build_step post_step
.PHONY: install test
all: pre_step create_dir build_step post_step

# Clean process
clean:
ifeq ($(OS),Windows_NT)
	-del /Q $(subst /,\,$(OBJS))
	-del /Q $(subst /,\,$(DEPS))
	-del /Q $(subst /,\,$(OUTPUT))
	-rmdir /Q $(subst /,\,$(TEMPDIR))
	-rmdir /Q $(subst /,\,$(OUTDIR))
else
	-rm -f $(OBJS) $(DEPS) $(OUTPUT)
	-rmdir $(TEMPDIR) $(OUTDIR)
endif

# Build process

pre_step:
create_dir:
ifeq ($(OS),Windows_NT)
	@cmd /c if not exist $(subst /,\,$(TEMPDIR)) mkdir $(subst /,\,$(TEMPDIR))
	@cmd /c if not exist $(subst /,\,$(OUTDIR)) mkdir $(subst /,\,$(OUTDIR))
else
	@test -d $(TEMPDIR) || mkdir $(TEMPDIR)
	@test -d $(OUTDIR)  || mkdir $(OUTDIR)
endif
build_step: $(OUTPUT)
post_step:

$(OUTPUT): $(OBJS)
	$(AR) $(OUTPUT) $(OBJS)

define Compile-C-Unit
$(CC) -MM $(INCDIR) $(CFLAGS) -o $(TEMPDIR)/$*.d $< -MT $@
$(CC) -c  $(INCDIR) $(CFLAGS) -o $@ $<
endef
define Compile-Cpp-Unit
$(CXX) -MM $(INCDIR) $(CFLAGS) -o $(TEMPDIR)/$*.d $< -MT $@
$(CXX) -c  $(INCDIR) $(CFLAGS) -o $@ $<
endef

-include $(DEPS)
$(TEMPDIR)/%.o: src/%.c
	$(Compile-C-Unit)
$(TEMPDIR)/%.o: src/%.cpp
	$(Compile-Cpp-Unit)

# User extended process

install:

test: all
