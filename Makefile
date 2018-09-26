#
# InfOS
#
# Copyright (C) University of Edinburgh 2016.  All Rights Reserved
# Tom Spink <tspink@inf.ed.ac.uk>
#
export MAKEFLAGS    += -rR --no-print-directory
export q	    := @
export arch	    ?= x86

__default: all
	
export top-dir	    := $(CURDIR)
export build-dir    := $(top-dir)/build
export inc-dir	    := $(top-dir)/include
export out-dir	    := $(top-dir)/out

-include $(build-dir)/Makefile.include

export common-flags := -I$(inc-dir) -nostdinc -nostdlib -g -Wall -O3 -std=gnu++17 -fno-pic
export common-flags += -mcmodel=kernel
export common-flags += -ffreestanding -fno-builtin -fno-omit-frame-pointer -fno-rtti -fno-exceptions -fno-stack-protector
export common-flags += -fno-delete-null-pointer-checks -mno-red-zone
export common-flags += -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-avx -mno-aes -mno-sse4a -mno-fma4

export cxxflags	:= $(common-flags)
export asflags	:= $(common-flags)
export ldflags  := -nostdlib -z nodefaultlib 

export target           := $(out-dir)/infos-kernel
export toplevel-obj	:= $(top-dir)/infos-kernel.o
export linker-script    := $(top-dir)/kernel.ld

export source-dirs  := arch/$(arch) kernel/ util/ drivers/ mm/ fs/

ifneq ($(wildcard $(top-dir)/oot/.),)
  export source-dirs += oot/
endif

export main-cpp-src := $(patsubst %,$(top-dir)/%,$(shell find $(source-dirs) | grep -E "\.cpp$$"))
export main-as-src  := $(patsubst %,$(top-dir)/%,$(shell find $(source-dirs) | grep -E "\.S$$"))
export main-obj	    := $(main-cpp-src:.cpp=.o) $(main-as-src:.S=.o)
export main-dep	    := $(main-obj:.o=.d)

all: $(target)
	@echo
	@echo "  InfOS kernel build complete: $(target)"
	@echo
	
clean: .FORCE
	rm -f $(target) $(toplevel-obj) $(main-dep) $(main-obj)
	
sources: .FORCE
	@echo $(main-cpp-src)
	
$(target): $(toplevel-obj) $(linker-script) $(out-dir)
	@echo "  LD       $(BUILD-TARGET).64"
	$(q)$(ld) -n -o $@.64 -T $(linker-script) $(ldflags) $(toplevel-obj)
	
	@echo "  OBJCOPY  $(BUILD-TARGET)"
	$(q)$(objcopy) --input-target=elf64-x86-64 --output-target=elf32-i386 $@.64 $@
	
$(toplevel-obj): $(main-obj)
	@echo "  LD       $(BUILD-TARGET)"
	$(q)$(ld) -r -o $@ $(ldflags) $^
	
$(out-dir):
	@echo "  MKDIR    $(BUILD-TARGET)"
	$(q)mkdir $@

%.o: %.cpp %.d
	@echo "  CXX      $(BUILD-TARGET)"
	$(q)$(cxx) -c -o $@ $(cxxflags) $<

%.o: %.S %.d
	@echo "  AS       $(BUILD-TARGET)"
	$(q)$(cxx) -c -o $@ $(asflags) $<

%.d: %.cpp
	$(q)$(cxx) -M -MT $(@:.d=.o) -o $@ $(cxxflags) $<

%.d: %.S
	$(q)$(cxx) -M -MT $(@:.d=.o) -o $@ $(cxxflags) $<

-include $(main-dep)

.PHONY: __default all clean .FORCE
