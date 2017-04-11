/*
 * plp.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#include "funcionesKernel.h"

void plp() {
	if(conexionConMemoria() == -1){
		printf("No se pudo conectar con la memoria\n");
		//Fijares que hacer si no puede tener acceso a estos modulos
		exit(1);
	}
	if(conexionConFileSystem() == -1){
		printf("No se pudo conectar con el file system\n");
		exit(1);
	}
	trabajarConexionConsola();
}
