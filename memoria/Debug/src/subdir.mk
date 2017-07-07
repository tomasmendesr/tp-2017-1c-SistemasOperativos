################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/funcionesMemoria.c \
../src/hash.c \
../src/memoria.c 

OBJS += \
./src/funcionesMemoria.o \
./src/hash.o \
./src/memoria.o 

C_DEPS += \
./src/funcionesMemoria.d \
./src/hash.d \
./src/memoria.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2017-1c-Dirty-Cow/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


