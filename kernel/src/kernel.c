/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funcionesKernel.h"


int main(int argc, char** argv){

	crearConfig(argc,argv);

	//Tiramos 2 hilos porque necesitamos escuchar conexiones y desconexion de sockets por puertos diferentes
	//Planificador de largo plazo
	//Planificador de corto plazo
	pthread_t hilo_plp, hilo_pcp;

	int resultado_creacion_plp, resultado_creacion_pcp;

	// Lanzar hilo PLP - conexion memoria, fs y consola
	resultado_creacion_plp = pthread_create(&hilo_plp, NULL, plp, NULL);

	// Se valida que se haya creado el hilo, pthread_create devuelve 0 en caso de que NO haya errores
	if(resultado_creacion_plp){
		printf("Se produjo un error al crear el hilo plp, codigo de error: %d\n", resultado_creacion_plp);
		exit(EXIT_FAILURE);
	}
	// Lanzar hilo PCP - conexion CPU
	resultado_creacion_pcp = pthread_create(&hilo_pcp, NULL, pcp, NULL);

	if(resultado_creacion_pcp){
		printf("Se produjo un error al crear el hilo pcp, codigo de error: %d\n", resultado_creacion_pcp);
		exit(EXIT_FAILURE);
	}

	pthread_join(hilo_pcp, NULL);
	pthread_join(hilo_plp, NULL);
	destruirConfiguracionKernel(config);
	return EXIT_SUCCESS;
}

