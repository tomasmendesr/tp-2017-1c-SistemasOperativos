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
	if(socketEscuchaConexiones != -1){
		printf("Esperando conexion del kernel.......\n");
	}else{
		printf("Error al levantar servidor\n");
		return -1;
	}
	socketConexionKernel = acceptSocket(socketEscuchaConexiones);
	if(socketConexionKernel < 0) return -1;

	recibirHanshake(socketConexionKernel, HANDSHAKE_KERNEL, HANDSHAKE_MEMORIA);
	//Esto se deberia checkear

	printf("kernel Conectado\n");

	//Lanzo el hilo que maneja el kernel
	pthread_t threadKernel;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

	pthread_create(&threadKernel, &atributos, requestHandler, &socketConexionKernel);

	return 1;
}
