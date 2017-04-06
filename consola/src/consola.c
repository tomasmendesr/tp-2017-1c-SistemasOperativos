/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "funcionesConsola.h"

int main(int argc, char** argv){

	crearLog();

	crearConfig(argv[1]);

	levantarInterfaz();

	return EXIT_SUCCESS;
}







