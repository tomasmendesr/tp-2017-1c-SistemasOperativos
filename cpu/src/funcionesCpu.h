/*
 * cpu.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/sockets.h>
#include <commons/structUtiles.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <sys/stat.h>
#include "primitivas.h"
#include <signal.h>

#define configuracionCPU "../confCpu.init"
#define ansisop "facil.ansisop"

typedef struct{
	char* puerto_Memoria;
	char* ip_Memoria;
	char* puerto_Kernel;
	char* ip_Kernel;
}t_config_cpu;

void* paquete2;
int socketConexionKernel;
int socketConexionMemoria;
t_config_cpu* config;
t_log* logger;
/*desde kernel*/
uint32_t tamanioStack;
int quantum;
/*desde memoria*/
uint32_t tamanioPagina;
int quamtum;

pcb_t* crearPCB(char* buf, int pid);
t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc);
void levantarArchivo(char*path,char**content);
t_config_cpu* levantarConfiguracionCPU(char* archivo);
int conexionConKernel(void);
int conexionConMemoria(void);
void freeConf(t_config_cpu* config);
int16_t almacenarBytes(pedido_bytes_t* pedido, void* paquete);
int16_t solicitarBytes(pedido_bytes_t* pedido, void** paquete);
t_puntero definirVariable(t_nombre_variable nombre);
void inicializarFunciones(void);
void ejecutarPrograma(void);
int crearLog(void);
void crearConfig(int argc, char* argv[]);
int16_t recibirTamanioStack(void);
void recibirPCB(void* paquete);
int32_t leerCompartida(void* paquete, char* variable);
int16_t asignarCompartida(int valor, char* variable);
int16_t recibirTamanioPagina(void);
void revisarFinalizarCPU();
void revisarSigusR1(int signo);
void limpiarInstruccion(char * instruccion);
void comenzarEjecucionDePrograma();
char* solicitarProximaInstruccion();
void finalizarEjecucionPorFinQuantum();
void finalizarEjecucionPorFinPrograma();
void finalizarProcesoPorStackOverflow();
void finalizarProcesoPorSegmentationFault();
int32_t requestHandlerKernel(void** paquete);
int32_t requestHandlerMemoria();
int32_t expulsarPCB(void);


#endif /* FUNCIONESCPU_H_ */
