/*
 * cpu.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>

typedef struct {

        int puerto_Memoria;
        char* ip_Memoria;
        int puerto_Kernel;
        char* ip_Kernel;


}t_config_cpu;

t_config_cpu* levantarConfiguracionCPU(char* archivo);

#endif /* CPU_H_ */
