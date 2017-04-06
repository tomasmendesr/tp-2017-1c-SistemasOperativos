/*
 ============================================================================
 Name        : memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funcionesMemoria.h"

int main(int argc, char** argv){

	log = log_create("logMemoria","memoria",true,LOG_LEVEL_TRACE);

	crearConfig(argv[1]);

	inicializarMemoria();

	//LevantarServer

	levantarInterfaz();

    destruirConfiguracionMemoria(config);

    return EXIT_SUCCESS;
}
