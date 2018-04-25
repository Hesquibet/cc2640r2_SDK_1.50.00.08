SIMPLELINK_CC2640R2_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC2640R2_LAUNCHXL/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = main_tirtos.obj pinStandby.obj CC2640R2_LAUNCHXL.obj ccfg.obj

CONFIGPKG = $(KERNEL_BUILD)/gcc

NAME = pinStandby

CFLAGS = -I../.. -DDeviceFamily_CC26X0R2 "-I$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source" "-I$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/sysbios/posix" -mcpu=cortex-m3 -march=armv7-m -mthumb -mfloat-abi=soft -ffunction-sections -fdata-sections -g -gstrict-dwarf -Wall "-I$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano"

LFLAGS = -Wl,-T,../../tirtos/gcc/CC2640R2_LAUNCHXL_TIRTOS.lds "-Wl,-Map,$(NAME).map" "-Wl,-T,$(KERNEL_BUILD)/gcc/linker.cmd" "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/display/lib" -l:display.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/mw/lcd/lib" -l:lcd.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/grlib/lib" -l:grlib.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/rf/lib" -l:rf_multiMode_cc26x0r2.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/lib" -l:drivers_cc26x0r2.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/pdm/lib" -l:pdm_cc26x0r2.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib" -l:dpl_cc26x0r2.am3g "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/devices/cc26x0r2/driverlib/bin/gcc" -l:driverlib.lib -march=armv7-m -mthumb -nostartfiles -static -Wl,--gc-sections "-L$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7-m" -lgcc -lc -lm -lnosys --specs=nano.specs

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

main_tirtos.obj: ../../tirtos/main_tirtos.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

pinStandby.obj: ../../pinStandby.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

CC2640R2_LAUNCHXL.obj: ../../CC2640R2_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK)  $(OBJECTS) $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
