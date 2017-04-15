/*
 * sockets.h
 *
 *  Created on: 4/4/2017
 *      Author: utnso
 */

#ifndef COMMONS_SOCKETS_H_
#define COMMONS_SOCKETS_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include "collections/list.h"
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

//
// Estructuras utilizadas para el intercambio de mensajes entre procesos.
//

typedef struct {
	int8_t type;
	int16_t length;
}__attribute__((__packed__)) header_t;

typedef struct {
	char *addr;
	char *port;
}__attribute__((__packed__)) ip_info_t;

typedef struct {
	u_int32_t offset;
	u_int32_t tamanio;
}__attribute__((__packed__)) codeIndex;

typedef struct {
	char* 	   nombre;
	u_int32_t  programCounter;
}__attribute__((__packed__)) labelIndex;

typedef struct {
	u_int16_t  id;					   //Identificador único del Programa en el sistema
	u_int32_t  codePointer;			   //Dirección del primer byte en la UMV del segmento de código
	u_int32_t  stackPointer;		   //Dirección del primer byte en la UMV del segmento de stack
	u_int32_t  stackContextPointer;    //Dirección del primer byte en la UMV del Contexto de Ejecución Actual
	u_int32_t  indexCodePointer;	   //Dirección del primer byte en la UMV del Índice de Código
	u_int32_t  labelIndexPointer; 	   //Dirección del primer byte en la UMV del Índice de Etiquetas
	u_int32_t  programCounter;  	   //Número de la próxima instrucción a ejecutar
	u_int32_t  tamanioContexto; 	   //Cantidad de variables (locales y parámetros) del Contexto de Ejecución Actual
	u_int32_t  tamanioIndiceEtiquetas; //Cantidad de bytes que ocupa el Índice de etiquetas
}__attribute__((__packed__)) pcb;

typedef struct {
	char      nombre;
	int32_t valor;
	u_int32_t direccion;
}__attribute__((__packed__)) t_variable;

typedef struct {
	u_int32_t base;
	u_int32_t offset;
	u_int32_t tamanio;
}__attribute__((__packed__)) t_request_umv;

typedef struct {
	u_int32_t base;
	u_int32_t offset;
	u_int32_t tamanio;
	char*	  buffer;
}__attribute__((__packed__)) t_envio_umv;

typedef struct {
	u_int16_t id;
	u_int16_t tamanio;
}__attribute__((__packed__)) t_segmento;

typedef struct{
	int8_t result;
	u_int32_t base;
}__attribute__((__packed__)) t_crear_segmento;

enum enum_protocolo {// Si yo soy el kernel tengo que enviar handshake_kernel.
	PEDIDO_INFO_CONEXION = 1,
	RESPUESTA_PEDIDO_INFO_CONEXION = 2,
	HANDSHAKE_CPU = 3,
	HANDSHAKE_KERNEL = 4,
	HANDSHAKE_MEMORIA = 5,
	HANDSHAKE_FS = 8,
	NOTIFICACION_DATOS_CPU = 6,
	OK = 7
};

//Mensajes que el kernel le envia al CPU
enum protocolo_kernel_a_cpu{
	EJECUTAR_QUANTUM = 20,
	VALOR_VARIABLE_COMPARTIDA = 21,
	RESPUESTA_WAIT_SEGUIR_EJECUTANDO = 22,
	RESPUESTA_WAIT_DETENER_EJECUCION = 23,
	RESPUESTA_SIGNAL_OK = 24,
	RESPUESTA_GRABAR_VARIABLE_COMPARTIDA_OK = 25,
	RESPUESTA_IMPRIMIR_TEXTO_OK = 26,
	RESPUESTA_IMPRIMIR_VARIABLE_OK = 27
};

