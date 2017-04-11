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

	esperarConexiones();

	//LevantarServer

	levantarInterfaz();

    destruirConfiguracionMemoria(config);

    return EXIT_SUCCESS;
}

int esperarConexiones(){

		int socketEscuchaConexiones = createServer(IP, config->puerto, BACKLOG);

		int numero_maximo_socket;
		int newSocket;

		fd_set read_fds;
		fd_set socket_master;

		FD_ZERO(&read_fds);
		FD_ZERO(&socket_master);
		FD_SET(socketEscuchaConexiones,&socket_master);
		numero_maximo_socket = socketEscuchaConexiones;
		int iterador_sockets;
		void* paquete;

		while(1){
			read_fds = socket_master;

			if(select(numero_maximo_socket + 1, &read_fds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(1);
			}
		for(iterador_sockets = 0; iterador_sockets <= numero_maximo_socket; iterador_sockets++) {
			if(FD_ISSET(iterador_sockets, &read_fds)) {
				if(iterador_sockets == socketEscuchaConexiones) {

					newSocket = acceptSocket(socketEscuchaConexiones);

					if(newSocket == -1) {
						perror("Error al aceptar");
					} else {
						FD_SET(newSocket, &socket_master);
						if(newSocket > numero_maximo_socket) numero_maximo_socket = newSocket;
					}
				} else {
					//Gestiono cada conexi�n -> Recibo los programas y creo sus PCB.
					int tipo_mensaje; //Para que la funcion recibir_string lo reciba
					int check = recibir_paquete(iterador_sockets, &paquete, &tipo_mensaje);

					//Chequeo de errores
					if (check == 0) {
						printf("Se cerro el socket %d\n", iterador_sockets);
						close(iterador_sockets);
						FD_CLR(iterador_sockets, &socket_master);
					}

					if(check == -1){
						perror("recv");
						close(iterador_sockets);
						FD_CLR(iterador_sockets, &socket_master);
					}
					// Fin chequeo de errores

					if(check > 0) {
						switch(tipo_mensaje){
							case HANDSHAKE_KERNEL:{
								printf("Conexion con el kernel establecido\n");
								enviar_paquete_vacio(HANDSHAKE_MEMORIA,iterador_sockets);
								//Lanzo el hilo que maneja el kernel
								pthread_t threadKernel;
								pthread_attr_t atributos;
								pthread_attr_init(&atributos);
								pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

								pthread_create(&threadKernel, &atributos, requestHandler, &socketConexionKernel);
								break;
							}
							case HANDSHAKE_CPU:{
								printf("Conexion con la CPU establecido\n");
								enviar_paquete_vacio(HANDSHAKE_MEMORIA,iterador_sockets);
								break;
							}
							default:{ printf("Se recibio un codigo no valido");
								break;
							}
						}
					}
				}
			}
		}
		}




	return 1;
}
