#include "funcionesConsola.h"

t_config_consola* levantarConfiguracionConsola(char * archivo) {

	t_config_consola* config = malloc(sizeof(t_config_consola));

	t_config* configConsola;
	configConsola = config_create(archivo);
	config->ip_Kernel = config_get_string_value(configConsola, "IP_KERNEL");
	config->puerto_Kernel = config_get_int_value(configConsola, "PUERTO_KERNEL");

	config_destroy(configConsola);
	return config;
}
