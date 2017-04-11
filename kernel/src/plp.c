/*
 * plp.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#include "funcionesKernel.h"

void* plp(void *b) {
	if(conexionConMemoria() == -1){
		printf("no se pudo conectar con fs\n");
		return EXIT_FAILURE;
	}
	if(conexionConFileSystem() == -1){
		printf("no se pudo conectar con fs\n");
		return EXIT_FAILURE;
	}
	trabajarConexionConsola();
}
