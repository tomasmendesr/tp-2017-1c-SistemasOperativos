/*
 * funcionesFs.h
 *
 *  Created on: 7/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESFS_H_
#define FUNCIONESFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/sockets.h>

#define configuracionFS "confFileSystem.init"
#define MAX_LEN_PUERTO 6
#define IP "127.0.0.1"
#define BACKLOG 10

//TADS
typedef struct{
	char* puertoEscucha;
	char* punto_montaje;
}t_config_FS;

//Prototipos
t_config_FS* levantarConfiguracion(char* archivo);
void destruirConfiguracionFS(t_config_FS* conf);
void esperarConexionKernel();

//Variables Globales
t_config_FS* conf;
int socketEscucha;
int socketConexionKernel;


#endif /* FUNCIONESFS_H_ */
