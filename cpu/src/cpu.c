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
	conexionConKernel();
	conexionConMemoria();
	procesarProgramas();
	freeConf(config);
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	return EXIT_SUCCESS;
}
