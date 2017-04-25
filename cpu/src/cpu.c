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

	// Conecta y obtiene tamanio de pagina (memoria) y de stack (kernel).
	if(conexionConKernel() == -1 || conexionConMemoria() == -1){
		return EXIT_FAILURE;
	}
	ejecutarPrograma();
	/*
	 * Manejo de la interrupcion SIGUSR1
	 */
	signal(SIGUSR1, revisarSigusR1);
	while(true){
		log_info(logger, "Comenzando ejecucion...");
		if(requestHandlerKernel() != 0) return -1;
	}

	freeConf(config);
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	return EXIT_SUCCESS;
}

