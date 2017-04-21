#include "funcionesFs.h"

void crearConfig(int argc, char* argv[]){
	char* pathConfig = string_new();

	if(argc>1){
		string_append(&pathConfig,argv[1]);
	}
		else string_append(&pathConfig,configuracionFS);
	if(verificarExistenciaDeArchivo(pathConfig)){
		conf = levantarConfiguracion(pathConfig);
	}else{
		log_info(logger,"No se pudo levantar archivo de configuracion\n");
		exit(EXIT_FAILURE);
	}
	log_info(logger,"Se levanto la configuracion correctamente\n");
	printf("Se levanto la configuracion correctamente\n");
}

t_config_FS* levantarConfiguracion(char* archivo){

	t_config_FS* conf = malloc(sizeof(t_config_FS));

	t_config* configFS = config_create(archivo);

	conf->puertoEscucha = malloc(strlen(config_get_string_value(configFS, "PUERTO"))+1);
	strcpy(conf->puertoEscucha, config_get_string_value(configFS, "PUERTO"));

	conf->punto_montaje = malloc(strlen(config_get_string_value(configFS, "PUNTO_MONTAJE"))+1);
	strcpy(conf->punto_montaje, config_get_string_value(configFS, "PUNTO_MONTAJE"));

	config_destroy(configFS);

	return conf;
}

void destruirConfiguracionFS(t_config_FS* conf){
	free(conf->puertoEscucha);
	free(conf->punto_montaje);
	free(conf);
}
