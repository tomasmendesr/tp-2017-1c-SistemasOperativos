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

//	freeConf(config);

	// Conecta con memoria
	if(conexionConMemoria() == -1){
		log_error(logger, "No se pudo conectar con la memoria");
		return EXIT_FAILURE;
	}
	// Conecta con kernel
	if(conexionConKernel() == -1){
		log_error(logger, "No se pudo conectar con el kernel");
		return EXIT_FAILURE;
	}

	while(true){
		log_info(logger, "Esperando mensajes del Kernel...");
		atenderKernel();
//		procesarProgramas();
	}

//	free(config);
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	return EXIT_SUCCESS;
}

