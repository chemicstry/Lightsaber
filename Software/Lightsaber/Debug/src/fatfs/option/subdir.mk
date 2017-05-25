################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fatfs/option/cc936.c \
../src/fatfs/option/cc949.c \
../src/fatfs/option/cc950.c \
../src/fatfs/option/ccsbcs.c \
../src/fatfs/option/syscall.c \
../src/fatfs/option/unicode.c 

OBJS += \
./src/fatfs/option/cc936.o \
./src/fatfs/option/cc949.o \
./src/fatfs/option/cc950.o \
./src/fatfs/option/ccsbcs.o \
./src/fatfs/option/syscall.o \
./src/fatfs/option/unicode.o 

C_DEPS += \
./src/fatfs/option/cc936.d \
./src/fatfs/option/cc949.d \
./src/fatfs/option/cc950.d \
./src/fatfs/option/ccsbcs.d \
./src/fatfs/option/syscall.d \
./src/fatfs/option/unicode.d 


# Each subdirectory must supply rules for building sources it contributes
src/fatfs/option/%.o: ../src/fatfs/option/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


