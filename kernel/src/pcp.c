/*
 * pcp.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#include "funcionesKernel.h"

void pcp(){
	trabajarConexionCPU();
}


void procesarMensajeCPU(int socketCPU, int mensaje, char* package){

	switch(mensaje){
	case ENVIO_PCB:
		break;
	case SIGNAL:
		realizarSignal(socketCPU, package);
		break;
	case WAIT:
		realizarWait(socketCPU, package);
		break;
	case LEER_VAR_COMPARTIDA:
		break;
	case ASIG_VAR_COMPARTIDA:
		break;
	case IO:
		break;

	default:
		log_warning(&logger_kernel,"Se recibio un codigo de operacion invalido.");
	}

}

void realizarSignal(int socketCPU, char* key){
	int valorSemaforo = semaforoSignal(config->semaforos, key);

	enviarValorSemaforo(socketCPU, SIGNAL, valorSemaforo);
}

void realizarWait(int socketCPU, char* key){
	int valorSemaforo = semaforoWait(config->semaforos, key);

	enviarValorSemaforo(socketCPU, WAIT, valorSemaforo);
}

void enviarValorSemaforo(int socketCPU, int tipoMensaje, int valorSemaforo){
	header_t header;
	header.type = tipoMensaje;
	header.length = sizeof(int);
	sendSocket(socketCPU, &header, &valorSemaforo);
}
