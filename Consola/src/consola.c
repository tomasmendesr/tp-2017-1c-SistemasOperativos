#include "consola.h"

uint32_t puertoKernel = 0;
char* ipKernel;
t_config* tConfig;

/*
 * Prototipos
 */
Boolean loadConfig();
t_log* getLogger();

t_log* logger = NULL;

int main(){
	log_info(getLogger(), "===============================================");
	log_info(getLogger(), "===== BIENVENIDO A LA CONSOLA DEL SISTEMA =====");
	log_info(getLogger(), "===============================================");
	log_info(getLogger(), "                                               ");
	log_info(getLogger(), "                                               ");
	log_info(getLogger(), "Leyendo archivo de configuracion y conectandose al KERNEL");

	return 0;
}



t_log* getLogger() {
	if (logger == NULL) {
		logger = log_create(LOGGER_FILE, "CONSOLA", FALSE, LOG_LEVEL_INFO);
	}
	return logger;
}



