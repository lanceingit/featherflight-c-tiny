#Program name
PROG = ff
BOARD = LINUX

CMAKE_DIR = cmake

CROSS_COMPILE     ?= ../out/sun8iw8p1/linux/common/buildroot/external-toolchain/bin/arm-linux-gnueabi-

############### Location configuration ################
LOAD_ADDRESS = 0x8000000
LINKER_DIR = make/linker
BUILD_DIR = make/build
BIN_DIR = make/bin
LIB = src/bsp
MAVLINKLIB = src/link/mavlink

################ Build configuration ##################
# St Lib
#VPATH += $(LIB)
#VPATH += $(LIB)/STM32F30x_StdPeriph_Driver/src
#VPATH += $(LIB)/CMSIS/CM4/DeviceSupport/ST/STM32F30x/startup/gcc_ride7
#VPATH += $(LIB)/STM32_USB_Device_Library/Core/src
#VPATH += $(LIB)/STM32_USB_Device_Library/Class/cdc/src
#VPATH += $(LIB)/STM32_USB_OTG_Driver/src
CRT0_CF2 = 
#CRT0_CF2 = startup_stm32f427x.o system_stm32f4xx.o

ST_OBJ=
#ST_OBJ+=misc.o
##ST_OBJ+=stm32f4xx_adc.o
##ST_OBJ+=stm32f4xx_bkp.o
##ST_OBJ+=stm32f4xx_can.o
##ST_OBJ+=stm32f4xx_crc.o
##ST_OBJ+=stm32f4xx_dac.o
##ST_OBJ+=stm32f4xx_dbgmcu.o
#ST_OBJ+=stm32f4xx_dma.o
##ST_OBJ+=stm32f4xx_exti.o
##ST_OBJ+=stm32f4xx_flash.o
##ST_OBJ+=stm32f4xx_fsmc.o
#ST_OBJ+=stm32f4xx_gpio.o
##ST_OBJ+=stm32f4xx_i2c.o
##ST_OBJ+=stm32f4xx_iwdg.o
##ST_OBJ+=stm32f4xx_pwr.o
#ST_OBJ+=stm32f4xx_rcc.o
##ST_OBJ+=stm32f4xx_rtc.o
##ST_OBJ+=stm32f4xx_sdio.o
##ST_OBJ+=stm32f4xx_spi.o
#ST_OBJ+=stm32f4xx_tim.o
#ST_OBJ+=stm32f4xx_usart.o
##ST_OBJ+=stm32f4xx_misc.o
##ST_OBJ+=stm32f4xx_wwdg.o
#ST_OBJ+=stm32f4xx_syscfg.o


# USB obj
#ST_OBJ += usb_core.o usb_dcd_int.o usb_dcd.o
## USB Device obj
#ST_OBJ += usbd_ioreq.o usbd_req.o usbd_core.o usbd_cdc_core.o


# sources
VPATH += src 
VPATH += src/driver src/modules src/link src/utils src/config src/param src/mathlib
#VPATH += src/kernel/FreeRTOS src/kernel/FreeRTOS/portable/MemMang src/kernel/FreeRTOS/portable/Common \
#         src/kernel/FreeRTOS/portable/GCC/ARM_CM4F   
#VPATH += src/utils/libcxx
VPATH += src/pilot src/pilot/navigator src/pilot/estimator


############### Source files configuration ################

# launch
PROJ_OBJ = main.o

# driver
PROJ_OBJ += timer.o  
#PROJ_OBJ += serial.o i2c.o spi.o
#PROJ_OBJ += hmc5883.o mpu6050.o ms5611.o
PROJ_OBJ += mpu6050_linux.o spl06_linux.o dps280_linux.o

# usb
#PROJ_OBJ += usb_bsp.o usbd_desc.o usbd_cdc_vcp.o usbd_usr.o

#kernel
#PROJ_OBJ += tasks.o list.o queue.o port.o heap_4.o

# module
PROJ_OBJ += mavlink_log.o 
PROJ_OBJ += log.o param.o mtd.o param_api.o cli.o
PROJ_OBJ += scheduler.o 

#link
PROJ_OBJ += link_mavlink.o link_wwlink.o 

#pilot
PROJ_OBJ += est.o mixer.o commander.o att_control.o sensor.o navigator.o alt_control.o

