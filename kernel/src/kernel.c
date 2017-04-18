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

	//me conecto con los servidores (memoria y file system)
	//conectarConServidores();

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
			printf("nueva cpu conectada\n");
		}

		if(FD_ISSET(socketEscuchaConsolas, &read_fd)){ //una consola quiere conectarse
			aceptarNuevaConexion(socketEscuchaConsolas, &setConsolas);
			printf("nueva consola conectada\n");
		}

		for(iterador_sockets = 0; iterador_sockets <= max_fd; iterador_sockets++) {

			if(FD_ISSET(iterador_sockets, &setCPUs)){ //una cpu realiza una operacion
				pthread_t hilo;
				resultadoHilo = pthread_create(&hilo, NULL, (void*)trabajarMensajeCPU, iterador_sockets);
				if(resultadoHilo) return EXIT_FAILURE;
			}

			if(FD_ISSET(iterador_sockets, &setConsolas)){ //una consola realiza una operacion
				pthread_t hilo;
				resultadoHilo = pthread_create(&hilo, NULL, (void*)trabajarMensajeConsola, iterador_sockets);
				if(resultadoHilo) return EXIT_FAILURE;
			}


		}

	}

	destruirConfiguracionKernel(config);
	return EXIT_SUCCESS;
}

void aceptarNuevaConexion(int socketEscucha, fd_set* set){

	int newSocket = acceptSocket(socketEscucha);

	if(newSocket == -1) {
		perror("Error al aceptar");
	} else {
		FD_SET(newSocket, &master);
		FD_SET(newSocket, set);
		if(newSocket > max_fd) max_fd = newSocket;
	}

}

void trabajarMensajeCPU(int socketCPU){
	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;

	int check = recibir_paquete(socketCPU, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if (check <= 0) {
		printf("Se cerro el socket %d\n", socketCPU);
		close(socketCPU);
		FD_CLR(socketCPU, &master);
		FD_CLR(socketCPU, &setCPUs);
	}else{
		procesarMensajeCPU(socketCPU, tipo_mensaje, paquete);
	}

}

void trabajarMensajeConsola(int socketConsola){
	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;

	int check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if (check <= 0) {
		printf("Se cerro el socket %d\n", socketConsola);
		close(socketConsola);
		FD_CLR(socketConsola, &master);
		FD_CLR(socketConsola, &setConsolas);
	}else{
		procesarMensajeConsola(socketConsola, tipo_mensaje, paquete);
	}

}

void inicializaciones(){
	sem_init(&mutex_cola_ready,0,1);
	sem_init(&mutex_cola_new,0,1);
	sem_init(&semCPUs, 0, 0);
	inicializarColas();
	listaCPUs = list_create();
}

