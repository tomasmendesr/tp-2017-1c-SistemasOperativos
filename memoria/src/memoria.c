/*
 ============================================================================
 Name        : memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funcionesMemoria.h"

int main(int argc, char** argv){

	if(verificarExistenciaDeArchivo(argv[1])){
		config = levantarConfiguracionMemoria(argv[1]);
	}else{
		printf("No se pudo levantar archivo de configuracion\n");
		return EXIT_FAILURE;
	}

    printf("soy la memoria\n");

    free(config);
    return EXIT_SUCCESS;
}
