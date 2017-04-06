#include "funcionesCpu.h"


t_config_cpu* levantarConfiguracionCPU(char* archivo) {

        t_config_cpu* conf = malloc(sizeof(t_config_cpu));
        t_config* configCPU;

        configCPU = config_create(archivo);
        conf->puerto_Kernel = config_get_int_value(configCPU, "PUERTO_KERNEL");
        conf->puerto_Memoria = config_get_int_value(configCPU, "PUERTO_MEMORIA");
        conf->ip_Memoria = config_get_string_value(configCPU, "IP_MEMORIA");
        conf->ip_Kernel = config_get_int_value(configCPU, "IP_KERNEL");

        config_destroy(configCPU);
        return conf;
}
