/*
 * cpu.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/sockets.h>

#define configuracionCPU "../confCpu.init"

typedef struct {
		char* puerto_Memoria;
        char* ip_Memoria;
        char* puerto_Kernel;
        char* ip_Kernel;

}t_config_cpu;

t_config_cpu* levantarConfiguracionCPU(char* archivo);
int conexionConKernel();

//Variables Globales
t_config_cpu* config;
int socketConexionKernel;
t_log* logger;

#endif /* FUNCIONESCPU_H_ */
