/*
 * operaciones.h
 *
 *  Created on: 21/4/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <commons/sockets.h>
#include "funcionesKernel.h"

void trabajarMensajeConsola(int socketConsola);
void procesarMensajeConsola(int consola_fd, int mensaje, char* package);
void trabajarMensajeCPU(int socketCPU);
void procesarMensajeCPU(int socketCPU, int mensaje, char* package);
void realizarSignal(int socketCPU, char* key);
void realizarWait(int socketCPU, char* key);
void enviarValorSemaforo(int socketCPU, int tipoMensaje, int valorSemaforo);

#endif /* OPERACIONES_H_ */