#estimator
PROJ_OBJ += att_est_q.o att_est_cf.o alt_est_3o.o alt_est_inav.o

#navgator
PROJ_OBJ += stabilize.o althold.o


#mathlib
PROJ_OBJ += mathlib.o matrix.o vector.o quaternion.o dcm.o srcdkf.o

# utils
PROJ_OBJ += fifo.o perf.o mm.o
PROJ_OBJ += lpf.o pid.o rotation.o


OBJ = $(PROJ_OBJ) $(ST_OBJ) $(CRT0_CF2)


############### Compilation configuration ################
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy

INCLUDES  = -Isrc -Isrc/config
INCLUDES += -Isrc/bsp -Isrc/driver \
			-Isrc/pilot -Isrc/pilot/navigator -Isrc/pilot/estimator\
		    -Isrc/modules -Isrc/link -Isrc/mathlib -Isrc/utils -Isrc/param 
INCLUDES += -I$(MAVLINKLIB) -I$(MAVLINKLIB)/common -Isrc/link/wwlink
#INCLUDES += -I$(LIB)/STM32F30x_StdPeriph_Driver/inc
#INCLUDES += -I$(LIB)/CMSIS/CM4/DeviceSupport/ST/STM32F4xx
#INCLUDES += -I$(LIB)/CMSIS/CM4/CoreSupport
#INCLUDES += -I$(LIB)/STM32_USB_Device_Library/Core/inc
#INCLUDES += -I$(LIB)/STM32_USB_Device_Library/Class/cdc/inc
#INCLUDES += -I$(LIB)/STM32_USB_OTG_Driver/inc
#INCLUDES += -Isrc/kernel -Isrc/kernel/FreeRTOS/include -Isrc/kernel/FreeRTOS/portable/GCC/ARM_CM4F


PROCESSOR = -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon -lrt
CFLAGS += -fno-math-errno 

#Flags required by the ST library
STFLAGS = -lrt -lpthread -ldl -lstdc++ -lm


# Fail on warnings
CFLAGS += -Os

CFLAGS += $(PROCESSOR) $(INCLUDES) $(STFLAGS) -D$(BOARD)

CFLAGS += -Wall -fno-strict-aliasing
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(BUILD_DIR)/dep/$(@).d -MQ $(@)
#Permits to remove un-used functions and global variables from output file
#CFLAGS += -ffunction-sections -fdata-sections
# Prevent promoting floats to doubles
#CFLAGS += -Wdouble-promotion

CCFLAGS += $(CFLAGS) -std=gnu99 -lstdc++

CPPFLAGS += $(CFLAGS) -std=gnu++11

ASFLAGS = $(PROCESSOR) $(INCLUDES)
LDFLAGS = $(PROCESSOR) -Wl,--gc-sections  -lpthread

#LDFLAGS += -T $(LINKER_DIR)/FLASH.ld


#Where to compile the .o
VPATH += $(BUILD_DIR)

#Dependency files to include
DEPS := $(foreach o,$(OBJ),$(BUILD_DIR)/dep/$(o).d)

#################### Targets ###############################


all: gen_param compile size

rebuild: clean all

gen_param:
	sh ./tools/gen_param.sh

compile: $(PROG) 

size: compile
	@$(SIZE) -B $(BIN_DIR)/$(PROG)


$(PROG): $(OBJ) 
	@echo LD $(OBJ) -o $@
	@$(LD) $(LDFLAGS) $(foreach o,$(OBJ),$(BUILD_DIR)/$(o)) -lm -o $(BIN_DIR)/$@

.c.o:
	@echo CC $^ -o $@
	@$(CC) $(CCFLAGS) -c $< -o $(BUILD_DIR)/$@

clean:
	rm -f $(BIN_DIR)/$(PROG).elf $(BIN_DIR)/$(PROG).hex $(BIN_DIR)/$(PROG).px4 \
	$(BIN_DIR)/$(PROG).bin $(BIN_DIR)/$(PROG).dfu $(BIN_DIR)/$(PROG).map \
	$(BUILD_DIR)/dep/*.d $(BUILD_DIR)/*.o $(BIN_DIR)/$(PROG)

define cmake-build
	@cd $(CMAKE_DIR) && cmake .. && $(MAKE)
endef

cmake:
	$(call cmake-build)

.PHONY: all clean cmake	


