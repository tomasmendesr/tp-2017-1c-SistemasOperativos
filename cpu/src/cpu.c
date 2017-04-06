/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "funcionesCpu.h"

int main(void) {

	if(verificarExistenciaDeArchivo(configuracionCPU)){
		config = levantarConfiguracionCPU(configuracionCPU);
	}else{
		printf("No se pudo levantar archivo configuracion\n");
		return EXIT_FAILURE;
	}

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	free(config);
	return EXIT_SUCCESS;
}
