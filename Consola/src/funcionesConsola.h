
#include <commons/config.h>

#ifndef SRC_FUNCIONESCONSOLA_H_
#define SRC_FUNCIONESCONSOLA_H_

typedef struct{
	char* ip_Kernel;
	int puerto_Kernel;

}t_config_consola;

t_config_consola* levantarConfiguracionConsola(char * archivo);



#endif /* SRC_FUNCIONESCONSOLA_H_ */
