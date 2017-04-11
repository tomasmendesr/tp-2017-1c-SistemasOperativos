/*
 * memoria.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/interface.h>
#include <commons/cosas.h>
#include <pthread.h>

#define configuracionMemoria "../confMemoria.init"
#define MAX_LEN_PUERTO 6
#define frame_size config->marcos_Size
#define IP "127.0.0.1"
#define BACKLOG 10

typedef struct{
        char* puerto;
        int marcos;
        int marcos_Size;
        int entradas_Cache;
        int cache_x_Proceso;
        char* reemplazo_cache;
        int retardo_Memoria;
}t_config_memoria;

typedef struct{
	int pid;
	int pag;
}t_entrada_tabla;

typedef struct{
	int pid;
	int pag;
	char* content;
}t_entrada_cache;

void crearConfig(int argc, char* argv[]);
t_config_memoria* levantarConfiguracionMemoria(char* archivo);
void destruirConfiguracionMemoria(t_config_memoria* config);

//Funciones de conexionado
int esperarConexiones();
/* Esta funcion hace la creacion de la memoria y todas las estructuras
 * administrativas necesarias para que el sistema arranque
 */
void inicializarMemoria();

//Funciones administracion memoria
int primerFrameLibre();
int primerFrameLibreCache();
int framesLibresCache();
int buscarPaginas(int pid, int frame);
int framesLibres();
int buscarFrame(int pid, int pag);
int buscarPagCache(int pid, int pag);
int escribir(int pid, int pag, int offset, char* contenido, int size); //Devuelve codigos error
int leer(int pid, int pag, int offset, int size, char* resultado); //Devuelve codigos error

 	 	 	 	 	 	/*Este thread maneja tanto cpus como kernel, porque la interfaz es una sola.*/
void requestHandler();	/* Solo una de las operaciones esta restringida a Kernel,
						asi que validamos eso solo*/
//void iniciarPrograma(int fd);
void iniciarPrograma(int fd, int pid, int cantPag);
//void finalizarPrograma(int fd);
void finalizarPrograma(int fd, int pid);
//void solicitudBytes();
char* solicitudBytes(int pid, int pag, int offset, int size);
void grabarBytes();

//Funciones de interfaz
void levantarInterfaz();
void retardo(char* comando, char* param);
void dump(char* comando, char* param);
void flush(char* comando, char* param);
void size(char* comando, char* param);

//Variables Globales
t_log* logger;
t_config_memoria* config;
int socketEscuchaConexiones;
int socketConexionKernel;
int socketConexionCpu;

char* memoria; /*Este va a ser el bloque que simula la memoria principal.
				Uso char* porque sizeof(char) = 1 y facilita la aritmetica,
				pero no tiene nada que ver con caracteres*/
t_entrada_cache* cache;

#endif /* FUNCIONESMEMORIA_H_ */
