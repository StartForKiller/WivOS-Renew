KERNEL	  := wivos
KERNELBIN := $(KERNEL).bin
KERNELELF := $(KERNEL).elf

HDD_FILE  := $(KERNEL).hdd

CFILES    := $(shell find . -type f -name '*.c' | grep -v submodules)
CPPFILES  := $(shell find . -type f -name '*.cpp' | grep -v submodules)
ASMFILES  := $(shell find . -type f -name '*.asm' | grep -v submodules)
REALFILES := $(shell find . -type f -name '*.real' | grep -v submodules)
BINS      := $(REALFILES:.real=.bin)
OBJ       := $(CFILES:.c=.o) $(ASMFILES:.asm=.asm.o) $(CPPFILES:.cpp=.o)
DEPS      := $(CFILES:.c=.d) $(CPPFILES:.cpp=.d)

DBGOUT = no
DBGSYM = no

PREFIX = $(shell pwd)

CC      = gcc
CPP     = g++
LD      = $(CC:gcc=ld)
AS      = nasm

ifeq ($(LINUX), 0)
QEMU    = qemu-system-x86_64.exe
else
QEMU    = qemu-system-x86_64
endif

CPPFLAGS    = -O2 -pipe -Wall -Wextra
LDFLAGS   = -O2

ifeq ($(LINUX), 0)
QEMUFLAGS = -m 2G -accel hax -hda C:/Users/jesus/proyectos/WivOS/$(HDD_FILE) -machine q35
else
QEMUFLAGS = -m 2G -hda $(HDD_FILE) -machine q35
endif

BUILD_TIME := $(shell date)

CPPHARDFLAGS := $(CFLAGS)            \
	-DBUILD_TIME='"$(BUILD_TIME)"' \
	-fno-pic                       \
	-mno-sse                       \
	-mno-sse2                      \
	-mno-mmx                       \
	-mno-80387                     \
	-mno-red-zone                  \
	-mcmodel=kernel                \
	-ffreestanding                 \
	-fno-stack-protector           \
	-fno-omit-frame-pointer        \
	-fno-exceptions				   \
	-fno-rtti					   \
	-I.							   \
	-I./lai/include

CHARDFLAGS := $(CFLAGS)            \
	-DBUILD_TIME='"$(BUILD_TIME)"' \
	-fno-pic                       \
	-mno-sse                       \
	-mno-sse2                      \
	-mno-mmx                       \
	-mno-80387                     \
	-mno-red-zone                  \
	-mcmodel=kernel                \
	-ffreestanding                 \
	-fno-stack-protector           \
	-fno-omit-frame-pointer        \
	-fno-exceptions				   \
	-I.							   \
	-I./lai/include

ifeq ($(DBGOUT), tty)
CPPHARDFLAGS := $(CPPHARDFLAGS) -D_DBGOUT_TTY_
else ifeq ($(DBGOUT), qemu)
CPPHARDFLAGS := $(CPPHARDFLAGS) -D_DBGOUT_QEMU_
else ifeq ($(DBGOUT), both)
CPPHARDFLAGS := $(CPPHARDFLAGS) -D_DBGOUT_TTY_ -D_DBGOUT_QEMU_
endif

ifeq ($(DBGSYM), yes)
CPPHARDFLAGS := $(CPPHARDFLAGS) -g -D_DEBUG_
endif

LDHARDFLAGS := $(LDFLAGS) -nostdlib -no-pie -T linker.ld -z max-page-size=0x1000

QEMUHARDFLAGS := $(QEMUFLAGS)          \
	-debugcon stdio                    \
	# -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0

.PHONY: symlist all prepare build clean run

all:
	echo $(OBJ)
	$(MAKE) build

$(KERNELELF): $(BINS) $(OBJ) symlist
	@printf "Building $(KERNELELF)... "
	@$(LD) $(LDHARDFLAGS) $(OBJ) symlist.o -o $@
	@OBJDUMP=$(CC:-gcc:-objdump) ./gensyms.sh
	@$(CC) -x c $(CHARDFLAGS) -c symlist.gen -o symlist.o
	@$(LD) $(LDHARDFLAGS) $(OBJ) symlist.o -o $@
	@echo "Done"

symlist:
	@echo '#include <symlist.h>' > symlist.gen
	@echo 'struct symlist_t symlist[] = {{0xffffffffffffffff,""}};' >> symlist.gen
	@$(CC) -x c $(CHARDFLAGS) -c symlist.gen -o symlist.o

build: $(KERNELELF)

-include $(DEPS)

%.o: %.c
	@printf "[CC] Compiling $< "
	@$(CC) $(CHARDFLAGS) -MMD -c $< -o $@
	@echo "[Done]"

%.o: %.cpp
	@printf "[CPP] Compiling $< "
	@$(CPP) $(CPPHARDFLAGS) -MMD -c $< -o $@
	@echo "[Done]"

%.bin: %.real
	@$(AS) $< -f bin -o $@

%.asm.o: %.asm
	@printf "[AS] Compiling $< "
	@$(AS) $< -f elf64 -o $@
	@echo "[Done]"

clean:
	@rm -f symlist.gen symlist.o $(OBJ) $(BINS) $(KERNELELF) $(DEPS) $(HDD_FILE)

$(HDD_FILE): build
	@printf "Building Hard Disk... "
	@dd if=/dev/zero bs=1M count=0 seek=64 of=$(HDD_FILE) 2> /dev/null
	@parted -s $(HDD_FILE) mklabel msdos 2> /dev/null
	@parted -s $(HDD_FILE) mkpart primary 1 100% 2> /dev/null
	@echfs-utils -m -p0 $(HDD_FILE) quick-format 32768 2> /dev/null
	@echfs-utils -m -p0 $(HDD_FILE) import qloader2.cfg qloader2.cfg 2> /dev/null
	@echfs-utils -m -p0 $(HDD_FILE) import $(KERNELELF) $(KERNELELF) 2> /dev/null
	@./submodules/qloader2/qloader2-install ./submodules/qloader2/qloader2.bin $(HDD_FILE) 2> /dev/null
	@echo "Done"

ifeq ($(LINUX), 0)
run: $(HDD_FILE)
	@echo "Running Qemu..."
	@rm -fr /mnt/c/Users/jesus/proyectos/WivOS/$(HDD_FILE)
	@cp $(HDD_FILE) /mnt/c/Users/jesus/proyectos/WivOS/
	@$(QEMU) $(QEMUHARDFLAGS)
else
run: $(HDD_FILE)
	@echo "Running Qemu..."
	@$(QEMU) $(QEMUHARDFLAGS)
endif