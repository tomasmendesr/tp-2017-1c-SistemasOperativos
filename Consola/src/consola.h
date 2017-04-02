#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <Libs-Commons/leerArchivoConfig.h>
#include "Libs-Commons/tipos.h"
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define PUERTO_KERNEL "PUERTO_KERNEL"
#define IP_KERNEL "IP_KERNEL"
#define ARCHIVO_CONF "configConsola"
#define LOGGER_FILE "consola_log.txt"

FILE * fp;

//int tamArchivo(char* direccionArchivo);

//char* leerProgramaAnSISOP(char* direccionArchivo);

t_log* getLogger();

//void funcionHiloConsola();

#endif /* CONSOLA_H_ */
