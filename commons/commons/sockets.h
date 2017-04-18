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


/*Me lo llevo para structUtiles*/
typedef struct{
	uint32_t pid;  //Identificador único del Programa en el sistema
	uint32_t programCounter; //Número de la próxima instrucción a ejecutar
	uint32_t cantPaginasCodigo;
//	t_intructions* indiceCodigo;
	//char* etiquetas;  Verificar si es necesario
	//t_list* indiceStack; Verificar si es necesario
	int16_t exitCode; //Codigo de finalizacion
	uint32_t consolaFd;

}__attribute__((__packed__))pcb_t;

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
	EJECUTAR_QUANTUM = 20,
	VALOR_VARIABLE_COMPARTIDA = 21,
	RESPUESTA_WAIT_SEGUIR_EJECUTANDO = 22,
	RESPUESTA_WAIT_DETENER_EJECUCION = 23,
	RESPUESTA_SIGNAL_OK = 24,
	RESPUESTA_GRABAR_VARIABLE_COMPARTIDA_OK = 25,
	RESPUESTA_IMPRIMIR_TEXTO_OK = 26,
	RESPUESTA_IMPRIMIR_VARIABLE_OK = 27,
	EXECUTE_PCB = 135,
	VAR_COMPARTIDA_ASIGNADA = 136,
	VALOR_VAR_COMPARTIDA = 137,
	SIGNAL_SEMAFORO = 138,
	TAMANIO_STACK_PARA_CPU = 139,
	TAMANIO_PAGINAS_NUCLEO = 140
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
	SENIAL_SIGUSR1 = 51,
	// AGREGADAS (Despues hago una limpieza)
	QUANTUM = 123,
	EXIT = 124,
	IMPRIMIR_VALOR = 125,
	LEER_VAR_COMPARTIDA = 127,
	ASIG_VAR_COMPARTIDA = 128,
	WAIT = 129,
	SIGNAL = 130,
	FINALIZO_POR_ERROR_MEMORIA = 133,
	STACKOVERFLOW = 134,
};

//Mensajes que se le envian a la memoria.
enum protocolo_a_memoria{
	ASIGNAR_PAGINAS = 59,
	SOLICITUD_BYTES = 60,
	GRABAR_BYTES = 61,
	CAMBIO_PROCESO_ACTIVO = 62,
	CREAR_SEGMENTO = 63,
	DESTRUIR_SEGMENTOS = 64,
	INICIAR_PROGRAMA = 65
};

//Mensajes que la memoria le envia el resto de los procesos.
enum protocolo_memoria_a_cualquiera{
	RESPUESTA_BYTES = 80,
	SEGMENTATION_FAULT = 81,
	MEMORY_OVERLOAD = 82,
	SEGMENTO_CREADO = 83,
	OVERFLOW = 84,
	ENVIAR_TAMANIO_PAGINA_A_CPU = 160

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
	FINALIZAR_EJECUCION_ERROR = 106
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


void* pcb_serializer(pcb_t* self, int16_t *length);
pcb_t* pcb_deserializer(int socketfd);

char *paqueteEnviarAEjecutar_serializer(u_int16_t quantum, uint32_t retardo_quantum,pcb_t *pcb_proceso);

int sendAll(int fd, char *cosa, int size, int flags);
int recvAll(int fd, char *buffer, int size, int flags);

#endif /* COMMONS_SOCKETS_H_ */
