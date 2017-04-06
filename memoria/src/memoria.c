/*
 ============================================================================
 Name        : memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "memoria.h"

int main(int argc, char** argv){

	if(!argv[1]) printf("No hay ningun archivo");

    t_config_memoria* config = levantarConfiguracionMemoria(argv[1]);

    char* memoria=malloc(config->marcos*config->marcos_Size);

    int size_adm = (config->marcos*8-1)/config->marcos_Size+1;

    printf("soy la memoria\n");

    free(config);
    return EXIT_SUCCESS;
}
