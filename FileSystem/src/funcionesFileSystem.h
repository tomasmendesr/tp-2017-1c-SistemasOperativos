/*
 * funcionesFileSystem.h
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */

#ifndef SRC_FUNCIONESFILESYSTEM_H_
#define SRC_FUNCIONESFILESYSTEM_H_

#include <commons/config.h>

typedef struct{

	int puerto;
	char* punto_Montaje

}t_config_fs;

t_config_fs* levantarConfiguracionFileSystem(char* archivo);

#endif /* SRC_FUNCIONESFILESYSTEM_H_ */
