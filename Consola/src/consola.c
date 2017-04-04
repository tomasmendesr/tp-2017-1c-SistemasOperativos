#include "funcionesConsola.h"

t_log* logger;

int crearLog();
int verificarExistenciaDeArchivo(char*);

int main(int argc, char **argv){

	if (crearLog()) {
		t_config_consola* config = levantarConfiguracionConsola("confConsola.init");
	} else {
		log_info(logger, "La consola no pudo iniciarse");
		return -1;
	}

	log_destroy(logger);
	return 0;
}

int crearLog() {
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/Consola/consola_log.txt"), "consola", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}

int verificarExistenciaDeArchivo(char* path) {
	FILE * archivoConfig = fopen(path, "r");
	if (archivoConfig!=NULL){
		fclose(archivoConfig);
		return 1;
	}
	return -1;
}