//Mensajes que el CPU le envia al kernel
enum protocolo_cpu_a_kernel{
	FINALIZACION_QUANTUM = 40,
	SEMAFORO_WAIT = 41,
	SEMAFORO_SIGNAL = 42,
	ENTRADA_SALIDA = 43,
	GRABAR_VARIABLE_COMPARTIDA = 44,
	OBTENER_VALOR_VARIABLE_COMPARTIDA = 45,
	IMPRIMIR = 46,
	IMPRIMIR_TEXTO = 47,
	FINALIZACION_PROCESO = 48,
	ENVIO_PCB = 49,
	MUERTE_CPU = 50,
	SENIAL_SIGUSR1 = 51
};

//Mensajes que se le envian a la memoria.
enum protocolo_a_memoria{
	ASIGNAR_PAGINAS = 59,
	SOLICITUD_BYTES = 60,
	GRABAR_BYTES = 61,
	CAMBIO_PROCESO_ACTIVO = 62,
	CREAR_SEGMENTO = 63,
	DESTRUIR_SEGMENTOS = 64,
	INICIAR_PROGRAMA = 65,
	FINALIZAR_PROGRAMA = 66
};

//Mensajes que la memoria le envia el resto de los procesos.
enum protocolo_memoria_a_cualquiera{
	RESPUESTA_BYTES = 80,
	SEGMENTATION_FAULT = 81,
	MEMORY_OVERLOAD = 82,
	SEGMENTO_CREADO = 83,
	OVERFLOW = 84
};

//Mensajes entre Kernel y Programa
enum protocolo_kernel_programa{
	IMPRIMIR_VARIABLE_PROGRAMA = 100,
	IMPRIMIR_TEXTO_PROGRAMA = 101,
	ERROR_GENERAL = 102,
	SEGMENTATION_FAULT_PROGRAMA = 103,
	MEMORY_OVERLOAD_PROGRAMA = 104,
	FINALIZAR_EJECUCION = 105,
	FINALIZAR_EJECUCION_ERROR = 106
};

enum protocolo_programa_a_kernel{
	CODIGO_PROGRAMA = 120,
	HANDSHAKE_PROGRAMA = 121,
	ENVIO_CODIGO = 122
};


//
// Funciones básicas para manejo de sockets.
//

int getSocket(void);
int bindSocket(int, char *, char *);
int listenSocket(int, int);
int acceptSocket(int);
int sendSocket(int socket, header_t* cabecera, void* data);
int connectSocket(int, char *, char *);
int sendallSocket(int s, void* buf, int len);
int createServer(char *, char *, int);
int createClient(char *, char *);
int recibir_paquete(int socket, void** paquete, int* tipo);
int recibir_string(int socket,void** puntero_buffer,int* tipo);
int agregar_caracter_nulo(void* stream, int tamanio);
header_t crear_cabecera(int codigo, int length);
int recibir_info(int socket, void** paquete, int* tipo);
int enviar_info(int sockfd, int codigo_operacion, int length, void* buff);
int enviar_paquete_vacio(int codigo_operacion, int socket);
int enviar_paquete_vacio_a_cpu(int codigo_operacion, int socket);
bool recibirHanshake(int socket, int handshakeRecibir, int handshakeRespuesta);
bool enviarHandshake(int socket, int handshakeEnviar, int handshakeRespuesta);
int finalizarConexion(int socket);

//
// Serializadores y Deserializadores de mensajes.
//

char *program_serializer(char *codigo_programa);
int deserializar_string(void* paquete, char** string);

void* variable_serializer(t_variable* var, int16_t *length);
t_variable* variable_deserializer(int socketfd);

void* codeIndex_serializer(codeIndex *self, int16_t *length);
codeIndex* codeIndex_deserializer(int socketfd);

void* pcb_serializer(pcb* self, int16_t *length);
pcb* pcb_deserializer(int socketfd);

char *paqueteEnviarAEjecutar_serializer(u_int16_t quantum, uint32_t retardo_quantum,pcb *pcb_proceso);

t_segmento* segmento_deserializer(int socketfd);
void* segmento_serializer(t_segmento *self, int16_t *length);

int sendAll(int fd, char *cosa, int size, int flags);
int recvAll(int fd, char *buffer, int size, int flags);

#endif /* COMMONS_SOCKETS_H_ */
