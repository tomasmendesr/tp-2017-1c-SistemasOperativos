
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
void finalizacion_quantum(void* paquete_from_cpu, int socket_cpu);
void finalizacion_proceso(void* paquete_from_cpu, int socket_cpu);
void finalizarPrograma(int consola_fd, int pid);
void finalizacion_segment_fault(void* package,int socketCPU);
void finalizacion_stackoverflow(void* package,int socketCPU);
void finalizacion_error_memoria(void* package,int socketCPU);
void reservarMemoria(int socket, char* paquete);
void liberarMemoria(int socket, char* paquete);
void terminarProceso(t_pcb* pcbRecibido, int socket_cpu);
void imprimirPorPantalla(void* imprimir, int socketCpu);

#endif /* OPERACIONES_H_ */
