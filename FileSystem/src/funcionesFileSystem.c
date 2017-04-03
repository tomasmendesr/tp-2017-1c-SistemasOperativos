#include "funcionesFileSystem.h"

t_config_fs* levantarConfiguracionFileSystem(char* archivo){

	t_config_fs* conf = malloc(sizeof(t_config_fs));

	t_config* configMemoria;
	configMemoria = config_create(archivo);
	conf->puerto = config_get_int_value(configMemoria, "PUERTO");
	conf->punto_Montaje = config_get_string_value(configMemoria, "PUNTO_MONTAJE");

	config_destroy(configMemoria);
	return conf;
}
