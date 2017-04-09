#include "funcionesCpu.h"

int crearLog() {
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/cpu/logCpu"),
					"cpu", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}

t_config_cpu* levantarConfiguracionCPU(char* archivo) {

        t_config_cpu* conf = malloc(sizeof(t_config_cpu));
        t_config* configCPU;

        configCPU = config_create(archivo);
        conf->puerto_Kernel = config_get_string_value(configCPU, "PUERTO_KERNEL");
        conf->puerto_Memoria = config_get_string_value(configCPU, "PUERTO_MEMORIA");
        conf->ip_Memoria = config_get_string_value(configCPU, "IP_MEMORIA");
        conf->ip_Kernel = config_get_string_value(configCPU, "IP_KERNEL");

        config_destroy(configCPU);
        return conf;
}

int conexionConKernel(){
	socketConexionKernel = createClient(config->ip_Kernel, config->puerto_Kernel);
	if (socketConexionKernel) {
		printf("Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");;
	}

	//------------Envio de mensajes al servidor------------
	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionKernel);
	int operacion;
	void* paquete;
	if (recibir_paquete(socketConexionKernel, &paquete, &operacion) == 0) {
		log_error(logger, "No se recibio nada");
		return -1;
	}else{
		switch (operacion) {
			case HANDSHAKE_KERNEL:
				log_info(logger, "Handshake Kernel.");
				printf("Conexion con Kernel establecida\n");
				return 1;
			default:
				log_error(logger, "Handshake no reconocido.");
				return -1;
		}
	}
}
