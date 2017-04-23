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
#include "structUtiles.h"
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


/*Me lo llevo para structUtiles*/

//typedef struct{
//	uint32_t pid;  //Identificador único del Programa en el sistema
//	uint32_t programCounter; //Número de la próxima instrucción a ejecutar
//	uint32_t cantPaginasCodigo;
////	t_intructions* indiceCodigo;
//	//char* etiquetas;  Verificar si es necesario
//	//t_list* indiceStack;
//	int16_t exitCode; //Codigo de finalizacion
//	uint32_t consolaFd;
//
//}__attribute__((__packed__))pcb_t;

typedef struct {
	char      nombre;
	int32_t valor;
	u_int32_t direccion;
}__attribute__((__packed__)) t_variable;

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

	EXEC_PCB = 10,
	EXEC_QUANTUM = 11,
	VALOR_VAR_COMPARTIDA = 12,
	RESPUESTA_WAIT_SEGUIR_EJECUCION = 13,
	RESPUESTA_WAIT_DETENER_EJECUCION = 14,
	TAMANIO_STACK_PARA_CPU = 19,
	TAMANIO_PAGINAS_NUCLEO = 20,
	/*estas se podrian reemplazar por un OK*/
	RESPUESTA_SIGNAL_OK = 15,
	RESPUESTA_ASIG_VAR_COMPARTIDA_OK = 16,
	RESPUESTA_IMPRIMIR_TEXTO_OK = 17,
	RESPUESTA_IMPRIMIR_VARIABLE_OK = 18
};

//Mensajes que el CPU le envia al kernel
enum protocolo_cpu_a_kernel{
	ENVIO_PCB = 30,
	SEM_WAIT = 31,
	SEM_SIGNAL = 32,
	LEER_VAR_COMPARTIDA = 33,
	ASIG_VAR_COMPARTIDA = 34,
	IMPRIMIR_VALOR = 35,
	IMPRIMIR_TEXTO = 36,
	FIN_EJECUCION = 37,
	FIN_PROCESO = 38,
	DESCONEXION_CPU = 39,
	RESERVAR_MEMORIA = 40,
	LIBERAR_MEMORIA = 41,
	ABRIR_ARCHIVO = 42,
	CERRAR_ARCHIVO = 43,
	BORRAR_ARCHIVO = 44,
	LEER_ARCHIVO = 45,
	ESCRIBIR_ARCHIVO = 46,
	SIGURSR = 303,
	/*Finalizaciones irregulares*/
	FIN_ERROR_MEMORIA = 47,
	STACKOVERFLOW = 48,
	FIN_SEGMENTATION_FAULT = 49
};

//Mensajes que se le envian a la memoria.
enum protocolo_a_memoria{
	ASIGNAR_PAGINAS = 59,
	LEER_VAR = 203,
	SOLICITUD_BYTES = 60,
	GRABAR_BYTES = 61,
	CAMBIO_PROCESO_ACTIVO = 62,
	CREAR_SEGMENTO = 63,
	DESTRUIR_SEGMENTOS = 64,
	INICIAR_PROGRAMA = 65
};

//Mensajes que la memoria le envia el resto de los procesos.
enum protocolo_memoria_a_cualquiera{
	ENVIAR_TAMANIO_PAGINA = 79,
	RESPUESTA_BYTES = 80,
	SEGMENTATION_FAULT = 81,
	MEMORY_OVERLOAD = 82,
	SEGMENTO_CREADO = 83,
	OVERFLOW = 84,
	OP_OK = 85,
	QUILOMBO = 86,
	SIN_ESPACIO = 87
};

//Mensajes entre Kernel y Programa
enum protocolo_kernel_programa{
	PID_PROGRAMA = 99,
	IMPRIMIR_VARIABLE_PROGRAMA = 100,
	IMPRIMIR_TEXTO_PROGRAMA = 101,
	ERROR_GENERAL = 102,
	SEGMENTATION_FAULT_PROGRAMA = 103,
	MEMORY_OVERLOAD_PROGRAMA = 104,
	FINALIZAR_EJECUCION = 105,
	FINALIZAR_EJECUCION_ERROR = 106,
	PROCESO_RECHAZADO = 107
};

enum protocolo_programa_a_kernel{
	CODIGO_PROGRAMA = 120,
	HANDSHAKE_PROGRAMA = 121,
	ENVIO_CODIGO = 122,
	FINALIZAR_PROGRAMA = 123
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
int recibir_info(int socket, void** paquete, int* tipo_mensaje);
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


//************* NUEVOS **************
t_list* deserializarIndiceStack(char* buffer);
t_buffer_tamanio* serializarIndiceStack(t_list* indiceStack);

t_list* deserializarIndiceCodigo(char* indice, uint32_t tam);
t_buffer_tamanio* serializarIndiceStack(t_list* indiceStack);

pcb_t* deserializar_pcb(char* package);
t_buffer_tamanio* serializar_pcb(pcb_t* pcb);
//***********************************


void* pcb_serializer(pcb_t* self, int16_t *length);
pcb_t* pcb_deserializer(int socketfd);

char *paqueteEnviarAEjecutar_serializer(u_int16_t quantum, uint32_t retardo_quantum,pcb_t *pcb_proceso);

int sendAll(int fd, char *cosa, int size, int flags);
int recvAll(int fd, char *buffer, int size, int flags);

#endif /* COMMONS_SOCKETS_H_ */
