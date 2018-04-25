SIMPLELINK_CC2640R2_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/imports.mak

XDCPATH = $(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source;$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages;

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

XDCTARGET = ti.targets.arm.elf.M3
PLATFORM = ti.platforms.simplelink:CC2640R2F

OBJECTS = swi.obj CC2640R2_LAUNCHXL.obj ccfg.obj

CONFIGPKG = swi

NAME = swi

CFLAGS = -I../.. -DDeviceFamily_CC26X0R2 "-I$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source" "-I$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/sysbios/posix" --silicon_version=7M3 --code_state=16 --little_endian --display_error_number --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --float_support=vfplib --symdebug:dwarf

LFLAGS = "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/display/lib/display.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/mw/lcd/lib/lcd.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/grlib/lib/grlib.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/rf/lib/rf_multiMode_cc26x0r2.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_cc26x0r2.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/drivers/pdm/lib/pdm_cc26x0r2.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib/dpl_cc26x0r2.aem3" "-l$(SIMPLELINK_CC2640R2_SDK_INSTALL_DIR)/source/ti/devices/cc26x0r2/driverlib/bin/ccs/driverlib.lib" ../../tirtos/ccs/CC2640R2_LAUNCHXL_TIRTOS.cmd "-m$(NAME).map" --warn_sections --display_error_number --diag_wrap=off --rom_model "-i$(CCS_ARMCOMPILER)/lib" -llibc.a

.PRECIOUS: $(CONFIGPKG)/compiler.opt $(CONFIGPKG)/linker.cmd

all: $(NAME).out

$(CONFIGPKG)/compiler.opt: $(CONFIGPKG)/linker.cmd

$(CONFIGPKG)/linker.cmd: ../../tirtos/swi.cfg
	@ echo Running Configuro...
	@ $(XDC_INSTALL_DIR)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c "$(CCS_ARMCOMPILER)" -t $(XDCTARGET) -p $(PLATFORM) --compileOptions "$(CFLAGS)" -o "$(CONFIGPKG)" ../../tirtos/swi.cfg

swi.obj: ../../swi.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

CC2640R2_LAUNCHXL.obj: ../../CC2640R2_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK) $(CFLAGS) -z $(OBJECTS) -l$(CONFIGPKG)/linker.cmd $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RMDIR) $(NAME) > $(DEVNULL) 2>&1
