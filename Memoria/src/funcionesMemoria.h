
#include <commons/config.h>

#ifndef SRC_FUNCIONESMEMORIA_H_
#define SRC_FUNCIONESMEMORIA_H_


typedef struct {

	int puerto;
	int marcos;
	int marcos_Size;
	int entradas_Cache;
	int cache_x_Proceso;
	char* reemplazo_cache;
	int retardo_Memoria;

}t_config_memoria;

t_config_memoria* levantarConfiguracionMemoria(char* archivo);

#endif /* SRC_FUNCIONESMEMORIA_H_ */
