#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "consola.h"

t_log* logger;
t_config* config;

char* ip_kernel;
int puerto_kernel;

int crearLog();
int leerArchivoConfig();
int verificarExistenciaDeArchivo(char*);

int main(int argc, char **argv){
	if (crearLog()) {
		if (leerArchivoConfig() != 1) {
			log_info(logger, "Hubo un error al abrir el archivo de configuracion de la consola.");
		}
	} else {
		log_info(logger, "La consola no pudo iniciarse");
		return -1;
	}

	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

int crearLog() {
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/Consola/consola_log.txt"), "consola", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}

int leerArchivoConfig() {
	char* path = "/home/utnso/tp-2017-1c-Dirty-Cow/Consola/confConsola.txt";
	 if (verificarExistenciaDeArchivo(path) == -1){
		 puts("[ERROR] Archivo de configuracion no encontrado");
		 return 0;
	 }else{
		 config = config_create(path);
	 }
	if (config) {
		if (config_has_property(config, "PUERTO_KERNEL")) {
			puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
			log_info(logger, "Puerto del Kernel seteado correctamente");
		} else {
			log_info(logger, "El archivo de configuracion no tiene puerto");
			return 0;

		}
		if (config_has_property(config, "IP_KERNEL")) {
			ip_kernel = config_get_string_value(config, "IP_KERNEL");
			log_info(logger, "IP del Kernel seteado correctamente");
		} else {
			log_info(logger, "El archivo de configuracion no tiene IP");
			return 0;
		}
	return 1;
	}else
		return 0;
}


int verificarExistenciaDeArchivo(char* path) {
	FILE * archivoConfig = fopen(path, "r");
	if (archivoConfig!=NULL){
		fclose(archivoConfig);
		return 1;
	}
	return -1;
}




