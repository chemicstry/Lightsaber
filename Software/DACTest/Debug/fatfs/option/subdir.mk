################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fatfs/option/cc936.c \
../fatfs/option/cc949.c \
../fatfs/option/cc950.c \
../fatfs/option/ccsbcs.c \
../fatfs/option/syscall.c \
../fatfs/option/unicode.c 

OBJS += \
./fatfs/option/cc936.o \
./fatfs/option/cc949.o \
./fatfs/option/cc950.o \
./fatfs/option/ccsbcs.o \
./fatfs/option/syscall.o \
./fatfs/option/unicode.o 

C_DEPS += \
./fatfs/option/cc936.d \
./fatfs/option/cc949.d \
./fatfs/option/cc950.d \
./fatfs/option/ccsbcs.d \
./fatfs/option/syscall.d \
./fatfs/option/unicode.d 


# Each subdirectory must supply rules for building sources it contributes
fatfs/option/%.o: ../fatfs/option/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -I../fatfs -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


