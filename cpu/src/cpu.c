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


	crearConfig(argc,argv);

	// Conecta con kernel
	if(conexionConKernel() == -1){
		log_error(logger, "No se pudo conectar con el kernel");
		return EXIT_FAILURE;
	}
	// Conecta con memoria
	if(conexionConMemoria() == -1){
		log_error(logger, "No se pudo conectar con la memoria");
		return EXIT_FAILURE;
	}

	free(config);
	finalizarConexion(socketConexionKernel);
	return EXIT_SUCCESS;
}
