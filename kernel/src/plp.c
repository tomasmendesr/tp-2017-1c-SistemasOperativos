/*
 * plp.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#include "funcionesKernel.h"

void conectarConServidores() {
	if(conexionConMemoria() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con la memoria.");
		exit(1);
	}
	if(conexionConFileSystem() == -1){
		log_error(&logger_kernel,"No se pudo establecer la conexion con el File System.");
		exit(1);
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
