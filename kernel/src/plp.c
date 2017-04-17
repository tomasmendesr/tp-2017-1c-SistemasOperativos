/*
 * plp.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#include "funcionesKernel.h"

void plp() {
	if(conexionConMemoria() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con la memoria.");
		exit(1);
	}
	if(conexionConFileSystem() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con el File System.");
		exit(1);
	}
	trabajarConexionConsola();
}

void trabajarConexionConsola(){

	int socket_servidor_kernel = createServer(IP,config->puerto_PROG,BACKLOG);

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
				int check = recibir_info(iterador_sockets, &paquete, &tipo_mensaje);

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
					procesarMensajeConsola(iterador_sockets,tipo_mensaje, (void*)paquete);
				}
			}
		}
	}
	}
}
void procesarMensajeConsola(int consola_fd, int mensaje, char* package){

	pcb_t* nuevoProceso;

	switch(mensaje){
		case HANDSHAKE_PROGRAMA:
			enviar_paquete_vacio(HANDSHAKE_KERNEL,consola_fd);
			log_info(&logger_kernel,"Conexion con la consola establecida");
			break;
		case ENVIO_CODIGO:

			nuevoProceso = crearProceso(consola_fd, package);

			if(nuevoProceso->pid == 0){
				log_error(&logger_kernel,"Se produjo un error en intentar guardar los datos del proceso.");
			}else{
				//Admitido en el sistema con todas sus estructuras guardadas en memoria.
				log_info(&logger_kernel,"Proceso creado correctamente %d",nuevoProceso->pid);
				sem_wait(&mutex_cola_ready);
				queue_push(colaReady,nuevoProceso);
				sem_post(&mutex_cola_ready);
			}
			break;
		case FINALIZAR_PROGRAMA:
			//finalizarPrograma(consola_fd,package);
			break;
		default: log_warning(&logger_kernel,"Se recibio un codigo de operacion invalido.");

		break;
	}
}
