################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fileSystem.c \
../src/funcionesFs.c 

OBJS += \
./src/fileSystem.o \
./src/funcionesFs.o 

C_DEPS += \
./src/fileSystem.d \
./src/funcionesFs.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/utnso/tp-2017-1c-Dirty-Cow/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


