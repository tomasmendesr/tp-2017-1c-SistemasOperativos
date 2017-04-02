#ifndef LIB-COMMONS_LEERARCHIVOCONFIG_H_
#define LIB-COMMONS_LEERARCHIVOCONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/config.h>

#define FAIL -1

t_log * logger;

void leerArchivoDeConfiguracion(char * direccionArchivo);

int verificarExistenciaDeArchivo(char* rutaArchivoConfig);

void setearValores(t_config * archivoConfig);

void crearLogger(int );

/*
int verificarMemoria(void*algo);


int cantidadPaginas (int tamanioArchivo, int tamanioPagina);

int cantidadPalabrasEnArrayDeStrings(char** array); */

#endif /* LIB-COMMONS LEERARCHIVOCONFIG_H */
