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
	inicializaciones();

	conectarConServidores();
	escucharConexiones();

	destruirConfiguracionKernel(config);
	return EXIT_SUCCESS;
}

void conectarConServidores(){
	if(conexionConMemoria() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con la memoria.");
		exit(1);
	}
	if(conexionConFileSystem() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con el File System.");
		exit(1);
	}
}

void escucharConexiones(){
	FD_ZERO(&master);
	FD_ZERO(&setCPUs);
	FD_ZERO(&setConsolas);

	socketEscuchaCPUs = createServer(IP,config->puerto_CPU,BACKLOG);
	socketEscuchaConsolas = createServer(IP,config->puerto_PROG,BACKLOG);

	max_fd = max(socketEscuchaCPUs, socketEscuchaConsolas);

	FD_SET(socketEscuchaCPUs, &master);
	FD_SET(socketEscuchaConsolas, &master);
	fd_set read_fd;
	int iterador_sockets, resultadoHilo;

	while(1){
		read_fd = master;

		if(select(max_fd + 1, &read_fd, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}
		if(FD_ISSET(socketEscuchaCPUs, &read_fd)){ //una cpu quiere conectarses
			aceptarNuevaConexion(socketEscuchaCPUs, &setCPUs);
		}

		if(FD_ISSET(socketEscuchaConsolas, &read_fd)){ //una consola quiere conectarse
			aceptarNuevaConexion(socketEscuchaConsolas, &setConsolas);
		}

		for(iterador_sockets = 0; iterador_sockets <= max_fd; iterador_sockets++) {

			if(FD_ISSET(iterador_sockets, &setCPUs)){ //una cpu realiza una operacion
				FD_CLR(iterador_sockets, &master);
				pthread_t hilo;
				resultadoHilo = pthread_create(&hilo, NULL, (void*)trabajarMensajeCPU, iterador_sockets);
				if(resultadoHilo) exit(1);
			}

			if(FD_ISSET(iterador_sockets, &setConsolas)){ //una consola realiza una operacion
				FD_CLR(iterador_sockets, &master);
				pthread_t hilo;
				resultadoHilo = pthread_create(&hilo, NULL, (void*)trabajarMensajeConsola, iterador_sockets);
				if(resultadoHilo) exit(1);
			}


		}

	}
}


void aceptarNuevaConexion(int socketEscucha, fd_set* set){

	int newSocket = acceptSocket(socketEscucha);

	if(newSocket == -1) {
		perror("Error al aceptar");
	} else {
		FD_SET(newSocket, &master);
		FD_SET(newSocket, set);
		if(newSocket > max_fd) max_fd = newSocket;


		/*-------------------- AGREGADO -----------------------------------------*/
		// TODO - agrego handshakes
		int tipo_mensaje;
		void* paquete;
		int check = recibir_paquete(newSocket, &paquete, &tipo_mensaje);

		//Chequeo de errores
		if (check == 0) {
			printf("Se cerro el socket %d\n", newSocket);
			close(newSocket);
			FD_CLR(newSocket, &master);
		}

		if(check == -1){
			perror("recv");
			close(newSocket);
			FD_CLR(newSocket, &master);
		}
		// Fin chequeo de errores

		if(check > 0) {
			switch(tipo_mensaje){
				case HANDSHAKE_CPU:
					printf("Conexion con nueva cpu establecida\n");
					enviar_paquete_vacio(HANDSHAKE_KERNEL,newSocket);
					enviarTamanioStack(newSocket);
					break;
				case HANDSHAKE_PROGRAMA:
						enviar_paquete_vacio(HANDSHAKE_KERNEL,newSocket);
						printf("Conexion con nueva consola establecida\n");
						break;
				default:
					printf("Se recibio un codigo no valido\n");
					break;
				}
			}
	/* ------------------ FIN AGREGADO ----------------------- */
	}
}

void inicializaciones(){
	sem_init(&mutex_cola_ready,0,1);
	sem_init(&mutex_cola_new,0,1);
	sem_init(&semCPUs, 0, 0);
	inicializarColas();
	listaCPUs = list_create();
}

