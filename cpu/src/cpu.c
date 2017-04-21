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

	conexionConKernel(); // Conecta con kernel
	conexionConMemoria(); // Conecta con memoria y se queda esperando a que le diga el tamaño de pagina

	while(true){
		log_info(logger, "Comenzando ejecucion...");
		if(atenderKernel() != 0) return -1;
		ejecutarPrograma();
	}

	freeConf(config);
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	return EXIT_SUCCESS;
}

