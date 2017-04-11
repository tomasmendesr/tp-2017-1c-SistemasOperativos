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

	logger = log_create("logMemoria","memoria",true,LOG_LEVEL_TRACE);

	crearConfig(argc, argv);

	inicializarMemoria();

	esperarConexiones();

	levantarInterfaz();

    destruirConfiguracionMemoria(config);

    return EXIT_SUCCESS;
}

void esperarConexiones(){

	int socketEscucha = createServer(IP, config->puerto, BACKLOG);
	int newSocket;
	void* paquete;
	int tipo_mensaje;
	int check;

	while(1){

		newSocket = acceptSocket(socketEscucha);

		if(newSocket == -1)
			perror("Error al aceptar");

		check = recibir_paquete(newSocket, &paquete, &tipo_mensaje);

		//Chequeo de errores
		if (check == 0){
			printf("Se cerro el socket %d\n", newSocket);
			close(newSocket);
		}
		if(check == -1){
			perror("recv");
			close(newSocket);
		}
		//Fin chequeo de errores

		if(check > 0){
			switch(tipo_mensaje){

				case HANDSHAKE_CPU:
					printf("Conexion con la CPU establecido\n");
					enviar_paquete_vacio(HANDSHAKE_MEMORIA,newSocket);
					pthread_t threadCpu;
					pthread_create(&threadCpu, NULL, (void*)requestHandlerCpu, &newSocket);
					pthread_detach(threadCpu);
					break;

				case HANDSHAKE_KERNEL:
					printf("Conexion con el kernel establecido\n");
					enviar_paquete_vacio(HANDSHAKE_MEMORIA,newSocket);
					pthread_t threadKernel;
					pthread_create(&threadKernel, NULL, (void*)requestHandlerKernel, &newSocket);
					pthread_detach(threadKernel);
					break;

				default:
					printf("Solo se aceptan conexiones de Kernel y Cpu\n");
					break;
			}
		}
	}
}

