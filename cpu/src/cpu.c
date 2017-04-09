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
	crearLog();

	// Leer config
	char *pathConfig=string_new();

	if(argv[1]!=NULL)string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionCPU);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionCPU(pathConfig);
	}else{
		printf("No se pudo levantar archivo configuracion\n");
		return EXIT_FAILURE;
	}

	// Conecta con kernel
	if(conexionConKernel() == -1){
		printf("No se pudo conectar con el kernel\n");
		return EXIT_FAILURE;
	}

	free(config);
	finalizarConexion(socketConexionKernel);
	return EXIT_SUCCESS;
}
