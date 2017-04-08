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

	log = log_create("logMemoria","memoria",true,LOG_LEVEL_TRACE);

	crearConfig(argc, argv);

	inicializarMemoria();

	esperarConexionKernel();


	//LevantarServer

	levantarInterfaz();

    destruirConfiguracionMemoria(config);

    return EXIT_SUCCESS;
}

int esperarConexionKernel(){

	socketEscuchaConexiones = createServer(IP, config->puerto, BACKLOG);

	socketConexionKernel = acceptSocket(socketEscuchaConexiones);
	if(socketConexionKernel < 0) return -1;

	recibirHanshake(socketConexionKernel, HANDSHAKE_KERNEL, HANDSHAKE_MEMORIA);

	printf("kernel Conectado\n");

	return 1;
}
