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

#define configuracionCPU "confCpu.init"

typedef struct {

        int puerto_Memoria;
        char* ip_Memoria;
        int puerto_Kernel;
        char* ip_Kernel;


}t_config_cpu;

t_config_cpu* levantarConfiguracionCPU(char* archivo);

//Variables Globales
t_config_cpu* config;

#endif /* FUNCIONESCPU_H_ */
