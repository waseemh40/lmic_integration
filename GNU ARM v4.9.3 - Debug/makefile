################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include resource\ managers_src/subdir.mk
-include lmic/subdir.mk
-include hal/subdir.mk
-include fifo_buffers_src/subdir.mk
-include fat_filesystem_src/subdir.mk
-include emlib/src/subdir.mk
-include drivers_src/subdir.mk
-include devices_src/subdir.mk
-include CMSIS/EFM32GG/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: lmic_and_testproject_iA.axf

# Tool invocations
lmic_and_testproject_iA.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "lmic_and_testproject_iA.ld" -u _printf_float -Xlinker --gc-sections -Xlinker -Map="lmic_and_testproject_iA.map" --specs=nano.specs -o lmic_and_testproject_iA.axf "./CMSIS/EFM32GG/startup_efm32gg.o" "./CMSIS/EFM32GG/system_efm32gg.o" "./devices_src/rgb_led.o" "./devices_src/sd_card.o" "./devices_src/tbr.o" "./devices_src/ublox_gps.o" "./drivers_src/delay.o" "./drivers_src/pwm.o" "./drivers_src/rs232.o" "./drivers_src/rs485.o" "./drivers_src/spi.o" "./emlib/src/em_acmp.o" "./emlib/src/em_adc.o" "./emlib/src/em_aes.o" "./emlib/src/em_assert.o" "./emlib/src/em_burtc.o" "./emlib/src/em_can.o" "./emlib/src/em_cmu.o" "./emlib/src/em_core.o" "./emlib/src/em_cryotimer.o" "./emlib/src/em_crypto.o" "./emlib/src/em_csen.o" "./emlib/src/em_dac.o" "./emlib/src/em_dbg.o" "./emlib/src/em_dma.o" "./emlib/src/em_ebi.o" "./emlib/src/em_emu.o" "./emlib/src/em_gpcrc.o" "./emlib/src/em_gpio.o" "./emlib/src/em_i2c.o" "./emlib/src/em_idac.o" "./emlib/src/em_int.o" "./emlib/src/em_lcd.o" "./emlib/src/em_ldma.o" "./emlib/src/em_lesense.o" "./emlib/src/em_letimer.o" "./emlib/src/em_leuart.o" "./emlib/src/em_mpu.o" "./emlib/src/em_msc.o" "./emlib/src/em_opamp.o" "./emlib/src/em_pcnt.o" "./emlib/src/em_prs.o" "./emlib/src/em_qspi.o" "./emlib/src/em_rmu.o" "./emlib/src/em_rtc.o" "./emlib/src/em_rtcc.o" "./emlib/src/em_system.o" "./emlib/src/em_timer.o" "./emlib/src/em_usart.o" "./emlib/src/em_vcmp.o" "./emlib/src/em_vdac.o" "./emlib/src/em_wdog.o" "./fat_filesystem_src/diskio.o" "./fat_filesystem_src/ff.o" "./fifo_buffers_src/fifo_rs232.o" "./fifo_buffers_src/fifo_tbr.o" "./hal/debug.o" "./hal/hal.o" "./lmic/aes.o" "./lmic/lmic.o" "./lmic/oslmic.o" "./lmic/radio.o" "./resource managers_src/app_manager.o" "./resource managers_src/lpwan_manager.o" "./src/main.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: lmic_and_testproject_iA.hex'
	arm-none-eabi-objcopy -O ihex "lmic_and_testproject_iA.axf" "lmic_and_testproject_iA.hex"
	@echo ' '

	@echo 'Building bin file: lmic_and_testproject_iA.bin'
	arm-none-eabi-objcopy -O binary "lmic_and_testproject_iA.axf" "lmic_and_testproject_iA.bin"
	@echo ' '

	@echo 'Building s37 file: lmic_and_testproject_iA.s37'
	arm-none-eabi-objcopy -O srec "lmic_and_testproject_iA.axf" "lmic_and_testproject_iA.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "lmic_and_testproject_iA.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) lmic_and_testproject_iA.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
