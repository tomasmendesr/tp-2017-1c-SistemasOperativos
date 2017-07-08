################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commons/bitarray.c \
../commons/config.c \
../commons/cosas.c \
../commons/error.c \
../commons/interface.c \
../commons/log.c \
../commons/process.c \
../commons/sockets.c \
../commons/string.c \
../commons/structUtiles.c \
../commons/temporal.c \
../commons/txt.c 

OBJS += \
./commons/bitarray.o \
./commons/config.o \
./commons/cosas.o \
./commons/error.o \
./commons/interface.o \
./commons/log.o \
./commons/process.o \
./commons/sockets.o \
./commons/string.o \
./commons/structUtiles.o \
./commons/temporal.o \
./commons/txt.o 

C_DEPS += \
./commons/bitarray.d \
./commons/config.d \
./commons/cosas.d \
./commons/error.d \
./commons/interface.d \
./commons/log.d \
./commons/process.d \
./commons/sockets.d \
./commons/string.d \
./commons/structUtiles.d \
./commons/temporal.d \
./commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
commons/%.o: ../commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


