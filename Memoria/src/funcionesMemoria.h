
#include <commons/config.h>

#ifndef SRC_FUNCIONESMEMORIA_H_
#define SRC_FUNCIONESMEMORIA_H_

#include <interface.h>
#include <pthread.h>

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

//Funciones de interfaz
void levantarInterfaz();
void retardo(char* comando, char* param);
void dump(char* comando, char* param);
void flush(char* comando, char* param);
void size(char* comando, char* param);

#endif /* SRC_FUNCIONESMEMORIA_H_ */
