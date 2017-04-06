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

int main(int argc, char** argv) {

	char *pathConfig=string_new();

	if(!argv[1])string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionCPU);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionCPU(pathConfig);
	}else{
		printf("No se pudo levantar archivo configuracion\n");
		return EXIT_FAILURE;
	}

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	free(config);
	return EXIT_SUCCESS;
}
