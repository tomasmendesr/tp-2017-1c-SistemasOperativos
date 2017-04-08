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

void trabajarConexiones();

int main(int argc, char** argv){

	crearConfig(argc,argv);

	if(conexionConFileSystem() == -1){
		printf("no se pudo conectar con fs\n");
		return EXIT_FAILURE;
	}

	//establecerConexiones();//Conectarse a FS y Memoria

	//trabajarConexiones();//Esto es lo que hace el thread principal, escucha.

	destruirConfiguracionKernel(config);

	return EXIT_SUCCESS;
}

void trabajarConexiones(){

	int socket_servidor_kernel = createServer(IP,config->puerto_PROG,BACKLOG);
	//todo: Checkear esto, BACKLOG es un char* y la funcion espera un int

	int numero_maximo_socket;
	int newSocket;

	fd_set read_fds;
	fd_set socket_master;

	FD_ZERO(&read_fds);
	FD_ZERO(&socket_master);
	FD_SET(socket_servidor_kernel,&socket_master);
	numero_maximo_socket = socket_servidor_kernel;
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
			if(iterador_sockets == socket_servidor_kernel) {

				newSocket = acceptSocket(socket_servidor_kernel);

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
						case HANDSHAKE_PROGRAMA:{
							enviar_paquete_vacio(HANDSHAKE_KERNEL,iterador_sockets);
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
}

int conexionConFileSystem(){

	socketConexionFS = createClient(config->ip_FS, config->puerto_FS);

	if(socketConexionFS == -1){
		return -1;
	}

	enviarHandshake(socketConexionFS, HANDSHAKE_KERNEL, HANDSHAKE_FS);

	printf("conexion con fs establecida");

	return 1;
}


