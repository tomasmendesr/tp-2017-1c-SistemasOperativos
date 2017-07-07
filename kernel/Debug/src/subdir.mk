################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/funcionesKernel.c \
../src/kernel.c \
../src/operaciones.c \
../src/pcb.c 

OBJS += \
./src/funcionesKernel.o \
./src/kernel.o \
./src/operaciones.o \
./src/pcb.o 

C_DEPS += \
./src/funcionesKernel.d \
./src/kernel.d \
./src/operaciones.d \
./src/pcb.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2017-1c-Dirty-Cow/parser" -I"/home/utnso/tp-2017-1c-Dirty-Cow/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


