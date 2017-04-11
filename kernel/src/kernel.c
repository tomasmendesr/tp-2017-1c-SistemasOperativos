/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funcionesKernel.h"


int main(int argc, char** argv){

	crearConfig(argc,argv);

//	if(conexionConMemoria() == -1){
//		printf("no se pudo conectar con fs\n");
//		return EXIT_FAILURE;
//	}
//	if(conexionConFileSystem() == -1){
//		printf("no se pudo conectar con fs\n");
//		return EXIT_FAILURE;
//	}

	trabajarConexionCPU();
	trabajarConexionConsola();//Esto es lo que hace el thread principal, escucha.

	destruirConfiguracionKernel(config);
	return EXIT_SUCCESS;
}

