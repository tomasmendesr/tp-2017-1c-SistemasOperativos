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

    levantarInterfaz();

    sleep(10000);

	char* pathConfig=string_new();

	if(!argv[1])string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionMemoria);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionMemoria(pathConfig);
	}else{
		printf("No se pudo levantar archivo de configuracion\n");
		return EXIT_FAILURE;
	}

    printf("soy la memoria\n");


    free(config);
    return EXIT_SUCCESS;
}
