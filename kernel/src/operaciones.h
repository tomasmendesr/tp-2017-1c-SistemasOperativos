
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
void finalizacion_error(void* paquete_from_cpu, int socket_cpu, int exitCode);

void terminarProceso(t_pcb* pcbRecibido, int socket_cpu);
void escribir(void* imprimir, int socketCpu);
void verificarProcesosConsolaCaida(int socketConsola);
void borrarArchivo(int socketCpu, void* package);
void cerrarArchivo(int socketCpu, void* package);
void leerArchivo(int socketCpu, t_lectura* lectura);
void abrirArchivo(int socketCpu, void* package);
void moverCursor(int socketCpu, t_cursor* cursor);

//Funciones Heap
//Pedido en si
void pedidoReserva(int socket, t_pedido_reserva* pedido);
//La reserva de la memoria
int reservarMemoria(t_pedido_reserva* pedido, t_pcb* pcb);
//Checkea si hay espacio en la pagina
int verificarEspacio(void* pagina, int cant_necesaria);

int solicitarPagina(int pid, int pag, void* resultado);
int escribirPagina(int pid, int pag, void* pagina);

void liberarMemoria(int socket, char* paquete);

#endif /* OPERACIONES_H_ */
