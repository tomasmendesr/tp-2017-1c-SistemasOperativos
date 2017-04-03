/*
 * funcionesCpu.h
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */

#ifndef SRC_FUNCIONESCPU_H_
#define SRC_FUNCIONESCPU_H_

#include <commons/config.h>

typedef struct {

	int puerto_Memoria;
	char* ip_Memoria;
	int puerto_Kernel;
	char* ip_Kernel;


}t_config_cpu;


t_config_cpu* levantarConfiguracionCPU(char* archivo);


#endif /* SRC_FUNCIONESCPU_H_ */
