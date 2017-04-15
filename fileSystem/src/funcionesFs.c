#include "funcionesFs.h"

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
