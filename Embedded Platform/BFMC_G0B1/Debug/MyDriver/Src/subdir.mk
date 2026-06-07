################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MyDriver/Src/as5600.c \
../MyDriver/Src/bno055.c \
../MyDriver/Src/i2c.c \
../MyDriver/Src/rcc.c \
../MyDriver/Src/servo.c \
../MyDriver/Src/systick.c \
../MyDriver/Src/timer.c \
../MyDriver/Src/uart.c \
../MyDriver/Src/watchdog.c 

OBJS += \
./MyDriver/Src/as5600.o \
./MyDriver/Src/bno055.o \
./MyDriver/Src/i2c.o \
./MyDriver/Src/rcc.o \
./MyDriver/Src/servo.o \
./MyDriver/Src/systick.o \
./MyDriver/Src/timer.o \
./MyDriver/Src/uart.o \
./MyDriver/Src/watchdog.o 

C_DEPS += \
./MyDriver/Src/as5600.d \
./MyDriver/Src/bno055.d \
./MyDriver/Src/i2c.d \
./MyDriver/Src/rcc.d \
./MyDriver/Src/servo.d \
./MyDriver/Src/systick.d \
./MyDriver/Src/timer.d \
./MyDriver/Src/uart.d \
./MyDriver/Src/watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
MyDriver/Src/%.o MyDriver/Src/%.su MyDriver/Src/%.cyclo: ../MyDriver/Src/%.c MyDriver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32G0 -DSTM32G0B1RETx -c -I../Inc -I"/home/tieuchi/Desktop/Autonomous_Vehicle/Embedded Platform/BFMC_G0B1/MyDriver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-MyDriver-2f-Src

clean-MyDriver-2f-Src:
	-$(RM) ./MyDriver/Src/as5600.cyclo ./MyDriver/Src/as5600.d ./MyDriver/Src/as5600.o ./MyDriver/Src/as5600.su ./MyDriver/Src/bno055.cyclo ./MyDriver/Src/bno055.d ./MyDriver/Src/bno055.o ./MyDriver/Src/bno055.su ./MyDriver/Src/i2c.cyclo ./MyDriver/Src/i2c.d ./MyDriver/Src/i2c.o ./MyDriver/Src/i2c.su ./MyDriver/Src/rcc.cyclo ./MyDriver/Src/rcc.d ./MyDriver/Src/rcc.o ./MyDriver/Src/rcc.su ./MyDriver/Src/servo.cyclo ./MyDriver/Src/servo.d ./MyDriver/Src/servo.o ./MyDriver/Src/servo.su ./MyDriver/Src/systick.cyclo ./MyDriver/Src/systick.d ./MyDriver/Src/systick.o ./MyDriver/Src/systick.su ./MyDriver/Src/timer.cyclo ./MyDriver/Src/timer.d ./MyDriver/Src/timer.o ./MyDriver/Src/timer.su ./MyDriver/Src/uart.cyclo ./MyDriver/Src/uart.d ./MyDriver/Src/uart.o ./MyDriver/Src/uart.su ./MyDriver/Src/watchdog.cyclo ./MyDriver/Src/watchdog.d ./MyDriver/Src/watchdog.o ./MyDriver/Src/watchdog.su

.PHONY: clean-MyDriver-2f-Src

