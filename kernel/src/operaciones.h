
#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <commons/sockets.h>
#include "funcionesKernel.h"

extern int socketCPU;

void trabajarMensajeConsola(int socketConsola);
void procesarMensajeConsola(int consola_fd, int mensaje, char* package);
void trabajarMensajeCPU(int socketCPU);
void procesarMensajeCPU(int socketCPU, int mensaje, char* package);
void leerVarCompartida(int socketCPU, char* variable);
void asignarVarCompartida(int socketCPU, void* buffer);
void realizarSignal(int socketCPU, char* key);
void realizarWait(int socketCPU, char* key);
void enviarValorSemaforo(int socketCPU, int tipoMensaje);

#endif /* OPERACIONES_H_ */
