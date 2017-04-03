#include "funcionesMemoria.h"

t_config_memoria* levantarConfiguracionMemoria(char* archivo) {

	t_config_memoria* config = malloc(sizeof(t_config_memoria));
	t_config* configMemoria;

	configMemoria = config_create(archivo);
	config->puerto = config_get_int_value(configMemoria, "PUERTO");
	config->marcos = config_get_int_value(configMemoria, "MARCOS");
	config->marcos_Size = config_get_int_value(configMemoria, "MARCOS_SIZE");
	config->entradas_Cache = config_get_int_value(configMemoria, "ENTRADAS_CACHE");
	config->entradas_Cache = config_get_int_value(configMemoria, "CACHE_X_PROC");
	config->reemplazo_cache = config_get_string_value(configMemoria, "REEMPLAZO_CACHE");
	config->retardo_Memoria = config_get_int_value(configMemoria, "RETARDO_MEMORIA");

	config_destroy(configMemoria);
	return config;

}
